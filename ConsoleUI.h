#pragma once
#include "Console.h"
#include <string>
#include <vector>
class Marquee;
class ConsoleUI {
 Console& console;
 int width=80,height=24,marqueeHeight=6;
 std::string input;
 std::vector<std::string> feedback,previous;
public:
 explicit ConsoleUI(Console& c):console(c) {}
 bool resize();
 void render(const Marquee& marquee);
 void appendChar(char c) { if(input.size()<4096) input+=c; }
 void removeLastChar() { if(!input.empty()) input.pop_back(); }
 std::string submitInput() { auto s=input; input.clear(); return s; }
 void addFeedback(const std::string& message);
 void clearFeedback() { feedback.clear(); previous.clear(); console.clear(); }
 int getConsoleWidth() const { return width; }
 int getMarqueeAreaHeight() const { return marqueeHeight; }
 int getMarqueeStartY() const { return 3; }
};
