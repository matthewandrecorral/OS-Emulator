#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

struct CommandEntry {
    std::string name;
    std::string usage;
    std::string description;
};

enum class CommandResult {
    OK,
    EXIT,
    UNKNOWN,
    BAD_ARGS
};

class Marquee;

class CommandInterpreter {
private:
    std::vector<CommandEntry> commandTable;
    Marquee& marquee;
    std::string feedbackMessage;

    void initCommandTable();

    CommandResult cmdHelp();
    CommandResult cmdStartMarquee();
    CommandResult cmdStopMarquee();
    CommandResult cmdSetText(const std::string& args);
    CommandResult cmdSetSpeed(const std::string& args);
    CommandResult cmdClear();

public:
    explicit CommandInterpreter(Marquee& marqueeRef);

    CommandResult dispatch(const std::string& input);

    const std::vector<CommandEntry>& getCommandTable() const;
    std::string consumeFeedback();
};
