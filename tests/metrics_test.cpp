#include "sysmon/metrics.h"
#include <iostream>
#include <sstream>

int main() {
    std::istringstream sample(
        "MemTotal: 8192 kB\n"
        "MemAvailable: 4096 kB\n"
    );

    long long total_kb;
    long long available_kb;

    if (!read_memory(sample, total_kb, available_kb)) {
        std::cerr << "FAIL: could not read the sample\n";
        return 1;
    }

    if (total_kb != 8192 || available_kb != 4096) {
        std::cerr << "FAIL: incorrect memory values\n";
        return 1;
    }

    std::istringstream reversed_sample(
    "MemAvailable: 4096 kB\n"
    "MemTotal: 8192 kB\n"
    );

    if (!read_memory(reversed_sample, total_kb, available_kb)
        || total_kb != 8192 || available_kb != 4096) {
        std::cerr << "FAIL: reversed field order\n";
        return 1;
    }

    std::istringstream missing_sample("MemTotal: 8192 kB\n");

    if (read_memory(missing_sample, total_kb, available_kb)
        || total_kb != 8192 || available_kb != -1) {
        std::cerr << "FAIL: missing available-memory field\n";
        return 1;
    }

    std::cout << "All Memory parser checks passed\n";

    return 0;
}