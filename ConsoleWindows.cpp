#include "Console.h"
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <conio.h>
#include <stdexcept>
namespace {
volatile LONG interrupted = 0;
BOOL WINAPI interruptHandler(DWORD event) {
 if(event==CTRL_C_EVENT || event==CTRL_BREAK_EVENT || event==CTRL_CLOSE_EVENT) { InterlockedExchange(&interrupted,1); return TRUE; } return FALSE;
}
class WindowsConsole final : public Console {
 HANDLE in=GetStdHandle(STD_INPUT_HANDLE), out=GetStdHandle(STD_OUTPUT_HANDLE);
 DWORD originalMode=0; CONSOLE_CURSOR_INFO originalCursor{};
public:
 WindowsConsole() {
  if(!GetConsoleMode(in,&originalMode) || !GetConsoleCursorInfo(out,&originalCursor)) throw std::runtime_error("Run in an interactive terminal.");
  if(!SetConsoleMode(in,originalMode & ~(ENABLE_LINE_INPUT|ENABLE_ECHO_INPUT|ENABLE_QUICK_EDIT_MODE))) throw std::runtime_error("Cannot configure console input.");
  SetConsoleCtrlHandler(interruptHandler,TRUE);
 }
 ~WindowsConsole() override { SetConsoleMode(in,originalMode); SetConsoleCursorInfo(out,&originalCursor); SetConsoleCtrlHandler(interruptHandler,FALSE); }
 int readKey() override {
  if(InterlockedCompareExchange(&interrupted,0,0)) return -2;
  if(!_kbhit()) return -1;
  int c=_getch(); if(c==0 || c==224) { _getch(); return -1; } return c;
 }
 std::pair<int,int> dimensions() const override { CONSOLE_SCREEN_BUFFER_INFO b{}; if(!GetConsoleScreenBufferInfo(out,&b)) return {80,24}; return {b.srWindow.Right-b.srWindow.Left+1,b.srWindow.Bottom-b.srWindow.Top+1}; }
 void cursor(int x,int y) override { CONSOLE_SCREEN_BUFFER_INFO b{}; GetConsoleScreenBufferInfo(out,&b); SetConsoleCursorPosition(out,{static_cast<SHORT>(b.srWindow.Left+x),static_cast<SHORT>(b.srWindow.Top+y)}); }
 void visible(bool v) override { auto c=originalCursor; c.bVisible=v; SetConsoleCursorInfo(out,&c); }
 void row(int y,const std::string& text) override { CONSOLE_SCREEN_BUFFER_INFO b{}; GetConsoleScreenBufferInfo(out,&b); DWORD n; WriteConsoleOutputCharacterA(out,text.data(),static_cast<DWORD>(text.size()),{b.srWindow.Left,static_cast<SHORT>(b.srWindow.Top+y)},&n); }
 void clear() override { auto d=dimensions(); for(int y=0;y<d.second;++y) row(y,std::string(d.first,' ')); }
};
}
std::unique_ptr<Console> makeConsole() { return std::make_unique<WindowsConsole>(); }
