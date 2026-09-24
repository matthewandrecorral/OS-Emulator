#pragma once

#include <string>
#include <vector>

using namespace std;

struct CommandInfo {
    string name;
    string usage;
    string description;

    CommandInfo(string n, string u, string d)
        : name(n), usage(u), description(d) {}
};

class ConsoleManager {
private:
    string command;
    vector<CommandInfo> commands;

    vector<CommandInfo> createCommands();

    void displayHeader();
    void clearScreen();
    void displayHelp();

    bool handleCommand();

public:
    ConsoleManager();

    void run();
};