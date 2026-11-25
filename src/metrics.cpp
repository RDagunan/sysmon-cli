#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include "sysmon/metrics.h"

using namespace std;

bool read_memory(istream& input,
    long long& total_kb,
    long long& available_kb) {
    total_kb = -1;
    available_kb = -1;
    string line;

    while (getline(input, line)) {
        istringstream fields(line);
        string field_name;
        long long value_kb;

        if (!(fields >> field_name >> value_kb)) {
            continue;
        }

        if (field_name == "MemTotal:") {
            total_kb = value_kb;
        } else if (field_name == "MemAvailable:") {
            available_kb = value_kb;
        }
    }

    return total_kb >= 0 && available_kb >= 0;
}

bool read_cpu(istream& input, long long& total_ticks, long long& nonbusy_ticks){
    total_ticks = -1;
    nonbusy_ticks = -1;

    string line;
    if(!getline(input, line)){
        return false;
    }

    istringstream fields(line);
    string label;
    long long user, nice, system, idle;
    long long iowait, irq, softirq, steal;

    if(!(fields >> label >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal)){
        return false;
    }

    if(label != "cpu"){
        return false;
    }

    total_ticks = user + nice + system + idle + iowait + irq + softirq + steal;
    nonbusy_ticks = idle + iowait;

    return true;
}

bool calculate_cpu_usage(long long elapsed_ticks,
                         long long nonbusy_change,
                         double& cpu_percent) {
    cpu_percent = -1.0;

    if (elapsed_ticks <= 0 || nonbusy_change < 0 || nonbusy_change > elapsed_ticks) {
        return false;
    }

    long long busy_ticks = elapsed_ticks - nonbusy_change;
    cpu_percent = 100.0 * busy_ticks / elapsed_ticks;

    return true;
}