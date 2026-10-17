#include "Console.h"
#include "ConsoleUI.h"
#include "CommandInterpreter.h"
#include "Marquee.h"
#include "Scheduler.h"
#include <chrono>
#include <algorithm>
#include <iostream>
#include <thread>
int main() {
 try {
  auto console=makeConsole();
  ConsoleUI ui(*console);
  Marquee marquee;
  Scheduler scheduler;
  CommandInterpreter interpreter(marquee,scheduler);
  marquee.start();
  auto last=std::chrono::steady_clock::now(); bool done=false;
  while(!done) {
   if(ui.resize()) marquee.setArea(std::max(1,ui.getConsoleWidth()-1),ui.getMarqueeAreaHeight(),ui.getMarqueeStartY());
   for(int keys=0;keys<64;++keys) {
    int c=console->readKey(); if(c==-1) break;
    if(c==-2 || c==3 || c==4) { done=true; break; }
    if(c==13 || c==10) {
     auto result=interpreter.dispatch(ui.submitInput());
     auto feedback=interpreter.consumeFeedback();
     if(result==CommandResult::EXIT) { done=true; break; }
     if(feedback=="__CLEAR__") ui.clearFeedback(); else ui.addFeedback(feedback);
    } else if(c==8 || c==127) ui.removeLastChar();
    else if(c>=32 && c<=126) ui.appendChar(static_cast<char>(c));
   }
   auto now=std::chrono::steady_clock::now();
   if(now-last>=std::chrono::milliseconds(marquee.getSpeed())) { marquee.advance(); last=now; }
   ui.render(marquee);
   std::this_thread::sleep_for(std::chrono::milliseconds(8));
  }
  scheduler.shutdown();
 } catch(const std::exception& e) { std::cerr << "OS emulator: " << e.what() << '\n'; return 1; }
 return 0;
}
