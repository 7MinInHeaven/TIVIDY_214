#ifndef WORKFLOWBLUEPRINT_H
#define WORKFLOWBLUEPRINT_H

#include <cstddef>
#include <stdexcept>
#include <string>

#include <nlohmann/json.hpp>

/**
 * @brief Reports a blueprint that does not match the supported schema.
 */
class BlueprintValidationError : public std::runtime_error {
public:
    explicit BlueprintValidationError(const std::string& message);
};

/**
 * @brief Defines safety limits applied while validating a blueprint.
 */
struct BlueprintLimits {
    std::size_t maxFileBytes;
    std::size_t maxDepth;
    std::size_t maxNodes;
    std::size_t maxInputs;
    std::size_t maxCapabilities;
    std::size_t maxNameLength;

    BlueprintLimits();
};

/**
 * @brief Loads and validates runtime workflow blueprint files.
 */
class WorkflowBlueprint {
public:
    /**
     * @brief Loads a JSON blueprint from disk and validates it.
     * @param path Path to the blueprint file.
     * @param limits Maximum permitted blueprint dimensions.
     * @return The validated JSON blueprint.
     */
    static nlohmann::json loadAndValidate(
        const std::string& path,
        const BlueprintLimits& limits = BlueprintLimits());

    /**
     * @brief Validates an already parsed workflow blueprint.
     * @param blueprint Parsed JSON blueprint.
     * @param source Source name used in validation errors.
     * @param limits Maximum permitted blueprint dimensions.
     */
    static void validate(
        const nlohmann::json& blueprint,
        const std::string& source = "<memory>",
        const BlueprintLimits& limits = BlueprintLimits());
};

#endif
