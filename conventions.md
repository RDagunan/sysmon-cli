# Development conventions

## C++ and CLI behavior

- Use four spaces for indentation, opening braces on the same line, and descriptive snake_case names. Include units where they clarify a value, as in `uptime_seconds`.
- Check that an input file opened, then check that extracting the value succeeded. Calculate derived values after the successful read.
- Send the measurement to `std::cout` and errors to `std::cerr`. Return `0` on success and `1` for a file-open or parse failure.
- Keep the uptime display in the form `0h:23m:53s`. Drop fractional seconds and let hours accumulate past 24.

## Build and verify

- Keep implementation in `src/`. When adding a compiled source file, list it in the `sysmon` target in `CMakeLists.txt`.
- Configure with `cmake -S . -B build` from the project root. Keep generated files in the ignored `build/` directory.
- Rebuild with `cmake --build build`, then run `./build/sysmon`. For a manual uptime check, run `cat /proc/uptime` immediately afterward and compare its first value with the displayed time, allowing for elapsed time between commands.

## Document the work

Update the README when run instructions or visible behavior change. Add a dated line to the decision log when a design choice is settled. In the session journal, explain commands you ran and record their actual results. Label suggested commands as pending until someone runs them.
