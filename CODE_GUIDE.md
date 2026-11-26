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

## Watch-mode lesson 1 — a reusable snapshot

The suggested next feature is an optional `--watch` argument that repeats readings until Ctrl+C. The learner has implemented the reusable print_snapshot function. The user's screenshot demonstrates a successful build, all three live measurements, and exit status 0.

Rename the existing function header `int main()` to:

```cpp
int print_snapshot() {
    // Keep the existing uptime, memory, and CPU statements here.
    // This comment stands for the current body, including its return statements.
}
```

The snippet above shows the shape of the function. Retain the full existing body when renaming it. Then add this new entry point after the closing brace of print_snapshot:

```cpp
int main() {
    return print_snapshot();
}
```

| Part | Meaning and reason |
| --- | --- |
| `int print_snapshot()` | Name the operation that reads and displays one set of measurements. Its return value remains an exit-style status, 0 for success or 1 for an error. |
| Existing body | Retain the file reads, output, one-second CPU pause, error checks, and final return. |
| New `int main()` | Keep the program's entry point. It now delegates the measurement work to the function. |
| `return print_snapshot();` | Call the function once and return its result to the shell. If reading fails, main returns the same failure status. |

Define print_snapshot above main so the compiler has seen the definition before the call. After the edit there should be exactly one function named main. Rebuild and run with `cmake --build build && ./build/sysmon`, then immediately run `echo $?`. Expected successful output is the same uptime, memory, and CPU lines followed by status 0. No execution result for this refactor is recorded yet.

The next lesson introduces command-line arguments and a loop that repeats print_snapshot only when --watch is selected. Each snapshot already includes the CPU sampling pause.

## Watch-mode lesson 2 — arguments and repetition

The learner has added the argument parser and loop. The user's screenshot shows the default run returning 0 and --watch beginning a second snapshot. The unknown-option check has also passed with a usage message and status 1. The user has also confirmed repeated complete measurements and stopping with Ctrl+C. The lesson code is:

```cpp
int main(int argc, char* argv[]) {
    bool watch_mode = false;

    if (argc == 2 && string(argv[1]) == "--watch") {
        watch_mode = true;
    } else if (argc != 1) {
        cerr << "Usage: sysmon [--watch]" << endl;
        return 1;
    }

    do {
        int status = print_snapshot();
        if (status != 0) {
            return status;
        }
    } while (watch_mode);

    return 0;
}
```

| Line or block | Explanation |
| --- | --- |
| `int main(int argc, char* argv[])` | Receive the argument count and an array of C-style strings from the shell. Each char pointer in the array points to an argument's text. |
| `bool watch_mode = false;` | Start with a single measurement run selected. |
| `argc == 2` | Require the program name plus exactly one argument before examining argv[1]. |
| `string(argv[1]) == "--watch"` | Convert the argument text to a C++ string and compare its contents with the flag. The preceding argc check and short-circuit AND keep this access valid. |
| `watch_mode = true;` | Select repetition when the flag is recognized. |
| `else if (argc != 1)` | Reject unrecognized or excess arguments. With only the program name, argc is 1 and single-run mode is accepted. |
| Usage message and `return 1;` | Tell the user the accepted command shape and end with an error. The square brackets in the message indicate an optional argument. |
| `do { ... } while (watch_mode);` | Run the body at least once, then repeat while watch_mode is true. The final semicolon is part of the loop syntax. |
| `int status = print_snapshot();` | Perform one reading and store its success or failure status. |
| Status check and `return status;` | Stop main immediately if a snapshot failed, preserving the error status. |
| Final `return 0;` | Report success after single-run mode finishes. |

For `./build/sysmon`, argc is 1. For `./build/sysmon --watch`, argc is 2 and argv[1] contains --watch. argv[0] contains the command's program name/path.

Build and run the default mode first using `cmake --build build && ./build/sysmon`, then immediately `echo $?`. Expect one set of measurements and status 0. Next run `./build/sysmon --watch`, wait for at least two sets, and press Ctrl+C to interrupt it. Repetition appends output in this first version. Each snapshot's CPU pause paces the loop. The default run has been demonstrated with exit status 0, and watch mode has begun repeating in the user's screenshot. The unknown-option check also passed. The user has now confirmed repeated measurements and stopping with Ctrl+C.

### Watch-mode verification commands

Let `./build/sysmon --watch` finish at least two full sets of uptime, memory, and CPU output, then press Ctrl+C and confirm the terminal prompt returns. Next run the following commands to exercise the invalid-argument branch:

```sh
./build/sysmon --oops
echo $?
```

Expected output is `Usage: sysmon [--watch]` followed by status 1. The flag --oops is deliberately invalid. This branch checks that the program explains its accepted arguments and exits before collecting measurements. The user's screenshot demonstrates the usage message and exit status 1 for --oops. The user has confirmed repeated uptime, memory, and CPU measurements, followed by interruption with Ctrl+C.

## Proposed next lesson — repeatable memory tests

The working watch-mode milestone is committed as 1e9ed3a. The suggested next step is to save our manual checks in a separate test program, starting with read_memory.

