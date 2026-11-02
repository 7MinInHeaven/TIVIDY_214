#ifndef WORKFLOWSCENARIOCATALOG_H
#define WORKFLOWSCENARIOCATALOG_H

#include <iosfwd>
#include <string>
#include <vector>

#include "WorkflowBlueprint.h"

/**
 * @brief Describes one validated workflow scenario available to the application.
 */
struct WorkflowScenario {
    std::string id;
    std::string label;
    std::string path;
};

/**
 * @brief Discovers validated JSON workflow scenarios and supports console selection.
 */
class WorkflowScenarioCatalog {
public:
    /**
     * @brief Creates a catalog for a directory containing blueprint files.
     * @param directory Directory scanned for JSON files.
     */
    explicit WorkflowScenarioCatalog(const std::string& directory = "src");

    /**
     * @brief Scans the configured directory and validates its JSON files.
     * @return Errors encountered while skipping invalid files.
     */
    std::vector<std::string> refresh();

    /**
     * @brief Returns the scenarios found by the last refresh.
     */
    const std::vector<WorkflowScenario>& scenarios() const;

    /**
     * @brief Displays a menu and reads one scenario selection.
     * @param input Stream used to read the user's choice.
     * @param output Stream used to display the menu and errors.
     * @param selected Receives the selected scenario.
     * @return True when a valid scenario was selected.
     */
    bool selectFromConsole(std::istream& input,
                           std::ostream& output,
                           WorkflowScenario& selected) const;

private:
    std::string directory;
    std::vector<WorkflowScenario> availableScenarios;
};

#endif
