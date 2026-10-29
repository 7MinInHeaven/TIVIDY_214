#include "WorkflowBlueprint.h"

#include <fstream>
#include <set>
#include <sstream>

/**
 * @brief Contains private validation helpers for workflow blueprints.
 */
namespace {

/**
 * @brief Reads and validates a node's type field.
 */
std::string nodeType(const nlohmann::json& node, const std::string& path) {
    if (!node.is_object()) {
        throw BlueprintValidationError(path + " must be an object");
    }

    if (!node.contains("type") || !node["type"].is_string()) {
        throw BlueprintValidationError(path + ".type must be a string");
    }

    return node["type"].get<std::string>();
}

/**
 * @brief Recursively validates a workflow tree node.
 */
void validateNode(const nlohmann::json& node,
                  const std::string& path,
                  std::size_t depth,
                  std::size_t& nodeCount,
                  const BlueprintLimits& limits,
                  bool isRoot) {
    if (depth > limits.maxDepth) {
        throw BlueprintValidationError(path + " exceeds maximum nesting depth");
    }

    ++nodeCount;
    if (nodeCount > limits.maxNodes) {
        throw BlueprintValidationError("blueprint exceeds maximum node count");
    }

    if (!node.is_object()) {
        throw BlueprintValidationError(path + " must be an object");
    }

    const std::string type = nodeType(node, path);
    if (type != "WorkGroup" && type != "Stage" &&
        type != "SubProcess" && type != "WorkUnit") {
        throw BlueprintValidationError(path + ".type is unsupported: " + type);
    }

    if (!node.contains("name") || !node["name"].is_string() ||
        node["name"].get<std::string>().empty()) {
        throw BlueprintValidationError(path + ".name must be a non-empty string");
    }

    if (node["name"].get<std::string>().size() > limits.maxNameLength) {
        throw BlueprintValidationError(path + ".name exceeds maximum length");
    }

    if (isRoot && type == "WorkUnit") {
        throw BlueprintValidationError(path + " cannot be a root WorkUnit");
    }

    const bool composite = type != "WorkUnit";
    if (composite) {
        if (!node.contains("children") || !node["children"].is_array()) {
            throw BlueprintValidationError(path + ".children must be an array");
        }

        std::set<std::string> siblingNames;
        std::size_t childIndex = 0;
        for (const nlohmann::json& child : node["children"]) {
            const std::string childPath =
                path + ".children[" + std::to_string(childIndex) + "]";
            if (!child.is_object() || !child.contains("name") ||
                !child["name"].is_string()) {
                validateNode(child, childPath, depth + 1, nodeCount, limits, false);
            } else {
                const std::string childName = child["name"].get<std::string>();
                if (!siblingNames.insert(childName).second) {
                    throw BlueprintValidationError(
                        path + " contains duplicate child name: " + childName);
                }
                validateNode(child, childPath, depth + 1, nodeCount, limits, false);
            }
            ++childIndex;
        }
    } else if (node.contains("children")) {
        throw BlueprintValidationError(path + ".children is not allowed on WorkUnit");
    }
}

/**
 * @brief Validates the optional required input definitions.
 */
void validateInputs(const nlohmann::json& blueprint,
                    const BlueprintLimits& limits) {
    if (!blueprint.contains("required_inputs")) {
        return;
    }

    if (!blueprint["required_inputs"].is_array()) {
        throw BlueprintValidationError("required_inputs must be an array");
    }

    if (blueprint["required_inputs"].size() > limits.maxInputs) {
        throw BlueprintValidationError("blueprint exceeds maximum input count");
    }

    std::set<std::string> keys;
    std::size_t index = 0;
    for (const nlohmann::json& input : blueprint["required_inputs"]) {
        const std::string path =
            "required_inputs[" + std::to_string(index) + "]";
        if (!input.is_object() || !input.contains("key") ||
            !input["key"].is_string() ||
            input["key"].get<std::string>().empty()) {
            throw BlueprintValidationError(path + ".key must be a non-empty string");
        }

        const std::string key = input["key"].get<std::string>();
        if (key.size() > limits.maxNameLength) {
            throw BlueprintValidationError(path + ".key exceeds maximum length");
        }
        if (!keys.insert(key).second) {
            throw BlueprintValidationError("duplicate required input key: " + key);
        }

        if (input.contains("prompt") && !input["prompt"].is_string()) {
            throw BlueprintValidationError(path + ".prompt must be a string");
        }
        ++index;
    }
}

} // namespace

/**
 * @brief Constructs a blueprint validation exception.
 */
BlueprintValidationError::BlueprintValidationError(const std::string& message)
    : std::runtime_error(message) {}

/**
 * @brief Creates the default blueprint safety limits.
 */
BlueprintLimits::BlueprintLimits()
    : maxFileBytes(1024 * 1024),
      maxDepth(32),
      maxNodes(1000),
      maxInputs(100),
      maxNameLength(512) {}

/**
 * @brief Loads and validates a blueprint file from disk.
 */
nlohmann::json WorkflowBlueprint::loadAndValidate(
    const std::string& path,
    const BlueprintLimits& limits) {
    std::ifstream file(path.c_str(), std::ios::binary);
    if (!file) {
        throw BlueprintValidationError("cannot open blueprint: " + path);
    }

    file.seekg(0, std::ios::end);
    const std::streamoff size = file.tellg();
    if (size < 0) {
        throw BlueprintValidationError("cannot determine blueprint size: " + path);
    }
    if (static_cast<std::size_t>(size) > limits.maxFileBytes) {
        throw BlueprintValidationError("blueprint exceeds maximum file size: " + path);
    }
    file.seekg(0, std::ios::beg);

    std::stringstream contents;
    contents << file.rdbuf();

    nlohmann::json blueprint;
    try {
        blueprint = nlohmann::json::parse(contents.str());
    } catch (const nlohmann::json::parse_error& error) {
        throw BlueprintValidationError(
            "invalid JSON in " + path + ": " + error.what());
    }

    validate(blueprint, path, limits);
    return blueprint;
}

/**
 * @brief Validates the complete workflow blueprint structure.
 */
void WorkflowBlueprint::validate(const nlohmann::json& blueprint,
                                 const std::string& source,
                                 const BlueprintLimits& limits) {
    if (!blueprint.is_object()) {
        throw BlueprintValidationError(source + " must contain a JSON object");
    }
    if (!blueprint.contains("workflow_name") ||
        !blueprint["workflow_name"].is_string() ||
        blueprint["workflow_name"].get<std::string>().empty()) {
        throw BlueprintValidationError(
            source + ".workflow_name must be a non-empty string");
    }
    if (blueprint["workflow_name"].get<std::string>().size() >
        limits.maxNameLength) {
        throw BlueprintValidationError(
            source + ".workflow_name exceeds maximum length");
    }
    if (!blueprint.contains("chain") || !blueprint["chain"].is_array() ||
        blueprint["chain"].empty()) {
        throw BlueprintValidationError(
            source + ".chain must be a non-empty array");
    }

    validateInputs(blueprint, limits);

    std::size_t nodeCount = 0;
    std::set<std::string> rootNames;
    std::size_t index = 0;
    for (const nlohmann::json& node : blueprint["chain"]) {
        const std::string path =
            source + ".chain[" + std::to_string(index) + "]";
        if (node.is_object() && node.contains("name") &&
            node["name"].is_string() &&
            !rootNames.insert(node["name"].get<std::string>()).second) {
            throw BlueprintValidationError(
                source + " contains duplicate root name: " +
                node["name"].get<std::string>());
        }
        validateNode(node, path, 1, nodeCount, limits, true);
        ++index;
    }
}
