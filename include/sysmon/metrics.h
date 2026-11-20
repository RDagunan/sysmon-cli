#ifndef SYSMON_METRICS_H
#define SYSMON_METRICS_H

#include <istream>

bool read_memory(std::istream& input,
                 long long& total_kb,
                 long long& available_kb);

#endif