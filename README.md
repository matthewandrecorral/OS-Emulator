# CSOPESY OS Emulator

One C++17 console application for Windows 10/11 and macOS Intel and Apple Silicon. No GUI framework, Wine or VM is required. Use an interactive terminal; redirected input/output is rejected.

The emulator combines an animated marquee with a non-preemptive First-Come, First-Served (FCFS) scheduler. One dispatcher assigns ten processes to four CPU worker threads. Each process executes 100 instructions and writes its own log file while the command prompt remains available.

## Get the FCFS version

Clone the implementation branch:

```sh
git clone --branch FCFS-scheduler https://github.com/matthewandrecorral/OS-Emulator.git
cd OS-Emulator
```

For an existing checkout with your work saved, use `git fetch origin` followed by `git switch FCFS-scheduler`.

## Windows

Install MSYS2 from https://www.msys2.org/. In its UCRT64 terminal:

    pacman -Syu

Reopen UCRT64 if requested, then install:

    pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja

In PowerShell/Windows Terminal, from the repository root (adjust installation path if needed):

    $env:Path = "C:\msys64\ucrt64\bin;" + $env:Path
    cmake -S . -B build -G Ninja
    cmake --build build
    ctest --test-dir build --output-on-failure
    .\build\OS-Emulator.exe

Use modern MinGW-w64 with standard thread support (GCC 9+ recommended). The obsolete MinGW.org GCC 6.3 on this machine lacks C++17 filesystem support. Keep the compiler runtime DLL directory on PATH. Use a new build directory when switching compilers or CMake generators.

## macOS

Install Apple Command Line Tools:

    xcode-select --install

Install CMake from https://cmake.org/download/ and enable its command-line tools, or use your existing package manager (brew install cmake with Homebrew).

In Terminal, from the repository root:

    clang++ --version
    cmake -S . -B build -DCMAKE_CXX_COMPILER=clang++
    cmake --build build --parallel
    ctest --test-dir build --output-on-failure
    ./build/OS-Emulator

The compiler targets the native architecture: arm64 on Apple Silicon, x86_64 on Intel. Optional universal build: configure a separate directory with -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" and a compatible SDK.

## Direct builds and VS Code

Create build first. Compile exactly one console backend:

    # Windows (MSYS2 UCRT64 GCC)
    g++ -std=c++17 -pthread main.cpp ConsoleUI.cpp ConsoleWindows.cpp CommandInterpreter.cpp Marquee.cpp Scheduler.cpp -o build/OS-Emulator.exe
    # macOS
    clang++ -std=c++17 -pthread main.cpp ConsoleUI.cpp ConsolePosix.cpp CommandInterpreter.cpp Marquee.cpp Scheduler.cpp -o build/OS-Emulator

The checked-in root executable is a legacy binary and is not replaced; run the freshly built executable in build.

For VS Code, install C/C++ and CMake Tools extensions, and start VS Code with the selected compiler and CMake on PATH. Ctrl+Shift+B configures/builds via the included tasks (Windows uses Ninja). Run in the integrated terminal with the commands above. The launch configuration uses GDB on Windows and LLDB on macOS; if your debugger extension lacks LLDB support, select a native debugger configuration or run in Terminal. No debugger is needed to build or run.

## Commands

| Command | Behavior |
| --- | --- |
| `help` | Show the command reference. |
| `start_marquee` | Start or resume the animation. |
| `stop_marquee` | Pause the animation. |
| `set_text <text>` | Change the displayed marquee text. |
| `set_speed <milliseconds>` | Set a positive animation interval; smaller values move faster. |
| `scheduler-test` | Start the ten-process FCFS batch. |
| `screen -ls` | Show process states, instruction counts, assigned cores, CPU usage and errors. |
| `clear` | Clear feedback and redraw the screen. |
| `exit` | Cancel unfinished work, join threads and restore the terminal. |

After launching, try:

```text
set_text CSOPESY FCFS Scheduler
set_speed 30
scheduler-test
screen -ls
```

Run `screen -ls` again after a few seconds. A successful batch shows ten Finished processes at 100/100, 0% CPU usage and four available cores. Core IDs are numbered 0 through 3. The report is a snapshot and does not refresh automatically.

The batch starts on scheduler-test and then executes automatically. One batch is allowed per application session. Each instruction takes a simulated 10 ms by default. Logs are logs/process_1.txt through logs/process_10.txt, relative to the working directory. A new session's batch replaces these files. Finished logs have one header and 100 instruction records with UTC ISO 8601 timestamps. File creation/writing failures appear as command errors or Failed process status.

