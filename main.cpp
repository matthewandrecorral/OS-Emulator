#include <iostream>
#include <string>
#include <cstdlib>

void displayHeader() {
    std::cout << R"(
   ____ ____   ___  ____  _____ ______   __
  / ___/ ___| / _ \|  _ \| ____/ ___\ \ / /
 | |   \___ \| | | | |_) |  _| \___ \\ V /
 | |___ ___) | |_| |  __/| |___ ___) || |
  \____|____/ \___/|_|   |_____|____/ |_|

)";

    std::cout << "Hello, Welcome to CSOPESY commandline!" << std::endl;
    std::cout << "Type 'exit' to quit, 'clear' to clear screen.\n\n";
    std::cout << "** IMPORTANT: Type 'initialize' to load config and start system. **\n\n";
}

void clearScreen() {
    std::system("cls");
    displayHeader();
}

bool handleCommand(const std::string& command) {
    if (command == "exit") {
        return false;
    }

    if (command == "clear") {
        clearScreen();
    } else if (
        command == "initialize" ||
        command == "screen" ||
        command == "scheduler-start" ||
        command == "scheduler-stop" ||
        command == "report-util"
    ) {
        std::cout << command << " command recognized. Doing something.\n\n";
    } else {
        std::cout << "** Invalid command. Please try again. **\n\n";
    }

    return true;
}

int main() {
    displayHeader();

    std::string command;

    while (true) {
        std::cout << "Enter a command: ";

        if (!std::getline(std::cin, command)) {
            break;
        }

        std::cout << std::endl;

        if (!handleCommand(command)) {
            break;
        }
    }

    return 0;
}