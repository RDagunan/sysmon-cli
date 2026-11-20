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
