#include "ConsoleUI.h"
#include "Marquee.h"

#include <iostream>
#include <string>

ConsoleUI::ConsoleUI()
    : hConsole(GetStdHandle(STD_OUTPUT_HANDLE)),
      consoleWidth(120), consoleHeight(30),
      inputDirty(true) {
}

ConsoleUI::~ConsoleUI() {
    showCursor();
}

void ConsoleUI::initialize() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        consoleWidth = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        consoleHeight = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    }

    system("cls");
    hideCursor();
}

void ConsoleUI::setCursorPosition(int x, int y) {
    COORD pos = { static_cast<SHORT>(x), static_cast<SHORT>(y) };
    SetConsoleCursorPosition(hConsole, pos);
}

void ConsoleUI::clearLine(int y) {
    DWORD written;
    FillConsoleOutputCharacter(hConsole, ' ', consoleWidth, {0, static_cast<SHORT>(y)}, &written);
    FillConsoleOutputAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
                               consoleWidth, {0, static_cast<SHORT>(y)}, &written);
}

void ConsoleUI::clearRegion(int startY, int endY) {
    for (int y = startY; y <= endY; ++y) {
        clearLine(y);
    }
}

void ConsoleUI::drawHorizontalBorder(int y) {
    std::string border(consoleWidth, '*');
    DWORD written;
    WriteConsoleOutputCharacterA(hConsole, border.c_str(), static_cast<DWORD>(border.size()),
                                 {0, static_cast<SHORT>(y)}, &written);
}

void ConsoleUI::hideCursor() {
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
}

void ConsoleUI::showCursor() {
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = TRUE;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
}

int ConsoleUI::getPromptY() const {
    return HEADER_HEIGHT + SEPARATOR_LINES + MARQUEE_AREA_HEIGHT + SEPARATOR_LINES;
}

int ConsoleUI::getFeedbackStartY() const {
    return getPromptY() + 1;
}

void ConsoleUI::drawHeader() {
    const char* headerArt =
        "   ____ ____   ___  ____  _____ ______   __  "
        "\n  / ___/ ___| / _ \\|  _ \\| ____/ ___\\ \\ / /  "
        "\n | |   \\___ \\| | | | |_) |  _| \\___ \\\\ V /   "
        "\n | |___ ___) | |_| |  __/| |___ ___) || |    "
        "\n  \\____|____/ \\___/|_|   |_____|____/ |_|    ";

    setCursorPosition(0, 0);
    std::cout << headerArt;
    setCursorPosition(0, 5);
    std::cout << "Hello, Welcome to CSOPESY commandline!";
    setCursorPosition(0, 6);
    std::cout << "Type 'help' for commands, 'exit' to quit.";
}

void ConsoleUI::drawMarqueeArea(const Marquee& marquee) {
    int borderTopY = HEADER_HEIGHT;
    int borderBotY = HEADER_HEIGHT + SEPARATOR_LINES + MARQUEE_AREA_HEIGHT - 1;
    int areaStartY = HEADER_HEIGHT + 1;

    drawHorizontalBorder(borderTopY);
    drawHorizontalBorder(borderBotY);

    clearRegion(areaStartY, borderBotY - 1);

    std::string text = marquee.getText();
    if (!text.empty()) {
        int drawY = areaStartY + marquee.getPosY();
        int drawX = marquee.getPosX();

        if (drawY >= areaStartY && drawY < borderBotY) {
            int maxLen = consoleWidth - drawX;
            if (maxLen > 0) {
                std::string display = text.substr(0, maxLen);
                DWORD written;
                WriteConsoleOutputCharacterA(hConsole, display.c_str(),
                                             static_cast<DWORD>(display.size()),
                                             {static_cast<SHORT>(drawX), static_cast<SHORT>(drawY)},
                                             &written);
            }
        }
    }

    std::string status = marquee.isRunning() ? "[RUNNING]" : "[STOPPED]";
    status += " Speed: " + std::to_string(marquee.getSpeed()) + "ms";
    int statusX = consoleWidth - static_cast<int>(status.size()) - 1;
    if (statusX < 0) statusX = 0;
    int statusY = borderBotY - 1;
    DWORD written;
    WriteConsoleOutputCharacterA(hConsole, status.c_str(),
                                 static_cast<DWORD>(status.size()),
                                 {static_cast<SHORT>(statusX), static_cast<SHORT>(statusY)},
                                 &written);
}

void ConsoleUI::drawPrompt() {
    int y = getPromptY();
    clearLine(y);

    std::string prompt = "Command> " + inputBuffer;
    DWORD written;
    WriteConsoleOutputCharacterA(hConsole, prompt.c_str(),
                                 static_cast<DWORD>(prompt.size()),
                                 {0, static_cast<SHORT>(y)}, &written);
    inputDirty = false;
}

void ConsoleUI::drawFeedback() {
    int startY = getFeedbackStartY();
    clearRegion(startY, startY + FEEDBACK_HEIGHT - 1);

    int linesToShow = static_cast<int>(feedbackLines.size());
    if (linesToShow > FEEDBACK_HEIGHT) linesToShow = FEEDBACK_HEIGHT;

    int offset = static_cast<int>(feedbackLines.size()) - linesToShow;
    for (int i = 0; i < linesToShow; ++i) {
        const std::string& line = feedbackLines[offset + i];
        int y = startY + i;
        std::string truncated = line.substr(0, consoleWidth);
        DWORD written;
        WriteConsoleOutputCharacterA(hConsole, truncated.c_str(),
                                     static_cast<DWORD>(truncated.size()),
                                     {0, static_cast<SHORT>(y)}, &written);
    }
}

void ConsoleUI::fullRedraw(const Marquee& marquee) {
    drawHeader();
    drawMarqueeArea(marquee);
    drawPrompt();
    drawFeedback();
}

void ConsoleUI::appendChar(char c) {
    inputBuffer += c;
    inputDirty = true;
}

void ConsoleUI::removeLastChar() {
    if (!inputBuffer.empty()) {
        inputBuffer.pop_back();
        inputDirty = true;
    }
}

std::string ConsoleUI::submitInput() {
    std::string result = inputBuffer;
    inputBuffer.clear();
    inputDirty = true;
    return result;
}

const std::string& ConsoleUI::getInputBuffer() const {
    return inputBuffer;
}

void ConsoleUI::setInputDirty() {
    inputDirty = true;
}

void ConsoleUI::addFeedback(const std::string& message) {
    std::string line;
    for (char c : message) {
        if (c == '\n') {
            feedbackLines.push_back(line);
            line.clear();
        } else {
            line += c;
        }
    }
    if (!line.empty()) {
        feedbackLines.push_back(line);
    }

    if (feedbackLines.size() > 50) {
        feedbackLines.erase(feedbackLines.begin(),
                            feedbackLines.begin() + static_cast<int>(feedbackLines.size()) - 50);
    }
}

void ConsoleUI::clearFeedback() {
    feedbackLines.clear();
}

int ConsoleUI::getConsoleWidth() const {
    return consoleWidth;
}

int ConsoleUI::getConsoleHeight() const {
    return consoleHeight;
}

int ConsoleUI::getMarqueeAreaHeight() const {
    return MARQUEE_AREA_HEIGHT - 2;
}

int ConsoleUI::getMarqueeStartY() const {
    return HEADER_HEIGHT + 1;
}
