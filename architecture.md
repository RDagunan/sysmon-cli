# SysMon CLI architecture and roadmap

SysMon answers a question from the Linux terminal: how long has this machine been running, and how much memory is available? The first working version is saved in commit `0f3e654`. The current source also reports CPU usage measured over a roughly one-second interval.

## Follow one run

Linux exposes live measurements through files under `/proc`. Running `./build/sysmon` starts `main()`, which prints uptime and memory, reads CPU counters from /proc/stat twice about one second apart, prints CPU usage, and exits. You can trace the entire run in [src/main.cpp](src/main.cpp).

| Step | What the code reads | What the user sees |
| --- | --- | --- |
| 1. Uptime | The first number in `/proc/uptime` gives seconds since boot. Drop the fraction and use division and remainder to split whole seconds into hours, minutes, and seconds. | `System uptime: 0h:23m:53s` for an input of 1433.09 seconds. |
| 2. Memory | Scan `/proc/meminfo` one line at a time for `MemAvailable:` and `MemTotal:`. Divide each value by 1024 to get whole MiB. | `Memory: 26286 MiB available / 32013 MiB total` for one sample. |
| 3. CPU | Read /proc/stat twice about one second apart and compute busy time from the differences. | `CPU usage: 1.50094%` in the user's run. |

Each step checks its input before printing. If all succeed, the program exits with status 0. If a required file or value cannot be read, it writes an error to standard error and exits with status 1. Because uptime prints first, someone can still see the uptime line if the memory read fails. Successful runs have been verified. The error handling has been reviewed in source and still needs runtime checks.

## Where changes belong

- **Today:** `src/main.cpp` owns the file reads, calculations, and output. [CMakeLists.txt](CMakeLists.txt) builds that source as the C++17 executable `sysmon`. The generated executable lives under `build/`.
- **As parsing grows:** give a calculation or parser its own function when you need to check it with a known input. For example, a memory parser could read from a stream so the same code accepts a real file during a run and a short piece of sample text during a test.
- **When a function needs its own file:** put the implementation in `src/`, declarations shared by multiple source files in `include/sysmon/`, and checks in `tests/`. List any new compiled source in CMake. These directories describe where future code can go.

## Proposed path to the next version

| Milestone | Work | How to tell it is done |
| --- | --- | --- |
| 1. Build the first snapshot — complete | Build with CMake and print uptime and available/total RAM. | The committed executable source builds and both lines appear on Linux. See [SESSION_LOG.md](SESSION_LOG.md) for the recorded runs. |
| 2. Check the existing parsers — complete | Checked both memory field orders, a missing available field, and uptime at 3600 seconds. Removed temporary test code afterward. | Screenshots show both field orders returning the same values and a missing field returning false. The user reported `1h:0m:0s` at the hour boundary. These are manual checks. |
| 3. Add CPU usage — complete | Read the aggregate `cpu` line from `/proc/stat` twice, about one second apart. The counters accumulate since boot. Calculate the share of elapsed CPU time spent busy from the differences between samples. | A known pair of samples with 25 busy ticks out of 100 elapsed gives 25%. A zero change in total ticks is handled before division. A live run prints a labeled CPU percentage. |
| 4. Verify the complete snapshot — live build and run complete | Build in a fresh directory and run all three measurements together. Document the CPU sampling delay, output, and run command. | Someone following the README can build the program, read uptime, RAM, and CPU usage in a single run, and understand what interval the CPU value covers. |

**Current checkpoint:** the known-input 25% case and zero-change guard have passed, and live CPU sampling is restored after removing temporary assignments. File/parse failures and the other consistency conditions remain untested. Review and commit this milestone before selecting the next feature.

After the three-measurement snapshot works, a possible extension is a watch mode that repeats the readings until Ctrl+C. Whether to add that mode and how often to refresh are open decisions. Record a choice in [decisions.md](decisions.md) when it is adopted.
