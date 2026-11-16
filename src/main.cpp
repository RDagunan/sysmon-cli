#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;

int main() {
    ifstream uptime_file("/proc/uptime");

    if (!uptime_file) {
        cerr << "Error: cannot open /proc/uptime" << endl;
        return 1;
    }

    double uptime_seconds = 0.0;

    if (!(uptime_file >> uptime_seconds)) {
        cerr << "Error: cannot read system uptime" << endl;
        return 1;
    }

    int total_seconds = static_cast<int>(uptime_seconds);
    int uptime_hours = total_seconds / 3600;
    int uptime_mins = (total_seconds % 3600) / 60;
    int uptime_secs = total_seconds % 60;

    cout << "System uptime: " << uptime_hours << "h:"
         << uptime_mins << "m:"
         << uptime_secs << "s" << endl;

    ifstream memory_file("/proc/meminfo");

    if (!memory_file) {
        cerr << "Error: cannot open /proc/meminfo" << endl;
        return 1;
    }

    long long mem_total_kb = -1;
    long long mem_available_kb = -1;
    string line;

    while (getline(memory_file, line)) {
        istringstream fields(line);
        string field_name;
        long long value_kb;

        if (!(fields >> field_name >> value_kb)) {
            continue;
        }

        if (field_name == "MemTotal:") {
            mem_total_kb = value_kb;
        } else if (field_name == "MemAvailable:") {
            mem_available_kb = value_kb;
        }
    }

    if (mem_total_kb < 0 || mem_available_kb < 0) {
        cerr << "Error: missing memory information" << endl;
        return 1;
    }

    cout << "Memory: " << mem_available_kb / 1024
         << " MiB available / " << mem_total_kb / 1024
         << " MiB total" << endl;

    return 0;
}
