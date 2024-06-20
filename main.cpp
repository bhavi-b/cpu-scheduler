#include <vector>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <chrono>
#include <thread>

// Define a Process struct to hold process data
struct Process {
    int id;
    double burst_time;
    double arrival_time;
    int priority;
    double burst_time_priority;
    double f;
    double fRank;
};


// Declare global variables
std::vector<Process> readyQueue;
std::vector<Process> arrangedReadyQueue;
double timeQuanta;
bool stop_flag = false;
double avgWaitingTimeNew = 0;
double avgTurnAroundTimeNew = 0;
double avgResponseTimeNew = 0;
std::vector<std::tuple<int, double, double>> ganttProposed;
double completionTimeNew = 0;

std::vector<Process> processes = {
    {1, 80, 0, 1},
    {2, 60, 20, 2},
    {3, 65, 40, 3},
    {4, 120, 60, 4},
    {5, 30, 80, 5},
    {6, 90, 90, 6},
    {7, 25, 120, 7},
    {8, 40, 240, 8},
    {9, 90, 260, 9},
    {10, 75, 380, 10}
};

void addProcess(std::vector<Process>& processes, int burst_time, int arrival_time, int priority) {
    int id = processes.size() + 1;
    processes.push_back({id, burst_time, arrival_time, priority});
}

void editProcess(std::vector<Process>& processes, int id, int burst_time, int arrival_time, int priority) {
    for (auto& p : processes) {
        if (p.id == id) {
            p.burst_time = burst_time;
            p.arrival_time = arrival_time;
            p.priority = priority;
            break;
        }
    }
}

void removeProcess(std::vector<Process>& processes, int id) {
    processes.erase(std::remove_if(processes.begin(), processes.end(), [id](Process p) {
        return p.id == id;
    }), processes.end());
}

void displayTable(const std::vector<Process>& processes) {
    std::cout << "<table class=\"table table-hover m-0 p-3 bg-linear\" id=\"table\">";
    std::cout << "<thead><tr>"
              << "<th>Process Id</th>"
              << "<th>Burst Time</th>"
              << "<th>Arrival Time</th>"
              << "<th>Priority</th>"
              << "</tr></thead>";
    std::cout << "<tbody>";
    for (const auto& p : processes) {
        std::cout << "<tr>"
                  << "<td>" << p.id << "</td>"
                  << "<td>" << p.burst_time << "</td>"
                  << "<td>" << p.arrival_time << "</td>"
                  << "<td>" << p.priority << "</td>"
                  << "</tr>";
    }
    std::cout << "</tbody></table>";
}

// Function to initialize the ready queue
void readyQueueInit(const std::vector<Process>& processes) {
    readyQueue = processes;
}

// Function to calculate time quanta
void calculateTimeQuanta() {
    double sum = 0, temp, max = std::numeric_limits<double>::min();
    if (!readyQueue.empty()) {
        for (const auto& process : readyQueue) {
            temp = process.burst_time;
            sum += temp;
            if (temp > max) max = temp;
        }
        timeQuanta = std::sqrt(sum / readyQueue.size() * max);
    } else {
        timeQuanta = 0;
    }
}

// Function to calculate burst time priority
void calculateBurstTimePriority() {
    std::vector<double> duplicate;
    std::vector<bool> flag;
    for (const auto& process : readyQueue) {
        duplicate.push_back(process.burst_time);
        flag.push_back(false);
    }

    std::sort(duplicate.begin(), duplicate.end());

    for (auto& process : readyQueue) {
        for (size_t d = 0; d < duplicate.size(); ++d) {
            if (process.burst_time == duplicate[d] && !flag[d]) {
                process.burst_time_priority = d + 1;
                flag[d] = true;
                break;
            }
        }
    }
}

// Function to calculate F
void calculateF() {
    for (auto& process : readyQueue) {
        process.f = 1.0 * ((3 * process.priority) + process.burst_time_priority) / 4;
    }
}

// Function to calculate F rank
void calculateFRank() {
    std::vector<double> duplicate;
    std::vector<bool> flag;
    for (const auto& process : readyQueue) {
        duplicate.push_back(process.f);
        flag.push_back(false);
    }

    std::sort(duplicate.begin(), duplicate.end());

    for (auto& process : readyQueue) {
        for (size_t d = 0; d < duplicate.size(); ++d) {
            if (process.f == duplicate[d] && !flag[d]) {
                process.fRank = d + 1;
                flag[d] = true;
                break;
            }
        }
    }
}

