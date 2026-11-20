# Learning SysMon through its code

Updated 2026-11-19. This guide follows [src/main.cpp](src/main.cpp) and [CMakeLists.txt](CMakeLists.txt). Read it alongside those files. The comments inside the snippets explain the code and its purpose. Some snippets are short extracts, so use the source file to see the complete program.

## Find your place

1. [Headers and syntax](#headers-and-syntax)
2. [How execution starts](#how-execution-starts)
3. [The memory function](#the-memory-function)
4. [Testing with sample text](#testing-with-sample-text)
5. [Reading uptime](#reading-uptime)
6. [Converting and printing uptime](#converting-and-printing-uptime)
7. [Reading real memory values](#reading-real-memory-values)
8. [Building and running](#building-and-running)
9. [What we have verified](#what-we-have-verified)
10. [Linux measurement reference](#linux-measurement-reference)
11. [CPU exercise — comparing two readings](#cpu-exercise--comparing-two-readings)
12. [Sources and further reading](#sources-and-further-reading)

## Headers and syntax

```cpp
#include <fstream>   // Provides ifstream for reading files.
#include <iostream>  // Provides cout, cerr, and stream input/output facilities.
#include <sstream>   // Provides istringstream for reading values from text.
#include <string>    // Provides string and getline for reading a whole line.

using namespace std; // Lets this file use names such as cout without std::.
```

A header supplies declarations and definitions needed to use library features. Our earlier `istringstream` build error came from omitting `<sstream>`. Adding the header made its full definition available to the compiler.

The existing file uses `using namespace std;`. For example, `ifstream` refers to `std::ifstream`, a type in the standard library.

| Syntax in our code | Meaning |
| --- | --- |
| `;` | Ends a declaration or statement such as `total_kb = -1;`. |
| `{ ... }` | Groups statements into a function, loop, or conditional block. |
| `( ... )` | Holds arguments to a function, a condition, or a grouped expression. |
| `"text"` | A string literal, used for file paths, field names, and messages. |
| `=` | Initializes a variable in a declaration, or assigns a new value afterward. |
| `==` | Compares two values for equality. |
| `!` | Reverses a true/false condition. |
| `&&` | Requires both conditions to be true in a C++ expression. |
| `//` | Starts a comment that continues to the end of the line. |

Blank lines separate related steps for readability. An indented statement remains inside its block because of the surrounding braces.

## How execution starts

```cpp
int main() {  // The program begins executing here.
    // Current order: uptime, real memory, then CPU sampling.
    return 0; // End successfully if execution reaches this point.
}
```

The `read_memory` definition appears above `main()`, so the compiler knows about it when it sees a call. Execution enters that function when `main()` calls it.

Our program uses `return 1;` in `main()` to stop after an error. A shell can inspect that exit status. A return statement immediately ends the function in which it appears.

## The memory function

### Parameters and references

```cpp
bool read_memory(istream& input,
                 long long& total_kb,
                 long long& available_kb) {
    // Function body follows below.
}
```

| Part | What it does and why we use it |
| --- | --- |
| `bool` | The return value is `true` when both memory values were found, or `false` otherwise. |
| `read_memory` | Our function's name. We call it to reuse the parsing steps. |
| `istream& input` | Receives a reference to an existing input stream. It accepts a file stream or a text stream, which lets us test the same parsing code with sample text. |
| `long long& total_kb` | Refers to the caller's variable for total memory. Assignments inside the function update that variable. |
| `long long& available_kb` | Refers to the caller's variable for available memory. |

`long long` is an integer type with at least 64 bits. We use it for memory values expressed in the file's `kB` units. A reference, written with `&` here, gives the function access to the original object supplied by its caller.

For this call:

```cpp
read_memory(memory_file, mem_total_kb, mem_available_kb);
```

`input` refers to `memory_file`, `total_kb` refers to `mem_total_kb`, and `available_kb` refers to `mem_available_kb`. The return value reports success while the two reference parameters receive the measurements. Reading advances the supplied stream.

### Starting values

```cpp
total_kb = -1;     // Mark total memory as missing before scanning.
available_kb = -1; // Mark available memory as missing before scanning.
string line;      // Hold the next line read from the stream.
```

We use `-1` to mean a field has not been found. Zero is a possible available-memory value, so it would be ambiguous as the missing marker. Resetting both outputs inside the function also gives every call a fresh starting state.

### Read one line at a time

```cpp
while (getline(input, line)) {  // Read a line, then enter the loop if it succeeded.
    istringstream fields(line); // Treat this line's text as an input stream.
    string field_name;         // Hold a label such as "MemTotal:".
    long long value_kb;        // Hold the number following that label.

    if (!(fields >> field_name >> value_kb)) { // Try reading both values.
        continue;                            // Skip this line if extraction failed.
    }

    // Field comparisons follow here.
}
```

`getline` puts a complete line into `line` and consumes its newline. At the end of the input, the loop stops when reading fails. Each iteration creates a fresh `fields` stream to parse the new line.

The `>>` operator extracts values separated by whitespace. For `MemTotal: 8192 kB`, it reads `MemTotal:` into `field_name` and `8192` into `value_kb`. The colon stays attached to the label. The trailing `kB` text is left unread in this line's stream.

In `if (!(fields >> field_name >> value_kb))`, the extraction result can be checked as a condition. The surrounding `!` makes the body run when extraction fails. `continue` moves to the next loop iteration, so an unread numeric value is never used.

### Select the fields

```cpp
if (field_name == "MemTotal:") {           // Check for the total-memory label.
    total_kb = value_kb;                  // Save its numeric value.
} else if (field_name == "MemAvailable:") { // Otherwise check for available memory.
    available_kb = value_kb;              // Save that numeric value.
}
```

The file contains many memory measurements. These comparisons choose the two used by SysMon. Looking up names lets the function accept either order. Other field names leave the two outputs as they were.

### Report whether both fields were found

```cpp
return total_kb >= 0 && available_kb >= 0;
```

`>= 0` accepts a nonnegative measurement. The `&&` expression is true when both comparisons are true. If a field remains `-1`, the function returns `false`.

This function checks whether it obtained two nonnegative values. It skips lines whose number cannot be extracted and assumes the requested fields use the kernel's `kB` units. Those details explain the scope of the current checks.

## Testing with sample text

We temporarily added samples to the start of `main()` while learning to test the parser. These samples have now been removed from the live program. This section preserves the exercise for reference. The missing-field sample was:

```cpp
istringstream sample(           // Build a stream from known text.
    "MemTotal: 8192 kB\n"       // Supply total memory and a newline.
);                             // Finish constructing the stream.

long long test_total_kb;        // The function will initialize and fill this.
long long test_available_kb;    // The function will initialize and fill this too.

bool success = read_memory(sample, test_total_kb, test_available_kb);

cout << "Sample success: " << success << endl;
cout << "Sample total: " << test_total_kb << " kB" << endl;
cout << "Sample available: " << test_available_kb << " kB" << endl;
```

`\n` creates a newline within the string. The function receives this text stream through its `istream&` parameter, just as it receives the real file later.

The call sets `test_total_kb` to `8192`, leaves `test_available_kb` at `-1`, and returns `false` because available memory is missing. With the current output settings, `cout` prints a boolean as `1` for true or `0` for false.

`<<` sends each value to the output stream. `endl` writes a newline and flushes the stream, making buffered output available immediately. The labels and units make the values easier to interpret.

During that exercise the sample result was printed and execution continued into the uptime and real-memory sections. A failed sample did not make `main()` exit with status 1. It was a manual learning check where you compared the printed result with the expected result.

To supply both fields, the earlier sample used adjacent string literals:

```cpp
istringstream sample(
    "MemTotal: 8192 kB\n"
    "MemAvailable: 4096 kB\n"
);
```

C++ joins those two literals into one text value. The expected results are success `1`, total `8192`, and available `4096`. Reversing the two lines should give those same results. Construct a fresh sample stream for each test because reading consumes the input.

## Reading uptime

```cpp
ifstream uptime_file("/proc/uptime"); // Open the Linux uptime file for reading.

if (!uptime_file) {                  // Check whether opening failed.
    cerr << "Error: cannot open /proc/uptime" << endl;
    return 1;                       // Stop main() with a failure status.
}

double uptime_seconds = 0.0;         // Store seconds with a fractional part.

if (!(uptime_file >> uptime_seconds)) { // Try extracting the first number.
    cerr << "Error: cannot read system uptime" << endl;
    return 1;                          // Stop if extraction failed.
}
```

Linux's `/proc` files expose kernel information for programs to read. A line such as `707.35 10681.00` begins with seconds since boot. SysMon extracts the first value. The second is accumulated idle time across CPUs.

`double` holds a number such as `707.35`. We check opening and reading separately so the message identifies which operation failed. `cerr` is the standard error stream.

The file stream closes automatically when it leaves scope as `main()` exits.

## Converting and printing uptime

```cpp
int total_seconds = static_cast<int>(uptime_seconds); // Drop the fractional part.
int uptime_hours = total_seconds / 3600;              // Whole hours.
int uptime_mins = (total_seconds % 3600) / 60;         // Remaining whole minutes.
int uptime_secs = total_seconds % 60;                 // Remaining seconds.

cout << "System uptime: " << uptime_hours << "h:"
     << uptime_mins << "m:"
     << uptime_secs << "s" << endl;
```

`static_cast<int>` explicitly converts the value to an integer. For positive uptime, `1433.09` becomes `1433`. Integer division drops the remainder. The `%` operator gives the remainder.

| Calculation for 3661 seconds | Result |
| --- | --- |
| `3661 / 3600` | 1 hour |
| `(3661 % 3600) / 60` | 61 remaining seconds divided by 60 gives 1 minute |
| `3661 % 60` | 1 remaining second |

The chained output prints `System uptime: 1h:1m:1s`. The line breaks in the C++ statement keep the source readable. Only the requested `endl` ends the output line.

These calculations run after reading uptime. A calculated variable stores its result at assignment time. It does not recalculate itself when another variable later changes.

The current conversion assumes uptime fits in an `int`. A wider integer and a checked conversion would be appropriate if we extend support to values beyond that range.

## Reading real memory values

```cpp
ifstream memory_file("/proc/meminfo"); // Open the kernel's memory information.

if (!memory_file) {                   // Check that it opened.
    cerr << "Error: cannot open /proc/meminfo" << endl;
    return 1;
}

long long mem_total_kb = -1;           // Declare storage for total memory.
long long mem_available_kb = -1;       // Declare storage for available memory.

if (!read_memory(memory_file, mem_total_kb, mem_available_kb)) {
    cerr << "Error: cannot read memory information" << endl;
    return 1;
}

cout << "Memory: " << mem_available_kb / 1024
     << " MiB available / " << mem_total_kb / 1024
     << " MiB total" << endl;
```

The call reuses the parser with an actual file. It resets and fills the output variables. When parsing returns `false`, the surrounding `!` makes `main()` enter the error branch and exit.

`MemTotal` means total usable RAM. `MemAvailable` estimates RAM available for applications without swapping. Linux labels these fields `kB`, using units of 1024 bytes. Dividing by `1024` converts them to MiB. Integer division displays whole MiB, dropping any fraction.

For the two-field sample, `4096 / 1024` gives 4 MiB available and `8192 / 1024` gives 8 MiB total. The real file is read separately from the sample, so actual memory values can appear after a failed sample check.

## Building and running

The current CMake file has four statements:

```cmake
cmake_minimum_required(VERSION 3.16)  # Require at least this CMake version.
project(SysMonCLI LANGUAGES CXX)     # Name the project and enable C++.
set(CMAKE_CXX_STANDARD 17)           # Request the C++17 language standard.
add_executable(sysmon src/main.cpp) # Build sysmon from this source file.
```

CMake generates build instructions. The build tool runs the compiler to translate the C++ source and link the executable.

Run these commands from the directory containing `CMakeLists.txt`:

```sh
cmake -S . -B build
cmake --build build && ./build/sysmon
```

| Command part | Purpose |
| --- | --- |
| `-S .` | Use the current directory as the source directory. |
| `-B build` | Put generated build files in `build/`. |
| `--build build` | Compile and link through the configured build system. |
| Shell `&&` | Run the next command only if the first command succeeds. |
| `./build/sysmon` | Run the executable at this path relative to the current directory. |

After changing C++ source, rebuild before running so your executable contains the edit. Successful compilation checks the source can be built. Runtime samples check the resulting program's behavior.

## What we have verified

The statuses below describe evidence from this learning session.

| Check | Expected result | Evidence |
| --- | --- | --- |
| Both memory fields in normal order | Success 1, total 8192, available 4096 | Shown in the user's build and run screenshot. |
| Both fields in reverse order | Same values as normal order | Shown in the user's build and run screenshot. Source inspection confirms the reversed input order. |
| Missing available-memory field | Success 0, total 8192, available -1 | Shown in the user's build and run screenshot. This exercises the parser's failure result. |
| Real file reads after the function extraction | Uptime and memory output | Shown in the user's screenshots. |
| Uptime at exactly one hour | 3600 seconds displays as `1h:0m:0s` | User reported this result after temporarily assigning 3600.0 before the conversion. Source inspection confirms the assignment was subsequently removed. |
| Live output after sample cleanup | Only uptime and memory output | Shown in the user's successful build and run screenshot. Source inspection confirms the sample block was removed. |
| File-open error or caller's memory-error branch | Error message and exit status 1 | Reviewed in source. Runtime result has not been demonstrated. |

## Linux measurement reference

### Uptime and memory fields

| File | Field | Unit in the file | Meaning | How SysMon uses it |
| --- | --- | --- | --- | --- |
| `/proc/uptime` | First number | Seconds, including a fraction | Time since boot, including time spent suspended | Convert to whole hours, remaining minutes, and remaining seconds. |
| `/proc/uptime` | Second number | Seconds | Accumulated idle time across CPUs | Available for comparison when learning about CPU time. |
| `/proc/meminfo` | `MemTotal:` | `kB` | Total usable RAM | Divide by 1024 for whole MiB. |
| `/proc/meminfo` | `MemAvailable:` | `kB` | Estimated RAM available for starting applications without swapping | Divide by 1024 for whole MiB. |

The uptime fields are documented in the [Linux uptime manual](https://man7.org/linux/man-pages/man5/proc_uptime.5.html). Memory field definitions appear in the [Linux memory-information manual](https://man7.org/linux/man-pages/man5/proc_meminfo.5.html). For the kernel's memory units and accounting details, see the [kernel /proc documentation](https://docs.kernel.org/filesystems/proc.html).

For the user's earlier memory sample, integer division gives the following conversions. The figures describe that sample rather than permanent machine measurements.

| Measurement | Sample in kB | Conversion | Whole MiB |
| --- | --- | --- | --- |
| Total | 32782048 | `32782048 / 1024` | 32013 |
| Available | 26916544 | `26916544 / 1024` | 26285 |

### CPU fields in order

The aggregate `cpu` line combines CPU time across the machine. Each number is a cumulative count in clock ticks (`USER_HZ` units). The following table maps every number from the user's first CPU sample. The field definitions and units come from the [Linux CPU-statistics manual](https://man7.org/linux/man-pages/man5/proc_stat.5.html).

```text
cpu 135446 757 27055 4111242 7445 0 500 0 0 0
```

| Numeric position after cpu | Field | Sample value | Meaning |
| --- | --- | --- | --- |
| 1 | `user` | 135446 | Running application code in user mode. |
| 2 | `nice` | 757 | Running user-mode code with lowered scheduling priority. |
| 3 | `system` | 27055 | Running kernel code. |
| 4 | `idle` | 4111242 | Time spent idle. |
| 5 | `iowait` | 7445 | Time accounted as waiting for input/output. |
| 6 | `irq` | 0 | Handling hardware interrupts. |
| 7 | `softirq` | 500 | Handling software interrupts. |
| 8 | `steal` | 0 | Time taken by other operating systems while this system runs in a virtualized environment. |
| 9 | `guest` | 0 | Running guest virtual CPUs. |
| 10 | `guest_nice` | 0 | Running guest virtual CPUs with lowered scheduling priority. |

For our proposed total, sum fields 1 through 8. Guest time is already accounted within user or nice time, so adding fields 9 and 10 again would count that time twice. This follows the kernel's [guest-time accounting implementation](https://github.com/torvalds/linux/blob/master/kernel/sched/cputime.c).

### Inspection commands

These commands read kernel measurements directly. Their observed results are recorded in [SESSION_LOG.md](SESSION_LOG.md).

| Command | What it does | Why we use it |
| --- | --- | --- |
| `cat /proc/uptime` | Print the uptime file. | Compare its first value with SysMon's formatted time. |
| `grep -E '^(MemTotal\|MemAvailable):' /proc/meminfo` | Print the two named memory fields. | Compare the file values with the displayed MiB values. |
| `head -n 1 /proc/stat` | Print the first line of CPU statistics. | Inspect the aggregate CPU counters. |
| `sleep 1` | Pause for one second. | Allow counters to change before another reading. |

In the grep expression, `^` matches the start of a line, `(...)` groups the two names, and `|` means either name. `-E` enables that regular-expression syntax. The colon matches the colon following the name. Single quotes pass the expression to grep as one argument. In `head -n 1`, `-n 1` requests one line.

## CPU exercise — comparing two readings

CPU monitoring is now implemented and has passed live builds and runs. The following is a worked calculation from the user's terminal screenshot, before implementation in C++.

```text
cpu 153665 758 30501 5199249 7559 0 540 0 0 0
cpu 153929 758 30542 5211582 7560 0 540 0 0 0
```

For each reading, sum the first eight numeric fields to obtain total ticks. For this monitor's proposed calculation, combine idle and iowait (the fourth and fifth numbers) as nonbusy ticks. The field definitions are documented in the [Linux proc_stat manual](https://man7.org/linux/man-pages/man5/proc_stat.5.html).

The proposed calculation uses these steps:

```text
total = user + nice + system + idle + iowait + irq + softirq + steal
nonbusy = idle + iowait

elapsed = second_total - first_total
nonbusy_change = second_nonbusy - first_nonbusy
busy_change = elapsed - nonbusy_change
usage_percent = 100.0 * busy_change / elapsed
```

For an illustrative change of 100 total ticks and 75 nonbusy ticks, the result is `100.0 * (100 - 75) / 100 = 25%`. This is a teaching example. The screenshot provides the actual measurements below.

| Value | First reading | Second reading | Change |
| --- | --- | --- | --- |
| Total ticks | 5392272 | 5404911 | 12639 |
| Idle plus iowait ticks | 5206808 | 5219142 | 12334 |

```text
busy change = 12639 - 12334 = 305
usage percent = 100.0 * 305 / 12639 = approximately 2.41%
```

The percentage covers the interval between the readings. When commands are entered separately, that interval also includes time spent entering commands. The ratio uses counter differences and therefore does not require converting ticks into seconds. Future C++ code must check for zero or inconsistent differences before dividing. The manual documents that iowait may sometimes decrease.

The collection commands used in the exercise were:

```sh
head -n 1 /proc/stat
sleep 1
head -n 1 /proc/stat
```

Live CPU output and the known-input 25% C++ calculation are now verified. The zero-change guard has passed with exit status 1. Other error-path checks remain pending. The 2.41% result is a verified calculation from the supplied counters.

## Sources and further reading

These are the technical references used for the Linux measurements discussed so far.

| Reference | Use in this project |
| --- | --- |
| [Linux uptime manual](https://man7.org/linux/man-pages/man5/proc_uptime.5.html) | Meaning and units of both uptime fields. |
| [Linux memory-information manual](https://man7.org/linux/man-pages/man5/proc_meminfo.5.html) | Memory field format, total RAM, and available RAM. |
| [Linux CPU-statistics manual](https://man7.org/linux/man-pages/man5/proc_stat.5.html) | CPU field order, units, and iowait limitations. |
| [Kernel /proc documentation](https://docs.kernel.org/filesystems/proc.html) | Kernel-provided file formats and memory-accounting context. |
| [Kernel /proc documentation, version 5.19](https://www.kernel.org/doc/html/v5.19/filesystems/proc.html) | Earlier version consulted during the initial CPU-field explanation. |
| [Kernel CPU-accounting source](https://github.com/torvalds/linux/blob/master/kernel/sched/cputime.c) | How guest time contributes to user and nice counters. |

## CPU coding lesson 1 — read one snapshot

Implemented by the learner and reviewed in source. A user-provided screenshot shows a successful build and live run with total ticks 9377605 and nonbusy ticks 9036187. The code now reads `/proc/stat`. This verifies one successful snapshot. The earlier field reference explains that file's first line.

Add this function after `read_memory` and before `main`:

```cpp
bool read_cpu(istream& input,
              long long& total_ticks,
              long long& nonbusy_ticks) {
    total_ticks = -1;
    nonbusy_ticks = -1;

    string line;
    if (!getline(input, line)) {
        return false;
    }

    istringstream fields(line);
    string label;
    long long user, nice, system, idle;
    long long iowait, irq, softirq, steal;

    if (!(fields >> label >> user >> nice >> system >> idle
                 >> iowait >> irq >> softirq >> steal)) {
        return false;
    }

    if (label != "cpu") {
        return false;
    }

    total_ticks = user + nice + system + idle
                + iowait + irq + softirq + steal;
    nonbusy_ticks = idle + iowait;
    return true;
}
```

- The signature follows `read_memory`. It returns a success flag and fills two caller-owned variables through references.
- `total_ticks = -1;` and `nonbusy_ticks = -1;` establish failure markers before reading.
- `string line;` declares storage for the first line. The `getline` check returns false immediately if reading it fails.
- `istringstream fields(line);` lets us extract individual values from that line.
- `string label;` stores the word `cpu`. The two `long long` declarations name the first eight numeric counters in file order. Each comma declares another variable of the same type.
- The chained `>>` expression reads the label and all eight values. A failed extraction returns false before any arithmetic uses those variables.
- `label != "cpu"` means the label differs from the aggregate label we expect. A line beginning with `cpu0`, for example, is rejected by this function.
- The addition computes the total across the first eight counters. The next assignment computes idle plus iowait under the proposed usage calculation.
- `return true;` tells the caller that the two outputs have been filled. The function reads one snapshot per call.

Replace the unfinished CPU section at the end of `main()`, before its final `return 0;`, with:

```cpp
ifstream cpu_file("/proc/stat");
if (!cpu_file) {
    cerr << "Error: cannot open /proc/stat" << endl;
    return 1;
}

long long total_ticks;
long long nonbusy_ticks;

if (!read_cpu(cpu_file, total_ticks, nonbusy_ticks)) {
    cerr << "Error: cannot read CPU counters" << endl;
    return 1;
}

cout << "CPU total ticks: " << total_ticks << endl;
cout << "CPU nonbusy ticks: " << nonbusy_ticks << endl;
```

The file construction opens the counter source. Its error check reports an open failure and exits. The declarations reserve the two outputs. The function call initializes and fills them, and its check reports a parse failure. The two output statements temporarily display the snapshot so the learner can inspect it. Values will change between runs.

The user ran `cmake --build build && ./build/sysmon` successfully and observed the two labeled counter totals after uptime and memory. The next lesson collects a second snapshot and checks the differences before calculating a percentage.

This first parser assumes the kernel supplies nonnegative counters whose sum fits in `long long`. Sampling and difference validation are upcoming work.

## CPU coding lesson 2 — compare snapshots

Implemented by the learner and reviewed in source on 2026-11-20. The user demonstrated a successful run with CPU usage 1.50094%. A fresh assistant build and run succeeded with 0.502197%. The known-input 25% case has now passed. The zero-change guard has passed with exit status 1. Other error-path checks remain pending. The following steps explain how this code was added.

Add the following headers at the top:

```cpp
#include <chrono> // Supplies chrono::seconds for expressing a duration.
#include <thread> // Supplies this_thread::sleep_for for pausing execution.
```

Keep the existing first CPU file read, its checks, and its variables `total_ticks` and `nonbusy_ticks`. Replace the two temporary CPU counter print statements with:

```cpp
this_thread::sleep_for(chrono::seconds(1));

ifstream second_cpu_file("/proc/stat");
if (!second_cpu_file) {
    cerr << "Error: cannot reopen /proc/stat" << endl;
    return 1;
}

long long second_total_ticks;
long long second_nonbusy_ticks;

if (!read_cpu(second_cpu_file, second_total_ticks, second_nonbusy_ticks)) {
    cerr << "Error: cannot read second CPU snapshot" << endl;
    return 1;
}

long long elapsed_ticks = second_total_ticks - total_ticks;
long long nonbusy_change = second_nonbusy_ticks - nonbusy_ticks;

if (elapsed_ticks <= 0 || nonbusy_change < 0 || nonbusy_change > elapsed_ticks) {
    cerr << "Error: inconsistent CPU readings" << endl;
    return 1;
}

long long busy_ticks = elapsed_ticks - nonbusy_change;
double cpu_percent = 100.0 * busy_ticks / elapsed_ticks;

cout << "CPU usage: " << cpu_percent << "%" << endl;
```

Keep the final `return 0;` after this code.

| Line or block | What it does and why |
| --- | --- |
| `this_thread::sleep_for(chrono::seconds(1));` | Pause this program's execution for at least roughly one second before taking the second reading. The actual interval may be longer due to scheduling and file-reading work. |
| `ifstream second_cpu_file("/proc/stat");` | Open a fresh stream positioned at the start of the file. The first stream has already advanced past its first line. |
| `if (!second_cpu_file)` and its body | Report failure to open the second stream and stop main() with exit status 1. |
| The two `long long` declarations | Reserve storage for the second snapshot's counters. |
| `read_cpu(second_cpu_file, ...)` and its check | Reuse the existing parser to fill both outputs. Stop if parsing fails. |
| `elapsed_ticks = second_total_ticks - total_ticks` | Calculate the total counter change across the interval. |
| `nonbusy_change = second_nonbusy_ticks - nonbusy_ticks` | Calculate the idle-plus-iowait counter change across the same interval. |
| The consistency check | Require a positive total change and a nonbusy change between zero and that total. The logical OR operator means any of the conditions can make the error branch run. This catches zero elapsed ticks before division and rejects inconsistent counter changes. |
| `busy_ticks = elapsed_ticks - nonbusy_change` | Calculate the portion counted as busy under our measurement definition. |
| `double cpu_percent = 100.0 * busy_ticks / elapsed_ticks` | Express that portion as a percentage. The `100.0` makes multiplication and division use floating-point arithmetic so fractions are retained. |
| The `cout` statement | Display CPU usage with a percent sign and a newline. |

Build and run with `cmake --build build && ./build/sysmon`. Expect uptime and memory immediately, followed by CPU usage after the sampling pause. Values will vary with activity. Live sampling is verified. The known 25% sample calculation has passed. The zero-change consistency check has passed with exit status 1. Other consistency conditions remain untested.

## Next manual CPU checks

These inputs check the existing C++ difference calculation. The 25% case has user-provided execution evidence. The zero-change error case also passed with exit status 1. The decreasing-nonbusy and excessive-nonbusy cases remain pending.

| First total | First nonbusy | Second total | Second nonbusy | Expected result |
| --- | --- | --- | --- | --- |
| 1000 | 800 | 1100 | 875 | Passed in the user's build-and-run screenshot: CPU usage 25%. |
| 1000 | 800 | 1000 | 800 | Passed in the user's screenshot: error message and exit status 1. |
| 1000 | 800 | 1100 | 799 | Error because nonbusy time decreased. |
| 1000 | 800 | 1100 | 901 | Error because nonbusy change exceeds total change. |

### Running the manual CPU calculation checks

These temporary assignments are proposed learning edits. Add them after the successful second read_cpu call and before the elapsed_ticks calculation. They replace the collected values with known inputs so the existing calculation and consistency check can be observed.

```cpp
// Temporary test values: expect 25% usage.
total_ticks = 1000;
nonbusy_ticks = 800;
second_total_ticks = 1100;
second_nonbusy_ticks = 875;
```

The first two assignments represent the starting counters. The next two represent the ending counters. The elapsed change is 100, the nonbusy change is 75, and the busy change is 25, giving 25%. Build and run with `cmake --build build && ./build/sysmon`. Execution should still print uptime and memory, followed by `CPU usage: 25%`. The user has demonstrated this result in a successful build-and-run screenshot.

After that result is confirmed, change only the two ending counters to test zero elapsed time:

```cpp
second_total_ticks = 1000;
second_nonbusy_ticks = 800;
```

Now both differences are zero. Expect `Error: inconsistent CPU readings` and an exit status of 1. Run `echo $?` immediately after the build-and-run command to inspect that status. In the shell, `$?` contains the exit status of the last command. A successful build is needed so the status describes the program's failure rather than a build error. The user's successful build-and-run screenshot demonstrates the error message, followed by `echo $?` printing 1. This verifies the zero-change branch. The other two consistency conditions remain untested.

After both checks are confirmed, remove all four temporary assignments and their test comment, rebuild, and run again to restore live sampling. The known-input checks test arithmetic and its guard. They do not test CPU field parsing with malformed input.

## Maintaining the guide

When we add or change code, the assistant will update the matching snippets and explanations, introduce new syntax using the actual project code, and record demonstrated results. Use [architecture.md](architecture.md) for the roadmap and [SESSION_LOG.md](SESSION_LOG.md) for the chronological record.

## Live sampling restored after checks

The user reported uptime 2h:17m:42s, available memory 25631 MiB of 32013 MiB, and CPU usage 1.19197% after removing the temporary assignments. Source inspection confirms the four assignments are gone. The 25% and zero-change checks are recorded above. Decreasing or excessive nonbusy changes and file/parse failures still lack runtime evidence.
