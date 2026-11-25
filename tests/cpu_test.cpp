#include "sysmon/metrics.h"
#include <iostream>
#include <sstream>

int main() {
    std::istringstream sample("cpu 10 0 15 70 5 0 0 0 0 0\n");

    long long total_ticks;
    long long nonbusy_ticks;

    if (!read_cpu(sample, total_ticks, nonbusy_ticks)) {
        std::cerr << "FAIL: could not read CPU sample\n";
        return 1;
    }

    if (total_ticks != 100 || nonbusy_ticks != 75) {
        std::cerr << "FAIL: incorrect CPU totals\n";
        return 1;
    }

    std::istringstream wrong_label("cpu0 10 0 15 70 5 0 0 0 0 0\n");

    if (read_cpu(wrong_label, total_ticks, nonbusy_ticks)
        || total_ticks != -1 || nonbusy_ticks != -1) {
        std::cerr << "FAIL: incorrect CPU label handling\n";
        return 1;
    }

    std::istringstream incomplete("cpu 10 0 15\n");

    if (read_cpu(incomplete, total_ticks, nonbusy_ticks)
        || total_ticks != -1 || nonbusy_ticks != -1) {
        std::cerr << "FAIL: incomplete CPU counter handling\n";
        return 1;
    }

    double cpu_percent;

    if (!calculate_cpu_usage(100, 75, cpu_percent) || cpu_percent != 25.0) {
        std::cerr << "FAIL: expected 25 percent CPU usage\n";
        return 1;
    }

    if (calculate_cpu_usage(0, 0, cpu_percent) || cpu_percent != -1.0) {
        std::cerr << "FAIL: zero elapsed time was not rejected correctly\n";
        return 1;
    }

    if (calculate_cpu_usage(100, -1, cpu_percent) || cpu_percent != -1.0) {
        std::cerr << "FAIL: negative nonbusy change was not rejected\n";
        return 1;
    }

    if (calculate_cpu_usage(100, 101, cpu_percent) || cpu_percent != -1.0) {
        std::cerr << "FAIL: excessive nonbusy change was not rejected\n";
        return 1;
    }

    std::cout << "All CPU checks passed\n";

    return 0;
}