// Function to sort by F rank
void sortByFRank() {
    while (!readyQueue.empty()) {
        double minRank = std::numeric_limits<double>::max();
        size_t j = 0;
        Process process;
        for (size_t p = 0; p < readyQueue.size(); ++p) {
            if (readyQueue[p].fRank < minRank) {
                minRank = readyQueue[p].fRank;
                process = readyQueue[p];
                j = p;
            }
        }
        arrangedReadyQueue.push_back(process);
        readyQueue.erase(readyQueue.begin() + j);
    }
}

// Function to get process by ID
Process getProcessById(const std::vector<Process>& processes, int id) {
    for (const auto& process : processes) {
        if (process.id == id) {
            return process;
        }
    }
    return Process(); // Return empty process if not found
}

// Function to calculate average time
double calculateAvgTime(const std::vector<double>& times) {
    double avg = 0;
    for (size_t i = 1; i < times.size(); ++i) {
        avg += times[i];
    }
    return avg / (times.size() - 1);
}

// Async function to run the new proposed algorithm
void newProposed(bool flag, const std::vector<Process>& processes) {
    readyQueueInit(processes);
    std::vector<double> turnAroundTime;
    std::vector<double> waitingTime;
    std::vector<double> completionTime;
    std::vector<double> responseTime;
    double time = 0;
    
    if (flag) {
        // Simulate visualization setup
        std::cout << "Visualization setup..." << std::endl;
    }
    
    while (!readyQueue.empty()) {
        calculateTimeQuanta();
        calculateBurstTimePriority();
        calculateF();
        calculateFRank();
        sortByFRank();

        while (!arrangedReadyQueue.empty()) {
            auto p = arrangedReadyQueue.front();
            arrangedReadyQueue.erase(arrangedReadyQueue.begin());
            double prev_time = time;

            if (p.burst_time == getProcessById(processes, p.id).burst_time) {
                responseTime[p.id] = prev_time;
            }
            
            if (p.burst_time > timeQuanta) {
                p.burst_time -= timeQuanta;
                time += timeQuanta;
                readyQueue.push_back(p);
            } else {
                time += p.burst_time;
                completionTime[p.id] = time;
                auto process = getProcessById(processes, p.id);
                waitingTime[p.id] = completionTime[p.id] - process.burst_time;
            }

            if (flag) {
                // Simulate visualization update
                std::cout << "Updating visualization..." << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(2));
                if (stop_flag) {
                    break;
                }
            }

            ganttProposed.push_back(std::make_tuple(p.id, prev_time, time));
        }
    }

    for (size_t i = 0; i < completionTime.size(); ++i) {
        turnAroundTime.push_back(completionTime[i]);
    }
    completionTimeNew = time;
    avgWaitingTimeNew = calculateAvgTime(waitingTime);
    avgTurnAroundTimeNew = calculateAvgTime(turnAroundTime);
    avgResponseTimeNew = calculateAvgTime(responseTime);
}

// Main function to demonstrate the code
int main() {
    std::cout << "Content-type: text/html\n\n";
    std::cout << "<html><head><title>CPU Scheduling Algorithm Visualiser</title>";
    std::cout << "<link rel=\"stylesheet\" href=\"/css/style.css\"></head><body>";

    std::cout << "<h1>CPU Scheduling Algorithm Visualiser</h1>";
    
    displayTable(processes);

    std::cout << "<h2>Operations</h2>";
    
    // Example operations
    std::cout << "<p>Adding a new process</p>";
    addProcess(processes, 50, 100, 11);
    displayTable(processes);

    std::cout << "<p>Editing process with ID 2</p>";
    editProcess(processes, 2, 70, 25, 2);
    displayTable(processes);

    std::cout << "<p>Removing process with ID 3</p>";
    removeProcess(processes, 3);
    displayTable(processes);

    FCFS(false);
    SJFNonPre(false);
    SJFPre(false);
    LJFNonPre(false);
    LJFPre(false);
    priorityNonPre(false);
    priorityPre(false);
    roundRobin(false);
    newProposed(false);

    std::cout << "</body></html>";

    return 0;
}




