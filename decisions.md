# Decisions

- [2026-11-12] Use CMake to build the C++17 executable `sysmon` so the compiler settings and source list live in one place.
- [2026-11-12] Generate build output in the root `build/` directory and ignore it in Git because CMake can recreate it.
- [2026-11-12] Read the first field of Linux `/proc/uptime` for system uptime because it gives elapsed seconds since boot.
- [2026-11-16] Document the CLI's whole-hour, minute, and second display so contributors can verify its output against the kernel value.
- [2026-11-16] Read `MemTotal` and `MemAvailable` by name from `/proc/meminfo` and display whole MiB because the file contains many fields reported in kB.
- [2026-11-20] Calculate aggregate CPU usage from two /proc/stat snapshots about one second apart, treating idle plus iowait as nonbusy and checking differences before division.
