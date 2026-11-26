# SysMon CLI

SysMon CLI is a Linux system monitor written in C++17. Run `sysmon` to see system uptime, available and total RAM, and aggregate CPU usage. It reads uptime from `/proc/uptime` and memory values from `/proc/meminfo`, samples CPU counters twice from `/proc/stat`, prints the measurements, and exits.

## Build and run

You need Linux with readable `/proc/uptime`, `/proc/meminfo`, and `/proc/stat`, CMake 3.16 or newer, a C++17 compiler, and a build tool such as Make. From the directory containing `CMakeLists.txt`, run:

```sh
cmake -S . -B build
cmake --build build
./build/sysmon
```

The first command prepares the build directory. After editing a source file, repeat the last two commands. Output looks like this:

```text
System uptime: 0h:23m:53s
Memory: 26286 MiB available / 32013 MiB total
CPU usage: 1.50%
```

The memory values come from the `MemAvailable` and `MemTotal` fields. They are displayed in MiB, and available memory may change between runs.

CPU usage appears after a pause of about one second between two reads. It reports the busy fraction across all CPUs over that interval, treating idle plus iowait as nonbusy. CPU percentages display two decimal places. Values vary with system activity.

## Help

Run `./build/sysmon --help` to see usage and supported options. Help prints immediately and exits successfully without collecting measurements.

## Watch mode

Run repeated measurements with:

```sh
./build/sysmon --watch
```

Each snapshot is appended to the terminal. The existing CPU sampling pause paces updates at roughly one snapshot per second plus processing time. Press Ctrl+C to stop. Running without arguments prints one snapshot. Unknown or extra arguments print `Usage: sysmon [--watch | --help]` and exit with status 1.

## Run the tests

After configuring and building, run:

```sh
(cd build && ctest --output-on-failure)
```

CTest runs the registered memory_parser and cpu_parser tests, covering memory parsing, CPU parsing, and CPU percentage calculations. A nonzero test exit status reports failure. The parentheses keep the directory change local to that command.

## GitHub build and test workflow

[.github/workflows/ci.yml](.github/workflows/ci.yml) configures a clean Ubuntu build and CTest run on pushes and pull requests, with a manual trigger available too. View results in the repository's Actions tab. The push-triggered and manual runs for commit f2a5630 both passed Checkout, Configure, Build, and Test. The [pull-request run](https://github.com/RDagunan/sysmon-cli/actions/runs/45574628586) and [merged-main run](https://github.com/RDagunan/sysmon-cli/actions/runs/45574902968) also passed.

The latest reviewed commit, `96aa51c`, also passed its [GitHub build and test run](https://github.com/RDagunan/sysmon-cli/actions/runs/45755811825). Final local CLI verification is recorded in CODE_GUIDE.md.

## Where things live

For annotated code snippets and explanations of each step, read [CODE_GUIDE.md](CODE_GUIDE.md).

For the program's structure, current milestone, and proposed next steps, read [architecture.md](architecture.md).

| Path | Purpose |
| --- | --- |
| [src/main.cpp](src/main.cpp) | Start at `main()` for argument handling and repetition. `print_snapshot()` reads and displays uptime, memory, and CPU usage. |
| [CMakeLists.txt](CMakeLists.txt) | Sets the C++ standard and lists the sources for the `sysmon` executable. |
| `build/` | Generated build files and executable. Git ignores this directory. |
| [include/sysmon/metrics.h](include/sysmon/metrics.h) | Declarations of the shared memory and CPU parsers. |
| [src/metrics.cpp](src/metrics.cpp) | Shared memory parser, CPU parser, and CPU percentage calculation used by the application and tests. |
| [tests/metrics_test.cpp](tests/metrics_test.cpp) | Automated memory-parser checks registered with CTest. |
| [tests/cpu_test.cpp](tests/cpu_test.cpp) | Automated CPU-parser and percentage-calculation checks registered with CTest. |

See [conventions.md](conventions.md) for development practices, [decisions.md](decisions.md) for design choices, and [SESSION_LOG.md](SESSION_LOG.md) for session history. AI contributors should start with [AGENTS.md](AGENTS.md).
