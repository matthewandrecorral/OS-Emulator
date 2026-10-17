#pragma once
#include <memory>
#include <string>
#include <utility>
class Console {
public:
 virtual ~Console() = default;
 virtual int readKey() = 0; // -1: no input, -2: EOF/interruption
 virtual std::pair<int,int> dimensions() const = 0;
 virtual void clear() = 0;
 virtual void cursor(int x,int y) = 0;
 virtual void visible(bool value) = 0;
 virtual void row(int y,const std::string& text) = 0;
};
std::unique_ptr<Console> makeConsole();
