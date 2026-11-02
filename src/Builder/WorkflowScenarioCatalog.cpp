#include "WorkflowScenarioCatalog.h"

#include <algorithm>
#include <dirent.h>
#include <fstream>
#include <sstream>

namespace {

/**
 * @brief Checks whether a directory entry has the JSON blueprint extension.
 */
bool isJsonFile(const std::string& name) {
    const std::string extension = ".json";
    return name.size() > extension.size() &&
           name.compare(name.size() - extension.size(), extension.size(),
                        extension) == 0;
}

/**
 * @brief Builds a stable path for a discovered scenario.
 */
std::string joinPath(const std::string& directory, const std::string& name) {
    if (directory.empty() || directory[directory.size() - 1] == '/') {
        return directory + name;
    }
    return directory + "/" + name;
}

} // namespace

WorkflowScenarioCatalog::WorkflowScenarioCatalog(const std::string& directory)
    : directory(directory) {}

std::vector<std::string> WorkflowScenarioCatalog::refresh() {
    availableScenarios.clear();
    std::vector<std::string> errors;

    DIR* directoryHandle = opendir(directory.c_str());
    if (directoryHandle == nullptr) {
        errors.push_back("cannot open scenario directory: " + directory);
        return errors;
    }

    while (dirent* entry = readdir(directoryHandle)) {
        const std::string filename(entry->d_name);
        if (filename == "." || filename == ".." || !isJsonFile(filename)) {
            continue;
        }

        const std::string path = joinPath(directory, filename);
        try {
            const nlohmann::json blueprint =
                WorkflowBlueprint::loadAndValidate(path);
            WorkflowScenario scenario;
            scenario.id = path;
            scenario.label = blueprint["workflow_name"].get<std::string>();
            scenario.path = path;
            availableScenarios.push_back(scenario);
        } catch (const BlueprintValidationError& error) {
            errors.push_back(error.what());
        }
    }

    closedir(directoryHandle);

    std::sort(availableScenarios.begin(), availableScenarios.end(),
              [](const WorkflowScenario& left,
                 const WorkflowScenario& right) {
                  return left.path < right.path;
              });
    return errors;
}

const std::vector<WorkflowScenario>& WorkflowScenarioCatalog::scenarios() const {
    return availableScenarios;
}

bool WorkflowScenarioCatalog::selectFromConsole(
    std::istream& input,
    std::ostream& output,
    WorkflowScenario& selected) const {
    if (availableScenarios.empty()) {
        output << "No valid workflow scenarios are available.\n";
        return false;
    }

    output << "\nAvailable workflow scenarios:\n";
    for (std::size_t index = 0; index < availableScenarios.size(); ++index) {
        output << (index + 1) << ". " << availableScenarios[index].label
               << "\n";
    }
    output << "Select a scenario (1-" << availableScenarios.size() << "): ";

    std::size_t choice = 0;
    if (!(input >> choice) || choice < 1 ||
        choice > availableScenarios.size()) {
        output << "Invalid scenario selection.\n";
        return false;
    }

    selected = availableScenarios[choice - 1];
    return true;
}
