#include "CommandInterpreter.h"
#include "Marquee.h"

#include <algorithm>
#include <cctype>

CommandInterpreter::CommandInterpreter(Marquee& marqueeRef)
    : marquee(marqueeRef) {
    initCommandTable();
}

void CommandInterpreter::initCommandTable() {
    commandTable = {
        {"help",          "help",                      "Displays the commands and their descriptions."},
        {"start_marquee", "start_marquee",              "Starts the marquee animation."},
        {"stop_marquee",  "stop_marquee",               "Stops the marquee animation."},
        {"set_text",      "set_text <text>",            "Sets the marquee display text."},
        {"set_speed",     "set_speed <milliseconds>",   "Sets the marquee refresh interval in ms."},
        {"clear",         "clear",                      "Clears the screen."},
        {"exit",          "exit",                       "Terminates the console."}
    };
}

CommandResult CommandInterpreter::cmdHelp() {
    std::ostringstream oss;
    oss << std::left
        << std::setw(20) << "COMMAND"
        << std::setw(32) << "USAGE"
        << "DESCRIPTION" << "\n";

    for (const auto& entry : commandTable) {
        oss << std::left
            << std::setw(20) << entry.name
            << std::setw(32) << entry.usage
            << entry.description << "\n";
    }

    feedbackMessage = oss.str();
    return CommandResult::OK;
}

CommandResult CommandInterpreter::cmdStartMarquee() {
    marquee.start();
    feedbackMessage = "Marquee animation started.";
    return CommandResult::OK;
}

CommandResult CommandInterpreter::cmdStopMarquee() {
    marquee.stop();
    feedbackMessage = "Marquee animation stopped.";
    return CommandResult::OK;
}

CommandResult CommandInterpreter::cmdSetText(const std::string& args) {
    if (args.empty()) {
        feedbackMessage = "Error: set_text requires a text argument. Usage: set_text <text>";
        return CommandResult::BAD_ARGS;
    }
    marquee.setText(args);
    feedbackMessage = "Marquee text set to: \"" + args + "\"";
    return CommandResult::OK;
}

CommandResult CommandInterpreter::cmdSetSpeed(const std::string& args) {
    if (args.empty()) {
        feedbackMessage = "Error: set_speed requires a numeric argument. Usage: set_speed <milliseconds>";
        return CommandResult::BAD_ARGS;
    }

    // Validate that args is a valid positive integer
    for (char c : args) {
        if (!std::isdigit(static_cast<unsigned char>(c))) {
            feedbackMessage = "Error: '" + args + "' is not a valid number. Usage: set_speed <milliseconds>";
            return CommandResult::BAD_ARGS;
        }
    }

    int ms = 0;
    try {
        ms = std::stoi(args);
    } catch (...) {
        feedbackMessage = "Error: '" + args + "' is not a valid number. Usage: set_speed <milliseconds>";
        return CommandResult::BAD_ARGS;
    }

    if (ms <= 0) {
        feedbackMessage = "Error: speed must be a positive integer (in milliseconds).";
        return CommandResult::BAD_ARGS;
    }

    marquee.setSpeed(ms);
    feedbackMessage = "Marquee speed set to " + std::to_string(ms) + " ms.";
    return CommandResult::OK;
}

CommandResult CommandInterpreter::cmdClear() {
    feedbackMessage = "__CLEAR__";  // sentinel value handled by main loop
    return CommandResult::OK;
}

CommandResult CommandInterpreter::dispatch(const std::string& input) {
    feedbackMessage.clear();

    // Trim leading/trailing whitespace
    std::string trimmed = input;
    size_t start = trimmed.find_first_not_of(" \t");
    if (start == std::string::npos) {
        return CommandResult::OK;  // empty input, ignore
    }
    trimmed = trimmed.substr(start);
    size_t end = trimmed.find_last_not_of(" \t");
    if (end != std::string::npos) {
        trimmed = trimmed.substr(0, end + 1);
    }

    if (trimmed.empty()) {
        return CommandResult::OK;
    }

    // Split into command keyword and arguments
    std::string keyword;
    std::string args;
    size_t spacePos = trimmed.find(' ');
    if (spacePos == std::string::npos) {
        keyword = trimmed;
    } else {
        keyword = trimmed.substr(0, spacePos);
        args = trimmed.substr(spacePos + 1);
        // Trim leading spaces from args
        size_t argStart = args.find_first_not_of(" \t");
        if (argStart != std::string::npos) {
            args = args.substr(argStart);
        } else {
            args.clear();
        }
    }

    // Convert keyword to lowercase for case-insensitive matching
    std::string lowerKeyword = keyword;
    std::transform(lowerKeyword.begin(), lowerKeyword.end(), lowerKeyword.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    // Dispatch
    if (lowerKeyword == "help")           return cmdHelp();
    if (lowerKeyword == "start_marquee")  return cmdStartMarquee();
    if (lowerKeyword == "stop_marquee")   return cmdStopMarquee();
    if (lowerKeyword == "set_text")       return cmdSetText(args);
    if (lowerKeyword == "set_speed")      return cmdSetSpeed(args);
    if (lowerKeyword == "clear")          return cmdClear();
    if (lowerKeyword == "exit")           return CommandResult::EXIT;

    feedbackMessage = "Unknown command: '" + keyword + "'. Type 'help' for available commands.";
    return CommandResult::UNKNOWN;
}

const std::vector<CommandEntry>& CommandInterpreter::getCommandTable() const {
    return commandTable;
}

std::string CommandInterpreter::consumeFeedback() {
    std::string msg = feedbackMessage;
    feedbackMessage.clear();
    return msg;
}
