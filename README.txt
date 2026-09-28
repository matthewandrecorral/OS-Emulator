==============================================
  CSOPESY - MO3 Marquee Operator
==============================================

Group Members:
  - [Member 1 Name]
  - [Member 2 Name]
  - [Member 3 Name]
  - [Member 4 Name]

Section: [Your Section]
Version Date: September 2026

----------------------------------------------
  HOW TO BUILD AND RUN
----------------------------------------------

Compiler: MinGW g++ (C++17 or later)
Platform: Windows

Build command:
  g++ -std=c++17 -o OS-Emulator.exe main.cpp ConsoleUI.cpp CommandInterpreter.cpp Marquee.cpp

Run:
  .\OS-Emulator.exe

Entry point: main.cpp

----------------------------------------------
  FILE STRUCTURE
----------------------------------------------

  main.cpp               - Entry point; main loop with input polling and marquee animation
  ConsoleUI.h / .cpp     - Console rendering (header, marquee area, prompt, feedback)
  CommandInterpreter.h / .cpp - Command parsing and dispatch logic
  Marquee.h / .cpp       - Marquee state and bouncing animation logic
  README.txt             - This file

----------------------------------------------
  AVAILABLE COMMANDS
----------------------------------------------

  help                   - Displays all available commands
  start_marquee          - Starts the marquee animation
  stop_marquee           - Stops the marquee animation
  set_text <text>        - Changes the marquee display text
  set_speed <ms>         - Changes the marquee refresh interval (milliseconds)
  clear                  - Clears the screen
  exit                   - Terminates the program

----------------------------------------------
  DESIGN NOTES
----------------------------------------------

  - Uses non-blocking input (_kbhit/_getch) to allow simultaneous
    marquee animation and command input without threads.
  - Uses SetConsoleCursorPosition and WriteConsoleOutputCharacterA
    for flicker-free rendering instead of system("cls") per frame.
  - Marquee uses a bouncing-ball pattern within a bordered area.
  - All parameters (text, speed) are changeable at runtime.