The marquee starts with built-in default text and a 100 ms interval; use the commands above to change it. No configuration file is required. Input supports printable ASCII, Enter and Backspace; navigation escape sequences are ignored. Long input scrolls horizontally. Enlarge very small terminals to see the marquee and full status list.

## Project files

| File | Purpose |
| --- | --- |
| `main.cpp` | Input polling, animation timing and application lifecycle. |
| `Console.h` | Shared console interface. |
| `ConsoleWindows.cpp` / `ConsolePosix.cpp` | Native console backends selected by CMake. |
| `ConsoleUI.h` / `ConsoleUI.cpp` | Terminal layout, prompt, feedback and changed-row rendering. |
| `CommandInterpreter.h` / `CommandInterpreter.cpp` | Shared command parsing and dispatch. |
| `Marquee.h` / `Marquee.cpp` | Animation state and movement. |
| `Scheduler.h` / `Scheduler.cpp` | Process model, FIFO dispatcher, four workers and logging. |
| `CMakeLists.txt` | C++17 build and CTest configuration. |
| `tests/emulator_tests.cpp` | Automated scheduler, command and rendering assertions. |
| [SCHEDULER_TEST_CASES.md](SCHEDULER_TEST_CASES.md) | Reviewed test cases, recorded results, coverage gaps and manual checks. |
| `.github/workflows/build.yml` | Windows and macOS build/test jobs. |

## Architecture and migration

Console.h is the portable console interface. ConsoleWindows.cpp retains Win32 output and nonblocking conio input without requiring VT support. ConsolePosix.cpp uses termios, select, ioctl and ANSI with an alternate screen. CMake selects one backend; main, rendering, command parsing and scheduling contain no OS conditionals. Standard steady-clock timing and sleep replace Sleep.

ConsoleUI composes separate marquee, feedback and prompt regions. It clips to terminal dimensions, reserves the final column against autowrap, writes only changed rows and reflows on resize. Only the main thread renders; workers never write to the terminal.

Backend scoped destruction restores Windows input mode and original cursor information, or POSIX termios and the normal screen with a visible cursor. Ctrl+C, POSIX SIGTERM/SIGHUP and Windows console control events request orderly shutdown. C++ exceptions use the same cleanup. Forced termination such as SIGKILL, task termination or power loss cannot guarantee restoration. Use stty sane and reset if a POSIX terminal needs recovery.

Scheduler owns one dispatcher and four standard C++ worker threads. A mutex protects the FIFO, core assignments and process state; condition variables coordinate dispatch, completion and cancellation. Workers run each assigned process to completion without preemption. Dispatch is FIFO; simultaneous availability is resolved by core number. Instruction interleaving and completion order depend on thread timing and need not match between operating systems. Each worker exclusively owns one process log; UTC conversion is separately synchronized. Shutdown wakes delayed workers, cancels queued work and joins all threads. Process is the common platform-independent state model.

## Verification

See [Scheduler test cases and review](SCHEDULER_TEST_CASES.md) for assertion-level coverage and limitations. The latest documented Windows run passed **1/1 CTest tests** in **5.07 seconds**. That one executable contains multiple assertions; it is not a count of individual scenarios.

Validated locally on Windows using MSYS2 MinGW-w64 GCC 16.2: warning-enabled direct build, CMake/Ninja build and CTest. Tests cover the original commands, four active cores, FCFS dispatch 1–10, ten processes at 100/100, ten complete ordered logs, screen output, directory failures, cancellation and repeated shutdown. Rendering tests use a fake console to check prompt preservation during animation, changed-row updates and small/large resize bounds.

Native macOS execution and interactive visual behavior have NOT been tested in this Windows environment. Run the macOS build/test commands on Intel and Apple Silicon Macs. The CI workflow provides native builds; its results are not claimed here.

Manual acceptance on each platform:

1. Run the new binary in an interactive terminal at least 80 columns by 30 rows.
2. Exercise help, start_marquee, set_text hello, set_speed 30, stop_marquee and start_marquee. Type slowly while animation runs; check responsiveness, flicker, lost input and region overlap.
3. Type a long command, use Backspace, and resize narrower/wider and shorter/taller during animation.
4. Run scheduler-test and immediately screen -ls. Confirm up to four Running processes. Repeat after several seconds: ten Finished at 100/100 and four available cores. Inspect all ten log files.
5. Run clear and exit. Check normal shell input, echo and cursor visibility.
6. Restart, start a batch and press Ctrl+C. On macOS also send SIGTERM from another terminal. Confirm quick exit, no hanging workers and restored terminal.
7. Run CTest, which checks FIFO dispatch independently of timestamps and completion order.