| Proposed path | Responsibility | Reason |
| --- | --- | --- |
| include/sysmon/metrics.h | Declare read_memory, including its parameter and return types. | Let both application and test code know how to call it. |
| src/metrics.cpp | Hold the existing read_memory definition. | Keep one implementation shared by application and tests. |
| tests/metrics_test.cpp | Supply known text and compare the result and output values with expectations. | Make the existing manual checks repeatable. |
| CMakeLists.txt | Build and connect the implementation, application, and test target. | Make the test runnable through the build workflow. |

A declaration states the function's name and types. Its definition contains the statements that run. The first exercise will move the existing function without changing its parser logic, then rebuild the application. The next exercise will add the normal-order sample test. Reversed-order and missing-field tests follow. This work remains proposed, with no source edits or automated test results yet.

## Reviewing the first file split

The learner created src/metrics.cpp and an empty tests/metrics_test.cpp. The memory parser moved into metrics.cpp, along with a second main function containing the live memory display. main.cpp lost its memory display block. No metrics header exists yet. CMake currently declares the sysmon executable twice.

The initial attempt failed with a duplicate-target error. The learner has now completed the corrections below. Source inspection confirms the header, one application main, restored memory output, and one target compiling both source files. The user's screenshot shows successful configuration, compilation, linking, and live output.

### Keep one application entry point

Keep read_memory in metrics.cpp. Move the statements inside its new main function that open /proc/meminfo, check it, call read_memory, and print the memory line back into print_snapshot in main.cpp, between uptime output and CPU sampling. Remove the extra main function from metrics.cpp after moving that block. Keep print_snapshot's existing final return statement.

The responsibilities are:

| File | Contents for this step |
| --- | --- |
| include/sysmon/metrics.h | Declaration of read_memory. |
| src/metrics.cpp | Definition of read_memory and the headers its implementation needs. |
| src/main.cpp | Application entry point, watch loop, print_snapshot, and read_cpu. |
| tests/metrics_test.cpp | Still empty. Its own test entry point will be added in a later lesson as a separate executable. |

### Declare the shared function

Create include/sysmon/metrics.h with:

```cpp
#ifndef SYSMON_METRICS_H
#define SYSMON_METRICS_H

#include <istream>

bool read_memory(std::istream& input,
                 long long& total_kb,
                 long long& available_kb);

#endif
```

The opening preprocessor directives and closing endif form an include guard. They prevent processing this header's contents more than once in one compilation unit. The istream header supplies the stream type. std:: qualifies the standard-library type explicitly. The function declaration states the return type and parameters and ends with a semicolon. The function body lives in metrics.cpp.

Add `#include "sysmon/metrics.h"` to both main.cpp and metrics.cpp. The compiler will use that declaration to check calls and the function definition. CMake's include-directory setting below lets it find the header.

metrics.cpp only needs the following includes for the memory parser:

```cpp
#include "sysmon/metrics.h"
#include <sstream>
#include <string>
```

Its existing using namespace std and read_memory definition can follow those includes. File opening and terminal output remain in print_snapshot.

### Compile both source files into one executable

Replace the two existing add_executable statements with one:

```cmake
add_executable(sysmon
    src/main.cpp
    src/metrics.cpp
)

target_include_directories(sysmon PRIVATE include)
```

The source list compiles both cpp files as part of sysmon. Linking joins their compiled code, so the call in main.cpp can reach the definition in metrics.cpp. The include directory lets this target resolve sysmon/metrics.h under include/. PRIVATE applies this setting to the sysmon target itself.

Keep the existing minimum-version, project, and C++17 statements above these lines. The test file is not part of this executable. A separate test target will be introduced when its test main is ready.

### Verify the corrected split

```sh
cmake -S . -B build
cmake --build build && ./build/sysmon
```

Expect a successful configuration and build followed by all three measurements, including the restored memory line. These commands are proposed for the corrected files. The only fresh result observed in this review was the duplicate-target configuration failure.

## Automated memory test lesson 1

The corrected file split has built and run successfully. The learner has now written the first test, and a screenshot demonstrates memory_parser passing through CTest. Source review found that the application target was removed from CMake while adding the test target. The correction below restores the combined build.

Put this into tests/metrics_test.cpp:

```cpp
#include "sysmon/metrics.h"
#include <iostream>
#include <sstream>

int main() {
    std::istringstream sample(
        "MemTotal: 8192 kB\n"
        "MemAvailable: 4096 kB\n"
    );

    long long total_kb;
    long long available_kb;

    if (!read_memory(sample, total_kb, available_kb)) {
        std::cerr << "FAIL: could not read the sample\n";
        return 1;
    }

    if (total_kb != 8192 || available_kb != 4096) {
        std::cerr << "FAIL: incorrect memory values\n";
        return 1;
    }

    std::cout << "Memory parser test passed\n";
    return 0;
}
```

The project header declares the function. iostream provides diagnostic output, and sstream provides the sample stream. The test main runs separately from the application main. The sample literals contain known values followed by newlines. The two integer declarations hold outputs initialized by read_memory. The first check fails if parsing returns false. The second check fails if either number differs from its expected value. The logical OR operator makes either mismatch sufficient to fail. Both failures return 1, allowing a test runner to detect them automatically. Reaching the final message and return 0 reports success.

Append this to CMakeLists.txt after the application target and its include-directory setting:

```cmake
enable_testing()

add_executable(metrics_test
    tests/metrics_test.cpp
    src/metrics.cpp
)
target_include_directories(metrics_test PRIVATE include)

add_test(NAME memory_parser COMMAND metrics_test)
```

| Statement | Purpose |
| --- | --- |
| enable_testing() | Enable CTest registration in this build directory. |
| add_executable(metrics_test ...) | Build a separate test program using the test main and the existing memory implementation. |
| target_include_directories(metrics_test PRIVATE include) | Let the test target find the shared header. Each target has its own include settings. |
| add_test(NAME memory_parser COMMAND metrics_test) | Register a named check whose success is determined by the test executable's exit status. |

The application and test compile the same metrics.cpp source into their own executables. Each executable has exactly one main. Keep the test source out of the sysmon source list.

From the project root, configure, build, and run the test:

```sh
cmake -S . -B build
cmake --build build
(cd build && ctest --output-on-failure)
```

The last command enters build in a subshell. Parentheses keep the directory change local to that command. CTest runs registered tests and reports failed tests' output when requested with --output-on-failure. Expect one passing test named memory_parser and a summary reporting 100% passed. Actual results remain pending. This checks the normal memory sample; reversed-order and missing-field tests will follow.

## Keep the application and test targets together

The first test passed, but the current CMake file defines only metrics_test. Restore the application target as well. The following complete CMakeLists.txt is the proposed correction:

```cmake
cmake_minimum_required(VERSION 3.16)
project(SysMonCLI LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

enable_testing()

add_executable(sysmon
    src/main.cpp
    src/metrics.cpp
)
target_include_directories(sysmon PRIVATE include)

add_executable(metrics_test
    tests/metrics_test.cpp
    src/metrics.cpp
)
target_include_directories(metrics_test PRIVATE include)

add_test(NAME memory_parser COMMAND metrics_test)
```

The two target names represent two executables. sysmon has its application main, and metrics_test has its testing main. Both compile the existing parser implementation and use the shared header directory. The add_test statement registers the test executable with CTest. Building only metrics_test can leave an older sysmon binary in build/, so running that existing file would not demonstrate that the current application sources were rebuilt.

After saving the correction, run these from the project root:

```sh
cmake -S . -B build
cmake --build build
./build/sysmon
(cd build && ctest --output-on-failure)
```

Expect both targets in the build output, all three live measurements, and the registered memory_parser test passing. The normal memory test has passed. The user's screenshot now confirms both targets building, the live application printing all three measurements, and memory_parser passing. Reversed-order and missing-field automated tests will follow.

## Automated memory test lesson 2 — order and missing fields

The normal sample and combined application/test build have passed. The learner has added the following checks to tests/metrics_test.cpp. Insert them before its final success message and return 0, reusing the existing total_kb and available_kb variables.

```cpp
std::istringstream reversed_sample(
    "MemAvailable: 4096 kB\n"
    "MemTotal: 8192 kB\n"
);

if (!read_memory(reversed_sample, total_kb, available_kb)
    || total_kb != 8192 || available_kb != 4096) {
    std::cerr << "FAIL: reversed field order\n";
    return 1;
}

std::istringstream missing_sample("MemTotal: 8192 kB\n");

if (read_memory(missing_sample, total_kb, available_kb)
    || total_kb != 8192 || available_kb != -1) {
    std::cerr << "FAIL: missing available-memory field\n";
    return 1;
}
```

The reversed sample must parse successfully and produce the same two values. Its condition fails the test if the function returns false or either value differs. The missing-field sample must return false, preserve the read total, and leave available memory at -1. Its condition deliberately omits the negation before read_memory because returning true for incomplete input would be a failure. Reusing output variables also checks that the function resets its outputs on each call instead of keeping the previous 4096 value.

Change the final success message to `All memory parser checks passed`. Build with `cmake --build build` and run `(cd build && ctest --output-on-failure)`. No CMake changes are needed. CTest will still report one registered test, memory_parser, which now runs three checks inside one test executable. The user's latest screenshot demonstrates memory_parser passing after these checks were added. Source review confirms all three cases execute before return 0.

## Automated memory milestone complete

Source review confirms the normal-order, reversed-order, and missing-available-field checks all run before the test returns 0. The user's screenshot shows both targets building and CTest reporting memory_parser passed, with 0 failures out of 1 registered test. Together, the test checks parsing success, exact returned values, rejection of missing data, and resetting an output between calls. The test source, shared implementation, header, CMake registration, and documentation are ready for a checkpoint commit. Automated CPU checks remain future work.

## CPU tests lesson 1 — share the existing parser

On 2026-11-25, source review found read_cpu still defined in src/main.cpp. The memory-test milestone is saved as e243224, and local main matches origin/main. The following source edits are the next proposed learning exercise.

1. Move the entire existing read_cpu definition from main.cpp to metrics.cpp, after read_memory.
2. Add the declaration below to include/sysmon/metrics.h, before its closing endif.
3. Keep both read_cpu calls in print_snapshot. main.cpp already includes the project header, so those calls can use its declaration.

```cpp
bool read_cpu(std::istream& input,
              long long& total_ticks,
              long long& nonbusy_ticks);
```

