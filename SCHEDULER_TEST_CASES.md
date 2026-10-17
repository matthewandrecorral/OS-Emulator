# Scheduler test cases and review

Reviewed: 2026-10-17. Implementation: [Scheduler.cpp](Scheduler.cpp), [Scheduler.h](Scheduler.h). Automated assertions: [tests/emulator_tests.cpp](tests/emulator_tests.cpp).

## Latest execution

**PASS** on Windows using MSYS2 MinGW-w64 GCC 16.2 and CMake/Ninja. The existing build was up to date. CTest reported **1/1 tests passed**, with scheduler_and_commands taking **5.07 seconds** (5.08 seconds total). This is one test executable containing multiple assertions, rather than separately registered CTest cases. Native macOS execution was not available and is not claimed.

The main batch uses a temporary log directory and a 1 ms simulated instruction delay. Production defaults to 10 ms. Temporary artifacts are removed on success; failures print the artifact directory for inspection.

## Existing automated scheduler coverage

All cases below were reached and passed in the latest execution.

| ID | Test case | Procedure and expected result | Scope of assertion |
| --- | --- | --- | --- |
| SCH-01 | Start the batch through the command interpreter | Dispatch scheduler-test; expect CommandResult::OK. | Confirms the command accepts the first batch. |
| SCH-02 | Four active cores | Poll snapshots up to 100 times, sleeping 1 ms between attempts; observe exactly four Running processes and never more than four in the sampled snapshots. | Does not continuously monitor the entire run. Running means assigned to a worker slot. |
| SCH-03 | Automatic completion | Wait up to 10 seconds for terminal process states. | A separate check below ensures these states are Finished, rather than Failed or Cancelled. |
| SCH-04 | Exactly ten dispatches | Inspect dispatchOrder(); expect ten entries. | Combined with the next case, excludes missing or duplicate process dispatches in this run. |
| SCH-05 | FCFS dispatch order | Expect dispatchOrder() to equal 1, 2, 3, 4, 5, 6, 7, 8, 9, 10. | Checks FIFO assignment order; worker execution and completion order may differ. |
| SCH-06 | Completed instruction counts and valid core IDs | For each of the ten snapshots, require Finished, completed == 100, and core in 0–3. | Does not independently assert total == 100 or the snapshot vector size before indexing it. |
| SCH-07 | Ten readable process log files | Open process_1.txt through process_10.txt in the temporary directory and read a header line from each. | Confirms expected filenames and a readable first line; does not validate header contents or exclude extra files. |
| SCH-08 | Complete, ordered instruction records | After each header, require exactly 100 lines and the expected instruction N/100 token on successive lines for N = 1–100. | Does not validate every timestamp, process identifier or core identifier in each record. |
| SCH-09 | Completed screen -ls report | Dispatch screen -ls after completion; expect Cores available: 4 and process_10 Finished in feedback. | Checks selected report fields, rather than all ten rows or utilization while running. |
| SCH-10 | Repeated shutdown | Call shutdown() twice after the completed batch. | Confirms this sequential usage returns without throwing or hanging; simultaneous shutdown callers are not tested. |
| SCH-11 | Prompt shutdown with a submitted batch | Create a scheduler with a 10-second instruction delay, submit a batch, then immediately shut down; expect shutdown in less than two seconds. | Dispatch may not have occurred yet, so this does not guarantee interruption of an actively delayed worker. |
| SCH-12 | Invalid log-directory path | Create a regular file where the log directory should be, then call startBatch(); expect an exception. | Tests directory creation failure, not a worker encountering a log-open or mid-write failure. |

The same executable also tests marquee commands and rendering, which are outside this scheduler-focused inventory.

## Review findings and remaining cases

The passing suite covers the main FCFS assignment path. It does not establish race freedom or full error-path coverage. The following cases are **proposed**, not implemented or passed results:

| ID | Additional case | Expected verification |
| --- | --- | --- |
| NEXT-01 | Idle scheduler | Before submission, snapshot and dispatch order are empty; report shows no processes, zero usage and four available cores. |
| NEXT-02 | Duplicate batch submission | A second startBatch() is rejected during execution and after completion, without adding processes or dispatch entries. |
| NEXT-03 | Submission after shutdown | startBatch() rejects the request; no work or logs are created. |
| NEXT-04 | Active-worker cancellation | Wait until workers have begun instructions before shutdown; require prompt joins, no Ready/Running states afterward, and unfinished processes marked Cancelled. |
| NEXT-05 | Worker log-open failure | Pre-create one expected log filename as a directory; that process becomes Failed with a nonempty error, while other processes continue and cores become reusable. |
| NEXT-06 | Mid-write failure | Use controlled failure injection; expect Failed with an error and no claim of successful completion. Avoid relying on platform-specific device paths. |
| NEXT-07 | Complete report validation | Check all ten rows, every final count, zero final CPU usage and running CPU usage in increments of 25%. |
| NEXT-08 | Log metadata validation | Parse the header and every record; match process/core IDs and validate UTC timestamp format and instruction sequence. |
| NEXT-09 | Concurrent snapshot stress | Repeatedly read snapshots/reports while work runs; require valid states, monotonic counts, distinct active core assignments and no deadlock. Use a thread sanitizer where supported. |
| NEXT-10 | Non-preemptive execution | Sample each process across its lifetime; its core stays fixed, progress never resets, and it never returns to Ready after dispatch. |
| NEXT-11 | Destructor cleanup | Destroy an idle scheduler and an actively executing scheduler without an explicit shutdown; verify prompt return. |
| NEXT-12 | Stronger test diagnostics | Assert snapshot size and IDs before indexing; register focused tests or label failures by case ID. |
| NEXT-13 | Native Mac coverage | Build and run on Intel and Apple Silicon macOS; perform the interactive checks below. |

The four-active-core test depends on observing a state within a short polling window. A heavily loaded runner could miss that window. A synchronization hook would make this check more deterministic. Likewise, SCH-11 should explicitly establish active execution before measuring cancellation latency.

## Reproduce the automated run

Windows PowerShell, with MSYS2 UCRT64 tools on PATH:

~~~powershell
cmake -S . -B build -G Ninja
cmake --build build
ctest --test-dir build --output-on-failure -R scheduler_and_commands
~~~

macOS Terminal with Apple Clang and CMake:

~~~sh
cmake -S . -B build -DCMAKE_CXX_COMPILER=clang++
cmake --build build --parallel
ctest --test-dir build --output-on-failure -R scheduler_and_commands
~~~

Use the directory you configured consistently. The recorded Windows run used build-ninja. To repeat the suite and stop on the first failure:

~~~sh
ctest --test-dir build --output-on-failure -R scheduler_and_commands --repeat until-fail:10
~~~

The repeat command is provided for future verification; ten repetitions were not performed for this report.

## Manual scheduler acceptance

These checks remain manual and were not performed in this review:

1. Launch the freshly built emulator in an interactive terminal and run screen -ls before starting work. Expect no processes and four available cores.
2. Run scheduler-test, then immediately screen -ls. Expect up to four Running processes and remaining Ready processes. A late snapshot may already show completed work.
3. Keep the marquee running while typing commands. Confirm keyboard responsiveness and no prompt overwrites while workers execute.
4. After several seconds, repeat screen -ls. Expect ten Finished processes at 100/100, zero CPU usage and four available cores.
5. Inspect logs/process_1.txt through logs/process_10.txt. Each finished log should contain one header and 100 ordered instruction records.
6. Run scheduler-test again. Expect an error explaining the batch was already started.
7. Restart, submit a batch, and exit or press Ctrl+C during execution. Expect prompt return to the shell, no hanging threads and normal terminal input/cursor behavior. Unfinished logs may be partial by design.
8. Repeat on Windows, Intel macOS and Apple Silicon macOS. Compare dispatch order and final counts; timestamps, core reuse and completion order need not match.
