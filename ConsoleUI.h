#pragma once

#include <string>
#include <vector>
#include <windows.h>

class Marquee;

class ConsoleUI {
private:
    HANDLE hConsole;
    int consoleWidth;
    int consoleHeight;

    static const int HEADER_HEIGHT = 7;
    static const int MARQUEE_AREA_HEIGHT = 12;
    static const int SEPARATOR_LINES = 2;
    static const int FEEDBACK_HEIGHT = 8;

    std::string inputBuffer;
    bool inputDirty;

    std::vector<std::string> feedbackLines;

    void setCursorPosition(int x, int y);
    void clearLine(int y);
    void clearRegion(int startY, int endY);
    void drawHorizontalBorder(int y);
    void hideCursor();
    void showCursor();
    int getPromptY() const;
    int getFeedbackStartY() const;

public:
    ConsoleUI();
    ~ConsoleUI();

    void initialize();
    void drawHeader();
    void drawMarqueeArea(const Marquee& marquee);
    void drawPrompt();
    void drawFeedback();
    void fullRedraw(const Marquee& marquee);

    void appendChar(char c);
    void removeLastChar();
    std::string submitInput();
    const std::string& getInputBuffer() const;
    void setInputDirty();

    void addFeedback(const std::string& message);
    void clearFeedback();

    int getConsoleWidth() const;
    int getConsoleHeight() const;
    int getMarqueeAreaHeight() const;
    int getMarqueeStartY() const;
};
