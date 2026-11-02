#include "WorkVisitor.h"
#include "Composite.h"
#include "Decorator.h"

ProgressReportVisitor::ProgressReportVisitor(std::string filename) : outputFile(filename + ".html"){
    if(outputFile.is_open()){
        outputFile  << "<!DOCTYPE html>\n"
            << "<html lang=\"en\">\n"
            << "<head>\n"
            << "\t<meta charset=\"UTF-8\">\n"
            << "\t<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
            << "\t<title>Progress Report</title>\n";
            writeCSS();
        outputFile << "</head>\n";
    }
}

void ProgressReportVisitor::writeCSS(){

static const char* const css = R"CSS(
    body{
  font-family: monospace;
}

/* Progress tracker */
.progress-circle {
    --size: 40px;                 
    --thickness: 4px;
    --track: #e0e0e0;
    --fill: #76e5b1;
    --progress: 0%;                

    position: relative;
    display: inline-flex;
    flex-direction: column;   
    align-items: center;
    justify-content: center;

    width: var(--size);
    height: var(--size);
    border-radius: 50%;
}

.progress-circle::before {
    content: "";
    position: absolute;
    inset: 0;
    border-radius: 50%;
    
    background: conic-gradient(var(--fill) var(--progress), var(--track) 0);
    mask: radial-gradient(farthest-side, transparent calc(100% - var(--thickness)), #000 calc(100% - var(--thickness) + 1px));
} 

.progress-value,
.done-total {
    position: relative;           
    text-align: center;
}

.progress-value { font-size: calc(var(--size) * 0.30); font-weight: 700; }
.done-total     { font-size: calc(var(--size) * 0.25); font-weight: 600; color: #818386; margin: 0; }

/* Priority */
.priority-color,
.overdue,
.deadline{
  color: #fff;
  padding: 2px 8px;
  border-radius: 8px;
  font-weight: 600;;
  margin-left: 4px;
}

.priority-color[data-value="0"]{
  background-color: #00796b;
}
.priority-color[data-value="1"]{
  background-color: #2e7d32;;
}
.priority-color[data-value="2"]{
  background-color: #b45309;
}
.priority-color[data-value="3"]{
  background-color: #c62828;
}
.overdue{
  background-color: #7f0000;
}
.deadline{
  background-color: #37474f;
}

.task-complete{
    color: green;
}

/* tables */
.log-table{
  width:100%;
  border-collapse: collapse;
}

.log-table th,
.log-table td{
  padding: 6px 10px;
  border: 1px solid #ccc;
  text-align: left;
}

.log-table thead th {
  background: #222;
  color:white;
}
.log-table tbody tr:nth-child(even){
  background: #f5f5f5;
}

.log-table tbody tr:hover,
.log-table tbody tr:focus-visible{
  background: #eee;
}

/* rules */
.rules{
  list-style: none;
  margin:0;
  padding:0;
  max-width: 400px;
  border: 1px solid #ccc;
  border-radius: 6px;
  overflow: hidden;
}

.rules li + li {
  border-top: 1px solid #ccc;
}

.rules li:hover,
.rules li:focus-visible{
  background: #eee;
}

.name{
  display: flex;
  align-items: center;
}

/* Group */
summary {
  list-style:none;
  cursor:pointer;
  user-select: none;
  display:flex;
  align-items: center;

  gap: 8px;
  padding: 8px 12px;
  border-radius: 6px;
}

summary::before{
  content: "";
  flex:none;
  width: 0.5em;
  height: 0.5em;  
  border-right: 2px solid currentColor;
  border-bottom: 2px solid currentColor;
  transform: rotate(-45deg);
  transition: transform 0.15s ease;
}

details[open] > summary::before{
  transform: rotate(45deg);
}

summary::hover{
  background-color: #eee;
}

summary:focus-visible {
  outline: 2px solid #3b82f6;
  outline-offset: 2px;
}

details.group {
  border: 1px solid #ccc;
  border-radius: 8px;
  background: #eee;
}

details.group > summary{
  font-weight: 600;
}

details.group[open] > summary {
  border-bottom: 1px solid #fafafa;
  border-bottom-left-radius: 0;
  border-bottom-right-radius: 0;
}

details.group > summary thead {
  display: flex;
  flex: 1;
  flex-wrap: wrap;
  align-items: center;
  gap: 8px;
}

details.group > summary .progress-circle {
  margin-left: auto;
}

details.group > ul {
  list-style: none;
  margin: 0;
  padding: 8px 12px 8px 20px;
  display:grid;
  gap: 6px;
}

details:not(.group){
  margin: 4px 0 4px 16px;
}

details:not(.group) > summary {
  display: inline-flex;
  padding: 2px 8px;
  font-size: 0.9em;
  border: 1px solid #ccc;
  background: #fafafa;
}

details:not(.group)[open] > summary {
  margin-bottom: 6px;
}

.rules {
  margin: 0;
  padding-left: 1.4em;
}
    )CSS";

outputFile << "\t<style>" << css << "</style>\n";

}

ProgressReportVisitor::~ProgressReportVisitor(){
    if(outputFile.is_open()){
        outputFile.close();
    }
}

void ProgressReportVisitor::visit(const WorkGroup& item){
    visitGroup(item);
}
void ProgressReportVisitor::visit(const Stage& item){
    visitGroup(item);
}
void ProgressReportVisitor::visit(const SubProcess& item){
    visitGroup(item);
}

void ProgressReportVisitor::visit(const WorkUnit& item){
    PriorityLevel priority = pendingPriority;
    pendingPriority = PriorityLevel::Normal;
    outputFile << item.getName() << writePriority(priority) << writeDoneCheck(item.isComplete()) << details << badges;

    badges.clear();
    details.clear();
}


void ProgressReportVisitor::visit(const PrioritisedTask& item){
    pendingPriority = item.getPriority();
    if(item.hasDeadline()){
        if(item.isOverdue()){
            badges += "<span class=\"overdue\">OVERDUE</span>";
        } else {
            badges += "<span class=\"deadline\">" + formatSeconds(item.getSecondsRemaining()) + "</span>";
        }
    }
    item.getWrappedTask()->accept(*this);
}

void ProgressReportVisitor::visit(const AuditedTask& item){
    details += writeLog(item.getLog());
    item.getWrappedTask()->accept(*this);
}

void ProgressReportVisitor::visit(const ValidatedTask& item){
    details += writeRules(item.getRules());
    item.getWrappedTask()->accept(*this);
}



void ProgressReportVisitor::visitGroup(const WorkGroup& item){
    std::vector<WorkComponent*> children = item.getChildren();

    std::string myBadges = badges;
    std::string myDetails = details;
    PriorityLevel myLevel = pendingPriority;
    pendingPriority = PriorityLevel::Normal;
    badges.clear();
    details.clear();

    int total = static_cast<int>(children.size());
    int done = 0;
    for(WorkComponent* child : children){
        if(child->isComplete()){
            ++done;
        }
    }


    outputFile  << "<details class=\"group\">\n"
                << "<summary>"
                << "\t<span class=\"name\">" << item.getName() 
                << writePriority(myLevel) << writeDoneCheck(item.isComplete())
                << myBadges << writeProgressCircle(done, total)
                << "</span>\n"
                << "</summary>\n"
                << myDetails;
    writeChildren(children);
    outputFile  << "</details>\n";
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

void ProgressReportVisitor::writeChildren(std::vector<WorkComponent*>& children){

    outputFile << "<ul>\n";
    for(WorkComponent* child : children){
        outputFile << "<li>";
        child->accept(*this);
        outputFile << "</li>\n";
    }
    outputFile << "</ul>\n";
}

std::string ProgressReportVisitor::writePriority(PriorityLevel level){
    std::ostringstream output;
    output << "<span class=\"priority-color\" data-value=\"" << static_cast<int>(level) << "\">" << getPriorityString(level)
            << "</span>";

    return output.str();
}

std::string ProgressReportVisitor::writeLog(const AuditLog* log){
    if(!log){
        return "";
    }
    std::ostringstream output;

    output << "<details><summary>Log</summary>\n";
    output  << "<table class=\"log-table\"\n>"
            << "\t<thead>\n"
            << "\t\t<tr>\n"
            << "\t\t\t<th>ID</th>\n"
            << "\t\t\t<th>Timestamp</th>\n"
            << "\t\t\t<th>Message</th>\n"
            << "\t\t</tr>\n"
            << "\t</thead>\n";
    
    output << "\t<tbody>\n";
    
    const std::vector<AuditEntry>& entries = log->getEntries();
    for(size_t i = 0; i < entries.size(); i++){
        const AuditEntry& entry = entries[i];
        output  << "\t\t<tr>\n"
                <<  "<td>" << std::to_string(entry.sequenceNumber) << "</td>\n"
                <<  "<td>" << formatTime(entry.timestamp) << "</td\n>"
                <<  "<td>" << entry.message << "</td\n>"
                << "\t\t</tr>\n";
    }
    output  << "\t</tbody>\n"
            << "</table>\n"
            << "</details>\n";
    return output.str();
}

std::string ProgressReportVisitor::writeRules(const std::vector<std::shared_ptr<const ValidationRule>>& rules){
    std::ostringstream output;
    output  << "<details><summary>Rules</summary>\n" 
            << "\t<ul class=\"rules\">\n";
    for(std::shared_ptr<const ValidationRule> rule : rules){ //need to make sure this works with shared pointers
        output << "\t\t<li>" << rule->getMessage() << "</li>\n";
    }
    output  << "\t</ul>\n"
            << "</details>\n";
    return output.str();
}

void ProgressReportVisitor::createReport(WorkComponent* root){
    if(root){
        outputFile << "<body>";
            root->accept(*this);
        outputFile << "</body>\n</html>";
    }
    outputFile.close();
}

std::string ProgressReportVisitor::getPriorityString(PriorityLevel priority){
    switch(priority){
        case PriorityLevel::Low: return "Low";
        case PriorityLevel::Normal: return "Normal";
        case PriorityLevel::High: return "High";
        case PriorityLevel::Urgent: return "Urgent";
        default: return "Unknown";
    }
}

std::string ProgressReportVisitor::formatTime(std::time_t timestamp){
    std::tm* localTime = std::localtime(&timestamp);  
    std::ostringstream oss;
    oss << std::put_time(localTime, "%Y-%m-%d %H:%M-%S");
    return oss.str();
}

std::string ProgressReportVisitor::formatSeconds(long totalSeconds){

    //I'm going to not have weeks and years because I don't think we need it, will check though
    long days = totalSeconds / 86400;
    long hours = (totalSeconds % 86400) / 3600;
    long minutes = (totalSeconds % 3600) / 60;
    long seconds = totalSeconds % 60;

    std::string format;
    if (days) format += std::to_string(days) + "d ";
    if (days || hours) format += std::to_string(hours) + "h ";
    if (days || hours || minutes) format += std::to_string(minutes) + "m ";

    format += std::to_string(seconds) + "s";

    return format;
}