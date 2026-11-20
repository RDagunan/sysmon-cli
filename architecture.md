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

- **Today:** `src/main.cpp` owns application flow, CPU parsing, file reads, calculations, and output. The memory parser now lives in `src/metrics.cpp`, declared in `include/sysmon/metrics.h`. [CMakeLists.txt](CMakeLists.txt) builds that source as the C++17 executable `sysmon`. The generated executable lives under `build/`.
- **As parsing grows:** give a calculation or parser its own function when you need to check it with a known input. For example, a memory parser could read from a stream so the same code accepts a real file during a run and a short piece of sample text during a test.
- **When a function needs its own file:** put the implementation in `src/`, declarations shared by multiple source files in `include/sysmon/`, and checks in `tests/`. List any new compiled source in CMake. These directories describe where future code can go.

## Proposed path to the next version

| Milestone | Work | How to tell it is done |
| --- | --- | --- |
| 1. Build the first snapshot — complete | Build with CMake and print uptime and available/total RAM. | The committed executable source builds and both lines appear on Linux. See [SESSION_LOG.md](SESSION_LOG.md) for the recorded runs. |
| 2. Check the existing parsers — complete | Checked both memory field orders, a missing available field, and uptime at 3600 seconds. Removed temporary test code afterward. | Screenshots show both field orders returning the same values and a missing field returning false. The user reported `1h:0m:0s` at the hour boundary. These are manual checks. |
| 3. Add CPU usage — complete | Read the aggregate `cpu` line from `/proc/stat` twice, about one second apart. The counters accumulate since boot. Calculate the share of elapsed CPU time spent busy from the differences between samples. | A known pair of samples with 25 busy ticks out of 100 elapsed gives 25%. A zero change in total ticks is handled before division. A live run prints a labeled CPU percentage. |
| 4. Verify the complete snapshot — live build and run complete | Build in a fresh directory and run all three measurements together. Document the CPU sampling delay, output, and run command. | Someone following the README can build the program, read uptime, RAM, and CPU usage in a single run, and understand what interval the CPU value covers. |

**Current checkpoint:** watch mode is implemented. The default run and unknown-option exit status have screenshot evidence, and the user confirmed repeated measurements and stopping with Ctrl+C. Watch-mode changes are saved in commit 1e9ed3a. The user reported pushing it, and local main matches the tracking ref origin/main. File/parse failures and the remaining CPU consistency conditions lack runtime evidence.

The earlier three-measurement milestone is committed as ee5bc5a and pushed to RDagunan/sysmon-cli. The current source puts one measurement run in print_snapshot(). main() selects single-run mode or repeated --watch mode, rejects invalid arguments, and propagates snapshot errors. Watch mode appends output and uses the CPU sampling pause to pace each cycle. Error propagation is reviewed in source; a snapshot failure during watch mode has not been triggered. See CODE_GUIDE.md for the code explanations.

## Watch-mode milestone

The learner extracted print_snapshot, added argc/argv parsing and a do/while loop, verified the default run returns 0, and verified an unknown option returns 1. The user reported that watch mode repeated all three measurements and stopped with Ctrl+C. The README documents both commands. This milestone is committed as 1e9ed3a.

## Repeatable memory checks — complete

read_memory lives in src/metrics.cpp and is declared in include/sysmon/metrics.h. Both sysmon and metrics_test compile this implementation. CMake registers metrics_test with CTest under the name memory_parser.

The normal-order, reversed-order, and missing-available-field checks have passed in the test executable. The application was also rebuilt and run after the file split, preserving all three live measurements. These changes are ready for a checkpoint commit. CPU parser and percentage tests can follow in a future lesson. See CODE_GUIDE.md for the code and explanations.