The boolean return still reports parsing success. The stream reference accepts real-file or sample-text input, and the two integer references receive the calculated totals. The declaration ends in a semicolon. The function body, including its existing braces, moves to metrics.cpp.

Keep metrics.cpp's existing includes and using namespace std, which also support the CPU parser. Each target already compiles metrics.cpp and searches include/, so this step needs no CMake changes. Keep exactly one read_cpu definition.

Run from the project root:

```sh
cmake --build build
./build/sysmon
(cd build && ctest --output-on-failure)
```

Expected results are both targets building, all three live measurements, and the existing memory_parser test passing. These results are pending for this refactor. No CPU automated test has been added yet.

The following lesson will supply a known CPU line with total 100 and nonbusy 75 to check the parser. Percentage calculation is a separate operation and will need its own testable function later.

### Source move reviewed

The user's 2026-11-25 screenshot demonstrates the application running with all three measurements and the existing memory test passing after the move. Source inspection confirms read_cpu is defined in metrics.cpp and the application calls it through the shared header. The read_cpu declaration was placed after endif. Move that endif to the bottom so the guard covers both declarations. Repeated identical function declarations are permitted, which explains why this placement still compiled.

## CPU tests lesson 2 — a known counter line

The following test and CMake additions are proposed learner edits. Create tests/cpu_test.cpp:

```cpp
#include "sysmon/metrics.h"
#include <iostream>
#include <sstream>

int main() {
    std::istringstream sample("cpu 10 0 15 70 5 0 0 0 0 0\n");
    long long total_ticks;
    long long nonbusy_ticks;

    if (!read_cpu(sample, total_ticks, nonbusy_ticks)) {
        std::cerr << "FAIL: could not read CPU sample\n";
        return 1;
    }

    if (total_ticks != 100 || nonbusy_ticks != 75) {
        std::cerr << "FAIL: incorrect CPU totals\n";
        return 1;
    }

    std::cout << "CPU parser test passed\n";
    return 0;
}
```

| Code | Purpose |
| --- | --- |
| Project header include | Declare the existing parser used by the test. |
| iostream and sstream includes | Provide output and a stream containing known text. |
| istringstream sample | Supply the aggregate label and ten numeric counters in kernel field order. |
| The two integer declarations | Receive values initialized and calculated by read_cpu. |
| First condition | Report failure and return 1 if the sample cannot be parsed. |
| Second condition | Report failure if either calculated total differs from its expected value. |
| Final output and return | Report a successful test with exit status 0. |

The first eight counters sum to 100. The idle counter 70 plus iowait counter 5 gives nonbusy 75. This is a parser test for one snapshot, distinct from the percentage calculation between two snapshots.

Append the following to CMakeLists.txt, retaining the existing application and memory test targets:

```cmake
add_executable(cpu_test
    tests/cpu_test.cpp
    src/metrics.cpp
)
target_include_directories(cpu_test PRIVATE include)

add_test(NAME cpu_parser COMMAND cpu_test)
```

The new target builds a separate CPU test executable against the existing implementation. Its include-directory setting resolves the shared header. The add_test statement registers cpu_parser alongside memory_parser.

Run from the project root:

```sh
cmake -S . -B build
cmake --build build
(cd build && ctest --output-on-failure)
```

Expect two registered tests to pass. The user's screenshot demonstrates both memory_parser and cpu_parser passing. Source review confirms the known-input CPU test. Invalid labels and incomplete counter lines will be checked in a following lesson.

## CPU tests lesson 3 — reject invalid input

The normal CPU parser sample has passed. The next two cases are proposed additions to tests/cpu_test.cpp, before its final success message and return 0. Reuse the existing total_ticks and nonbusy_ticks variables.

```cpp
std::istringstream wrong_label("cpu0 10 0 15 70 5 0 0 0 0 0\n");

if (read_cpu(wrong_label, total_ticks, nonbusy_ticks)
    || total_ticks != -1 || nonbusy_ticks != -1) {
    std::cerr << "FAIL: accepted an incorrect CPU label or kept old totals\n";
    return 1;
}

std::istringstream incomplete("cpu 10 0 15\n");

if (read_cpu(incomplete, total_ticks, nonbusy_ticks)
    || total_ticks != -1 || nonbusy_ticks != -1) {
    std::cerr << "FAIL: accepted incomplete CPU counters or kept old totals\n";
    return 1;
}
```

| Case | Why it matters | Expected result |
| --- | --- | --- |
| cpu0 label with full counters | cpu0 represents an individual CPU, while this function expects the aggregate cpu line. | false and both outputs -1. |
| cpu label with three counters | The function needs at least the first eight numeric counters before calculating totals. | false and both outputs -1. |

Each istringstream supplies a fresh sample. The condition deliberately calls read_cpu without negation because returning true for either invalid sample would fail the test. The output comparisons verify that failure leaves the reset markers in place. The test main returns 1 for an unexpected result, which CTest detects.

After adding the blocks, rebuild with `cmake --build build` and run `(cd build && ctest --output-on-failure)`. CMake needs no changes. Expect the same two registered tests, with three cases inside cpu_test. These two additional cases passed in a fresh assistant build on 2026-11-25. The CPU usage percentage calculation is still separate and has not yet been automated.

