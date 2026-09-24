#include "ConsoleManager.h"

#include <iostream>
#include <iomanip>
#include <cstdlib>

using namespace std;

// Constructor
ConsoleManager::ConsoleManager()
    : commands(createCommands()) {

}

// Creates the list of available commands
vector<CommandInfo> ConsoleManager::createCommands() {
    vector<CommandInfo> list {
        {
            "Help",
            "help",
            "Displays the commands and its description."
        }, 

        {
            "Clear",
            "clear",
            "Performs a clear of the entire systemm outputs."
        }, 
        
        {
            "start_marquee",
            "start_marquee",
            "Starts the marquee \"animation.\""
        },

        {
            "stop_marquee",
            "stop_marquee",
            "Stops the marquee \"animation.\""
        },

        {
            "set_text",
            "set_text <text>",
            "Accepts text input and displays it as a marquee."
        },

        {
            "set_speed",
            "set_speed <milliseconds>",
            "Sets the marquee information refresh in milliseconds."
        },

        {
            "Initialize",
            "initialize",
            "Loads configuration and starts the system."
        }, 

        {
            "Screen",
            "screen",
            "N/A"
        },

        {
            "Start scheduler",
            "scheduler-start",
            "N/A"
        },

        {
            "Stop scheduler",
            "scheduler-stop",
            "N/A"
        },

        {
            "Report utilization",
            "report-util",
            "N/A"
        },

        {
            "Exit",
            "exit",
            "Terminates the console."
        }
    };

    return list;
}

void ConsoleManager::displayHeader() {
    cout << R"(
   ____ ____   ___  ____  _____ ______   __
  / ___/ ___| / _ \|  _ \| ____/ ___\ \ / /
 | |   \___ \| | | | |_) |  _| \___ \\ V /
 | |___ ___) | |_| |  __/| |___ ___) || |
  \____|____/ \___/|_|   |_____|____/ |_|

)";

    cout << "Hello, Welcome to CSOPESY commandline!" << endl;
    cout << "Type 'exit' to quit, 'clear' to clear screen.\n\n";
}

void ConsoleManager::clearScreen() {
    system("cls");
    displayHeader();
}

void ConsoleManager::displayHelp() {
    cout << left
         << setw(20) << "NAME"
         << setw(30) << "USAGE"
         << "DESCRIPTION"
         << endl;

    for (const CommandInfo& entry : commands) {
        cout << left
             << setw(20) << entry.name
             << setw(30) << ("\"" + entry.usage + "\"")
             << entry.description
             << endl;
    }

    cout << endl;
}

bool ConsoleManager::handleCommand() {
    if (command == "exit") {
        return false;
    }

    else if (command == "clear") {
        clearScreen();
    }

    else if (command == "help") {
        displayHelp();
    }

    else {
        cout << "Invalid command. Please try again.\n\n";
    }

    return true;
}

void ConsoleManager::run() {
    displayHeader();

    while (true) {
        cout << "Enter a command: ";

        if (!getline(cin, command)) {
            break;
        }

        cout << endl;

        if (!handleCommand()) {
            break;
        }
    }
}