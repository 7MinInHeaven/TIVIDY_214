#include "WorkVisitor.h"

ProgressReportVisitor::ProgressReportVisitor() : output("report.html"){
    output  << "<!DOCTYPE html>\n"
            << "<html lang=\"en\">\n"
            << "<head>\n"
            << "\t<meta charset=\"UTF-8\">\n"
            << "\t<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
            << "\t<title>Progress Report</title>\n"
            << "\t<link rel=\"stylesheet\ href=\"style.css\">\n"
            << "</head>\n";
}

ProgressReportVisitor::~ProgressReportVisitor(){
    if(output.is_open()){
        output.close();
    }
}

void ProgressReportVisitor::visit(WorkGroup& item){
    std::vector<WorkComponent*> children = item.getChildren();

    int total = static_cast<int>(children.size());
    int done;
    for(WorkComponent* child : children){
        if(child->isComplete()){
            ++done;
        }
    }

    output  << "<details><summary> " << item.getName() << writeProgressCircle(progress) << " </summary>\n"
            << writeChildren(children)
            << "</details>\n";

}

void ProgressReportVisitor::visit(WorkUnit& item){
    output << item.getName() << "\n";
}


void ProgressReportVisitor::visit(PrioritisedTask& item){
    output  << item.getName() 
            << "PRIORITY: " << getPriorityString(item.getPriority());
    if(item.hasDeadline()){
        if(item.isOverdue()){
            output << "OVERDUE";
        } else {
            output << formatSeconds(item.getSecondsRemaining());
        }
    }
    output << "\n";
}

void ProgressReportVisitor::visit(AuditedTask& item){
    output  << item.getName()
            << writeLog(item.getLog()); //need function to get log
}

void ProgressReportVisitor::visit(ValidatedTask& item){
    output  << item.getName()
            << writeRules(item.getRules()); // need function to getRules
}

std::string ProgressReportVisitor::writeProgressCircle(int done, int total){
    std::ostringstream output;
    int progress = (total > 0) ? (done* 100) / total : 0;
    
    output  << "<div class=\"progress-circle\" style=\"--progress: " << progress << ";\" role=\"progressbar\" value>\n"
            << "\t<span class=\"progress-value\">" << progress << "%</span>\n"
            << "\t<span class=\"done-total\">" << done << "/" << total << "</span>\n"
            << "</div>";

    return output.str();
}

std::string ProgressReportVisitor::writeDoneCheck(bool complete){
    std::ostringstream output;
    complete ? output << "<span class=\"task-complete\">✔</span>" : output << "";

    return output.str();
}

std::string ProgressReportVisitor::writeChildren(std::vector<WorkComponent*>& children){
    std::ostringstream output;
    for(WorkComponent* child : children){

        child->accept(*this);
    }
}

std::string ProgressReportVisitor::writeLog(AuditLog& log){
    std::ostringstream output;
    output << "<summary><details>Log</details>\n";

    const std::vector<AuditEntry>& entries = log.getEntries();
    for(size_t i = 0; i < entries.size(); i++){
        const AuditEntry& entry = entries[i];
        output  << "\t<ul>" << formatTime(entry.timestamp) << " | "
                << std::to_string(entry.sequenceNumber) << " | "
                << entry.message
                << "</ul>\n";
    }

    output << "</summary>\n";
    return output.str();
}

std::string ProgressReportVisitor::writeRules(std::vector<std::shared_ptr<const ValidationRule>>& rules){
    std::ostringstream output;
    output << "<summary><details>Log</details>\n";

    for(std::shared_ptr<const ValidationRule> rule : rules){ //need to make sure this works with shared pointers
        output << "\t<ul>" << rule->getMessage() << "</ul>\n";
    }

    output << "</summary>\n";
    return output.str();
}

void ProgressReportVisitor::createReport(WorkComponent* root){
    output << "<body>";
        root->accept(*this);
    output << "</body>";

    output.close();
}

std::string ProgressReportVisitor::getPriorityString(PriorityLevel priority){
    switch(priority){
        case PriorityLevel::Low: return "Low";
        case PriorityLevel::Normal: return "Normal";
        case PriorityLevel::High: return "High";
        case PriorityLevel::Urgent: return "Urgent";
        default: return "Unknown"
    }
}

std::string ProgressReportVisitor::formatTime(std::time_t timestamp){
    std::tm* localTime = std::localtime(&timestamp);  
    std::ostringstream oss;
    oss << std::put_time(localTime, "%YY-%mm-%dd %H:%M-%S");
    return oss.str();
}

std::string ProgressReportVisitor::formatSeconds(long totalSeconds){

    //I'm going to not have weeks and years because I don't think we need it, will check though
    long days = totalSeconds / 86000;
    long hours = (totalSeconds % 86000) / 3600;
    long minutes = (totalSeconds % 3600) / 60;
    long seconds = totalSeconds % 60;

    std::string format;
    if (days) format += std::to_string(days) + "d ";
    if (days || hours) format += std::to_string(hours) + "h ";
    if (days || hours || minutes) format += std::to_string(minutes) + "m ";

    format += std::to_string(seconds) = "s";

    return format;
}