# SysMon CLI

SysMon CLI is a Linux system monitor written in C++17. Run `sysmon` to see system uptime and available and total RAM. It reads uptime from `/proc/uptime` and memory values from `/proc/meminfo`, prints the measurements, and exits.

## Build and run

You need Linux with a readable `/proc/uptime`, CMake 3.16 or newer, a C++17 compiler, and a build tool such as Make. From the directory containing `CMakeLists.txt`, run:

```sh
cmake -S . -B build
cmake --build build
./build/sysmon
```

The first command prepares the build directory. After editing a source file, repeat the last two commands. Output looks like this:

```text
System uptime: 0h:23m:53s
Memory: 26286 MiB available / 32013 MiB total
```

The memory values come from the `MemAvailable` and `MemTotal` fields. They are displayed in MiB, and available memory may change between runs.

## Where things live

| Path | Purpose |
| --- | --- |
| [src/main.cpp](src/main.cpp) | Start at `main()` to follow the uptime and memory reads, conversions, and terminal output. |
| [CMakeLists.txt](CMakeLists.txt) | Sets the C++ standard and lists the sources for the `sysmon` executable. |
| `build/` | Generated build files and executable. Git ignores this directory. |
| `include/sysmon/`, `tests/` | Reserved for project headers and tests. Both are currently empty. |

See [conventions.md](conventions.md) for development practices, [decisions.md](decisions.md) for design choices, and [SESSION_LOG.md](SESSION_LOG.md) for session history. AI contributors should start with [AGENTS.md](AGENTS.md).