## CPU tests lesson 4 — make the percentage calculation testable

On 2026-11-25 a fresh build passed memory_parser and cpu_parser, including all three CPU cases. The header guard now covers both declarations. Live output also succeeded. The next proposed learner edits extract the existing calculation and consistency guard.

Add this definition after read_cpu in src/metrics.cpp:

```cpp
bool calculate_cpu_usage(long long elapsed_ticks,
                         long long nonbusy_change,
                         double& cpu_percent) {
    cpu_percent = -1.0;

    if (elapsed_ticks <= 0 || nonbusy_change < 0 || nonbusy_change > elapsed_ticks) {
        return false;
    }

    long long busy_ticks = elapsed_ticks - nonbusy_change;
    cpu_percent = 100.0 * busy_ticks / elapsed_ticks;
    return true;
}
```

Add its declaration before endif in include/sysmon/metrics.h:

```cpp
bool calculate_cpu_usage(long long elapsed_ticks,
                         long long nonbusy_change,
                         double& cpu_percent);
```

| Part | Meaning and reason |
| --- | --- |
| First two parameters | Receive copies of the already-calculated counter differences. The calculation needs no file or delay. |
| double reference output | Let the caller receive the calculated percentage. |
| Assignment to -1.0 | Mark the output invalid until calculation succeeds. |
| Existing consistency condition | Reject zero or negative elapsed time and invalid nonbusy differences before division. |
| return false | Tell the caller calculation failed. The caller chooses the error message and exit status. |
| busy_ticks and percentage assignment | Reuse the existing arithmetic. 100.0 retains fractional percentages. |
| return true | Report that the percentage output is ready to use. |

In print_snapshot keep the two lines calculating elapsed_ticks and nonbusy_change. Replace the existing consistency check, busy_ticks calculation, and cpu_percent declaration/calculation with:

```cpp
double cpu_percent;

if (!calculate_cpu_usage(elapsed_ticks, nonbusy_change, cpu_percent)) {
    cerr << "Error: inconsistent CPU readings" << endl;
    return 1;
}
```

Keep the existing CPU output statement afterward. The function now owns arithmetic and its guard, while print_snapshot owns data collection, messages, and application exit behavior. CMake needs no changes because metrics.cpp is already compiled into all three targets.

Build, run, and check the existing tests:

```sh
cmake --build build
./build/sysmon
(cd build && ctest --output-on-failure)
```

The learner implemented the extraction. Source review confirms the header declaration, definition, and application call. The user's screenshot demonstrates live output and both existing parser tests passing. Direct calculation tests are the next lesson.

## CPU tests lesson 5 — known percentage and zero elapsed time

The calculation function is now used by the live program. Its direct automated checks are proposed below. Add these statements to tests/cpu_test.cpp before its final success message and return 0:

```cpp
double cpu_percent;

if (!calculate_cpu_usage(100, 75, cpu_percent) || cpu_percent != 25.0) {
    std::cerr << "FAIL: expected 25 percent CPU usage\n";
    return 1;
}

if (calculate_cpu_usage(0, 0, cpu_percent) || cpu_percent != -1.0) {
    std::cerr << "FAIL: zero elapsed time was not rejected correctly\n";
    return 1;
}
```

| Statement or case | What it checks |
| --- | --- |
| double cpu_percent | Reserve the function's output variable. The function initializes it on each call. |
| calculate_cpu_usage(100, 75, cpu_percent) | 100 total ticks and 75 nonbusy ticks leave 25 busy ticks, so the expected result is true with 25.0. |
| Negated call or mismatched percentage | Fail the test if valid data is rejected or its answer is wrong. The value 25.0 is exactly representable for this sample. |
| calculate_cpu_usage(0, 0, cpu_percent) | Expect false because division by zero must be prevented. |
| Unnegated call or output differing from -1.0 | Fail if invalid data is accepted or if the previous 25.0 output was not reset to the failure marker. |
| Diagnostic output and return 1 | Let CTest identify a failed test and show its diagnostic with --output-on-failure. |

Change the final success message to `All CPU checks passed`. Rebuild and run:

```sh
cmake --build build
(cd build && ctest --output-on-failure)
```

CMake needs no changes. The registered cpu_parser executable now checks parsing and calculation, so CTest will still list two tests overall. The user's screenshot now shows both registered tests passing after these cases were added. Source review confirms the direct 25% and zero-elapsed checks execute before success is returned. Negative or excessive nonbusy differences are separate cases to add afterward.

## CPU tests lesson 6 — remaining nonbusy guards

The direct 25% and zero-elapsed checks have passed. Add the following two cases before the final success message in tests/cpu_test.cpp. Reuse its cpu_percent variable.

```cpp
if (calculate_cpu_usage(100, -1, cpu_percent) || cpu_percent != -1.0) {
    std::cerr << "FAIL: negative nonbusy change was not rejected\n";
    return 1;
}

if (calculate_cpu_usage(100, 101, cpu_percent) || cpu_percent != -1.0) {
    std::cerr << "FAIL: excessive nonbusy change was not rejected\n";
    return 1;
}
```

