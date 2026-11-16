# Working on SysMon CLI

This is a Linux CLI built with C++17, the standard library, and CMake. The executable target is `sysmon`. Start with [README.md](README.md), then inspect [CMakeLists.txt](CMakeLists.txt) and [src/main.cpp](src/main.cpp) before describing or changing behavior.

- **When in doubt, the code wins.** Source and build files define current behavior. The user's request defines the intended change. Use [decisions.md](decisions.md) for rationale and [SESSION_LOG.md](SESSION_LOG.md) for history. Check stale claims against the current checkout.
- Follow [conventions.md](conventions.md) for naming, file input, output, and builds. Preserve the CLI output and exit codes unless the task calls for a change.
- Keep changes scoped to the request. Avoid adding dependencies, redesigning the program, or editing generated files under `build/` to fix source issues.
- When the user asks for a code review or learning help, explain the issue using their code. Leave implementation to the user unless they ask for edits.
- Verify behavior changes by rebuilding and running the executable. Record which paths you exercised. Only mark an error case tested if you triggered it and observed the result.
- Maintain the project documentation as part of development. Update README.md when usage or behavior changes, conventions.md when practices are adopted, decisions.md when choices are settled, and SESSION_LOG.md with actual work and verification. The assistant owns these updates unless the user asks to write them personally.
