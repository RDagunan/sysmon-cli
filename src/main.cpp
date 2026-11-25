#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <chrono>
#include <thread>
#include "sysmon/metrics.h"

using namespace std;




int print_snapshot() {

///////////////// UPTIME INFORMATION ///////////////////

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


    //////////////// MEMORY INFORMATION ///////////////////

    ifstream memory_file("/proc/meminfo");

    if (!memory_file) {
        cerr << "Error: cannot open /proc/meminfo" << endl;
        return 1;
    }

    long long mem_total_kb = -1;
    long long mem_available_kb = -1;

    if (!read_memory(memory_file, mem_total_kb, mem_available_kb)) {
        cerr << "Error: cannot read memory information" << endl;
        return 1;
    }

    cout << "Memory: " << mem_available_kb / 1024
         << " MiB available / " << mem_total_kb / 1024
         << " MiB total" << endl;


/////////////// CPU INFORMATION ///////////////////

    ifstream cpu_file("/proc/stat");

    if (!cpu_file) {
        cerr << "Error: cannot open /proc/stat" << endl;
        return 1;
    }

    long long total_ticks;
    long long nonbusy_ticks;

    if (!read_cpu(cpu_file, total_ticks, nonbusy_ticks)) {
        cerr << "Error: cannot read CPU counters" << endl;
        return 1;
    }

   this_thread::sleep_for(chrono::seconds(1));

    ifstream second_cpu_file("/proc/stat");

    if (!second_cpu_file) {
        cerr << "Error: cannot reopen /proc/stat" << endl;
        return 1;
    }

    long long second_total_ticks;
    long long second_nonbusy_ticks;

    if (!read_cpu(second_cpu_file, second_total_ticks, second_nonbusy_ticks)) {
        cerr << "Error: cannot read second CPU snapshot" << endl;
        return 1;
    }

    long long elapsed_ticks = second_total_ticks - total_ticks;
    long long nonbusy_change = second_nonbusy_ticks - nonbusy_ticks;

    double cpu_percent;

    if (!calculate_cpu_usage(elapsed_ticks, nonbusy_change, cpu_percent)) {
        cerr << "Error: inconsistent CPU readings" << endl;
        return 1;
    }

    cout << "CPU usage: " << cpu_percent << "%" << endl;

    return 0;
}

int main(int argc, char* argv[]) {
    bool watch_mode = false;

    if (argc == 2 && string(argv[1]) == "--watch") {
        watch_mode = true;
    } else if (argc != 1) {
        cerr << "Usage: sysmon [--watch]" << endl;
        return 1;
    }

    do {
        int status = print_snapshot();

        if (status != 0) {
            return status;
        }
    } while (watch_mode);

    return 0;
}