| Inputs | Guard condition exercised | Expected outputs |
| --- | --- | --- |
| elapsed 100, nonbusy -1 | nonbusy_change less than zero | false and cpu_percent -1.0 |
| elapsed 100, nonbusy 101 | nonbusy_change greater than elapsed_ticks | false and cpu_percent -1.0 |

Each condition fails the test if the function accepts invalid differences or does not preserve its failure marker. These checks cover the remaining nonbusy conditions in the existing guard. They require no application or CMake changes. Rebuild with `cmake --build build` and run `(cd build && ctest --output-on-failure)`. Expect two registered tests to pass. The user's screenshot now shows both registered tests passing with these additional cases present. Source review confirms they run before the final success return. After they pass, the shared CPU parser and calculation tests form the next checkpoint milestone.

## CPU-testing milestone complete — 2026-11-25

Source review confirms all seven CPU cases execute before cpu_test returns success. The user's screenshot reports both memory_parser and cpu_parser passing. The CPU percentage helper is shared between the live application and this test executable.

| CPU check | Expected result | Evidence |
| --- | --- | --- |
| Known aggregate counter line | true, total 100, nonbusy 75 | Present in reviewed source and included in passing CTest run. |
| cpu0 label | false, both outputs -1 | Present in reviewed source and included in passing CTest run. |
| Incomplete counter line | false, both outputs -1 | Present in reviewed source and included in passing CTest run. |
| 100 elapsed, 75 nonbusy | true, percentage 25.0 | Present in reviewed source and included in passing CTest run. |
| Zero elapsed ticks | false, percentage -1.0 | Present in reviewed source and included in passing CTest run. |
| Negative nonbusy difference | false, percentage -1.0 | Present in reviewed source and included in passing CTest run. |
| Nonbusy difference greater than elapsed ticks | false, percentage -1.0 | Present in reviewed source and included in passing CTest run. |

CTest lists two registered test executables. The memory executable contains its three sample cases, and the CPU executable contains the seven cases above. This milestone is ready for a checkpoint commit. Earlier lesson sections preserve the steps used to reach it.

## GitHub Actions lesson 1 — prerequisites and workflow plan

Reviewed 2026-11-25. The workflow below is proposed learner work. The learner has now saved .github/workflows/ci.yml. Its contents match the workflow below. Configure, build, and test commands passed in a fresh local build; a GitHub run is still pending.

### Prerequisites

| Item | Purpose | Current project status |
| --- | --- | --- |
| GitHub repository and push access | Save the workflow and let GitHub receive triggering events. | RDagunan/sysmon-cli exists, and the user has pushed milestones successfully. |
| Committed source, header, CMake file, and tests | A fresh runner needs the same inputs used locally. | The CPU-testing milestone is committed as 1dd032a. |
| Passing local build and tests | Establish a baseline before investigating workflow-specific failures. | Both registered tests passed during the preceding milestone. |
| Repository Actions enabled with the checkout action allowed | Permit GitHub to execute the workflow and retrieve the source. | Repository settings have not been inspected. If Actions is disabled, review Settings > Actions > General. |
| Linux runner with CMake, a C++17 compiler, and a build tool | Build this Linux project. | The proposed ubuntu-24.04 hosted image includes GNU C++ and CMake. |

GitHub supplies the runner and its tools. The build uses the C++ standard library and the existing CMake tests. Checkout uses GitHub's automatic workflow token with contents-read permission. No custom secret is needed for this build. Standard GitHub-hosted runner usage in a public repository is free under the current documented policy.

References checked for this lesson:

