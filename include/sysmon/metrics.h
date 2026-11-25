#ifndef SYSMON_METRICS_H
#define SYSMON_METRICS_H

#include <istream>

bool read_memory(std::istream& input,
                 long long& total_kb,
                 long long& available_kb);



bool read_cpu(std::istream& input,
              long long& total_ticks,
              long long& nonbusy_ticks);



bool calculate_cpu_usage(long long elapsed_ticks,
                         long long nonbusy_change,
                         double& cpu_percent);


#endif