- [GitHub Actions concepts and workflow placement](https://docs.github.com/en/actions/get-started/understand-github-actions)
- [Ubuntu 24.04 runner tool inventory](https://github.com/actions/runner-images/blob/main/images/ubuntu/Ubuntu2404-Readme.md)
- [Checkout action instructions](https://github.com/actions/checkout)
- [Checkout v7.0.1 release verified during preparation](https://github.com/actions/checkout/releases/tag/v7.0.1)
- [GitHub Actions billing policy](https://docs.github.com/en/actions/concepts/billing-and-usage)

### Setup steps

1. Create .github/workflows/ci.yml in the repository root.
2. Add the workflow below using spaces for YAML indentation.
3. Review, commit, and push the workflow alongside any documentation updates you want included.
4. Open the repository's Actions tab and select the Build and test run.
5. Confirm Checkout, Configure, Build, and Test succeed. Expand the failed step's logs if a run fails.

The workflow can run on a push, on a pull request, or manually from the Actions tab after it exists on the default branch.

```yaml
name: Build and test

on: [push, pull_request, workflow_dispatch]

permissions:
  contents: read

jobs:
  build-and-test:
    runs-on: ubuntu-24.04

    steps:
      - name: Checkout source
        uses: actions/checkout@v7

      - name: Configure
        run: cmake -S . -B build

      - name: Build
        run: cmake --build build

      - name: Test
        working-directory: build
        run: ctest --output-on-failure
```

| YAML entry | What it does and why |
| --- | --- |
| name | Labels the workflow in GitHub. |
| on | Lists the repository events that start it. push checks uploaded commits, pull_request checks proposed changes, and workflow_dispatch permits a manual run. |
| permissions / contents: read | Give the automatic workflow token read access to source contents. |
| jobs | Groups work into jobs. This workflow needs one job. |
| build-and-test | A chosen identifier for the job. |
| runs-on | Select GitHub's Ubuntu 24.04 runner image. |
| steps | Execute the following list in order on that runner. |
| A dash followed by name | Start a step and give it a readable label. |
| uses | Run the official checkout action, which downloads the repository into the workspace. v7 selects its major release. |
| Configure run | Generate build files with the same CMake command used locally. |
| Build run | Compile the application and both test executables. |
| working-directory: build | Run the following test command from the generated build directory. Each step otherwise starts in the checkout directory. |
| Test run | Execute CTest and print test diagnostics when a check fails. A nonzero result causes the step and job to fail. |

YAML uses indentation to show nesting. Keep two-space indentation consistent with this example. The proposed job builds the application and runs finite automated tests. Watch mode remains a manual interactive check.

From the project root, the file can be created with:

```sh
mkdir -p .github/workflows
nano .github/workflows/ci.yml
```

mkdir -p creates any missing parent directories. nano opens the new workflow file. After the learner adds the file, review its contents before the commit and first push. The workflow file has been saved and reviewed. Its commit, push, and remote run remain pending.

### Workflow reviewed and ready for first push

Reviewed the saved ci.yml: triggers, contents-read permission, Ubuntu runner, checkout action, and configure/build/test steps match the example. A fresh local configuration and build succeeded, and both registered tests passed. This verifies the project commands locally. The hosted runner, checkout step, and actual event trigger will be verified by the first GitHub run.

To save the workflow and accompanying documentation from the project root:

```sh
git add .github/workflows/ci.yml README.md CODE_GUIDE.md SESSION_LOG.md architecture.md
git commit -m "Add GitHub Actions build and test workflow"
git push origin main
```

The add command stages the reviewed workflow and documentation files. The commit records them locally, and push sends the commit to GitHub, triggering this workflow. Those Git commands have not been executed in this review. After pushing, open the repository's Actions tab and inspect the Build and test run. The first hosted-run result remains pending.

## CI milestone verified on GitHub — 2026-11-25

The learner committed and pushed the workflow as f2a5630. GitHub API checks confirm both runs completed successfully at commit f2a563038b42a0bdc9453c258913c57a8cccdcb7.

| Trigger | Run | Result |
| --- | --- | --- |
| Push | [Build and test #1](https://github.com/RDagunan/sysmon-cli/actions/runs/45570663053) | Checkout source, Configure, Build, and Test all succeeded. |
| Manual workflow_dispatch | [Build and test #2](https://github.com/RDagunan/sysmon-cli/actions/runs/45570812709) | Checkout source, Configure, Build, and Test all succeeded. |
| Pull request | Configured in ci.yml | Has not yet been exercised. |

A green workflow result means the hosted job completed successfully. Expand its steps on the run page to see the configure output, compiler output, and test report. The push and manual triggers are verified. Earlier lesson text describing the first run as pending records the preparation stage and is superseded by this result.

## Pull-request lesson — publish the CI verification notes

Push-triggered and manual CI runs have passed. The next proposed exercise uses the existing documentation updates to demonstrate a pull request and its CI trigger. No application changes are needed for this exercise.

From the source repository root:

```sh
git switch -c docs/ci-results
git add README.md CODE_GUIDE.md SESSION_LOG.md architecture.md
git commit -m "Document successful GitHub CI runs"
git push -u origin docs/ci-results
```

| Command | What it does |
| --- | --- |
| git switch -c docs/ci-results | Create and switch to a new branch. The current uncommitted documentation edits remain available on that branch. |
| git add with the four paths | Stage the CI result documentation and this lesson. |
| git commit | Save the staged documentation as a local commit on the branch. |
| git push -u origin docs/ci-results | Publish the branch and set its upstream. This also triggers the workflow's push event. |

On GitHub, create a pull request with base main and compare docs/ci-results. A suitable title is Document successful GitHub CI runs. The description can explain that it records the successful push and manual runs, updates the project status, and documents the pull-request learning exercise.

A branch is a named line of commits. A pull request proposes merging one branch into another and provides a place to review the diff and automated checks. Opening it triggers the workflow's pull_request event. Expect both push and pull-request runs for this branch. A green push run alone does not establish that the pull-request event ran.

After opening the pull request, inspect its Checks section and the Actions page. Confirm the Build and test workflow completed successfully for the pull-request event. Branch creation, commit, push, pull request, and its CI outcome are pending for this exercise. The lesson ends with a reviewable pull request. Merging will be a separate step after reviewing the diff and results.

## Pull-request exercise completed — 2026-11-25

The learner opened [PR #1](https://github.com/RDagunan/sysmon-cli/pull/1) from docs/ci-results into main. The reviewed diff contained four documentation files and no blocking issues. GitHub reported it mergeable. The [pull-request CI run](https://github.com/RDagunan/sysmon-cli/actions/runs/45574628586) passed Checkout, Configure, Build, and Test.

The learner merged the PR as 1acf37d6816d50ce5cd006a05e3a90c3fc188707. GitHub's [push run for the merged main](https://github.com/RDagunan/sysmon-cli/actions/runs/45574902968) also passed all four steps. This verifies push, manual, and pull-request CI triggers during the project exercises.

To sync the local application checkout, the assistant executed:

```sh
git switch main
git pull --ff-only origin main
```

The first command selected the existing local main branch. The second downloaded origin/main and advanced the local branch to the merge commit without creating another merge. The fast-forward succeeded. These commands are verified execution, and the documentation update recording them remains uncommitted. Earlier lesson statements marking PR creation or CI as pending describe the preparation stage.

## First-release lesson — add --help

Reviewed the source on 2026-11-26. main currently recognizes only --watch. The following is proposed learner code. Add this block at the start of main, before bool watch_mode:

```cpp
if (argc == 2 && string(argv[1]) == "--help") {
    cout << "SysMon CLI - Linux system monitor\n\n"
         << "Usage: sysmon [--watch | --help]\n\n"
         << "Options:\n"
         << "  --watch  Repeat readings until Ctrl+C.\n"
         << "  --help   Show this help and exit.\n"
         << "\nRun without options to print one snapshot.\n";
    return 0;
}
```

Also update the existing invalid-argument message in main:

```cpp
cerr << "Usage: sysmon [--watch | --help]" << endl;
```

| Code | Meaning and reason |
| --- | --- |
| argc == 2 | Require the program name plus exactly one option before reading argv[1]. |
| string(argv[1]) == "--help" | Compare the supplied option with the help flag. |
| Logical AND between the conditions | Only examine argv[1] after confirming it exists. |
| Chained cout statements | Send a short description, usage, and supported options to standard output. |
| Newline escapes | Format the help text into lines. Two consecutive newlines leave an empty line. |
| return 0 | Exit main successfully after showing help, before collecting any measurements or entering watch mode. |
| Updated usage error | Make the diagnostic reflect both supported options. |

The usage message uses square brackets for an optional argument and a vertical bar to indicate choosing one of the options. The current parser accepts one option per invocation. An invocation with extra options remains an error.

Build and check help:

```sh
cmake --build build && ./build/sysmon --help
echo $?
```

Expect help text and exit status 0. Uptime, memory, and CPU measurements should not print for --help. Those effects remain pending until the edit is made and run. After confirmation, check the default run, --watch, and an unknown option to verify existing behavior. The source code has not been edited by the assistant for this lesson.

## Help command verified — 2026-11-26

The user's screenshot demonstrates help output and exit status 0. Source review confirms the early --help branch and updated invalid-argument usage message. The first failed command used /build/sysmon, an absolute path from the filesystem root. The corrected ./build/sysmon uses the current project directory.

A fresh assistant build passed both CTest registrations. Additional command checks confirmed --help returns 0 and prints help without measurements, the default invocation returns 0 and prints all three measurements, and --oops returns 1 with usage on standard error. Watch mode was not rerun in this review. The body of the help if block should be indented one level deeper for consistency.

## First-release lesson — consistent CPU precision

This is the next proposed learner edit. Add this include near the other headers in src/main.cpp:

```cpp
#include <iomanip>
```

Replace the current CPU output statement with:

```cpp
cout << "CPU usage: " << fixed << setprecision(2)
     << cpu_percent << "%" << endl;
```

| Part | What it does and why |
| --- | --- |
| iomanip include | Supplies setprecision for controlling output formatting. |
| fixed | Select fixed-point formatting so precision counts digits after the decimal point. |
| setprecision(2) | Print two digits after the decimal point, rounding the displayed value. |
| cpu_percent | Print the existing calculated percentage with that formatting. |
| Percent text and endl | Add the unit and finish the line. |

For example, 1.50094 displays as 1.50 and 0 displays as 0.00. The stored number and calculation are unchanged by the output formatting. The formatting settings remain on cout for later floating-point output. Current uptime and memory outputs use integers, so they retain their current format during watch mode.

Rebuild and run with `cmake --build build && ./build/sysmon`. Expect exactly two digits after the decimal point on the CPU line. Results for this proposed change remain pending.

## Final local release checks — 2026-11-26

The learner added fixed and setprecision(2). The stored calculation stays unchanged while output now has two decimal places. A fresh assistant Release build at /tmp/sysmon-release-check-z2TGyj compiled all targets and passed both CTest registrations.

| Check executed against the fresh executable | Observed result |
| --- | --- |
| Default invocation | All three measurement lines and exit 0; CPU line has two decimal places. |
| --help | Help text without measurement lines and exit 0. |
| --oops | Usage on standard error and exit 1. |
| --watch --help | Usage on standard error and exit 1 for excess arguments. |
| --watch for four seconds, then SIGINT | Multiple complete snapshots, two-decimal CPU values, interrupted process. |

The command-line checks used bounded temporary subprocess calls and did not add a new repository test target. The existing CTest tests remain the parser/calculation checks. The local verification passed; hosted CI for these uncommitted finishing edits remains pending.

Two suggested style tidy-ups before the next commit are to write the standard header as `#include <iomanip>` and indent the help branch's return 0 and output continuations consistently. Quotes around iomanip compiled successfully here; angle brackets express its standard-library role.

The next steps are to commit and push the finishing source and documentation changes, inspect their CI result, then tag a first release after the final commit is verified. A proposed tag name is v0.1.0. No release tag has been created yet.
