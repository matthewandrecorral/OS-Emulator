#include "Console.h"
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <csignal>
#include <iostream>
#include <stdexcept>
namespace {
volatile std::sig_atomic_t interrupted=0;
void interruptHandler(int s) { interrupted=s; }
class PosixConsole final : public Console {
 termios original{}; struct sigaction oldInt{},oldTerm{},oldHup{};
 bool escape=false;
public:
 PosixConsole() {
  if(!isatty(STDIN_FILENO) || !isatty(STDOUT_FILENO) || tcgetattr(STDIN_FILENO,&original)!=0) throw std::runtime_error("Run in an interactive terminal.");
  auto raw=original; raw.c_lflag &= ~(ICANON|ECHO); raw.c_cc[VMIN]=0; raw.c_cc[VTIME]=0;
  if(tcsetattr(STDIN_FILENO,TCSANOW,&raw)!=0) throw std::runtime_error("Cannot configure terminal input.");
  struct sigaction action{}; action.sa_handler=interruptHandler; sigemptyset(&action.sa_mask);
  sigaction(SIGINT,&action,&oldInt); sigaction(SIGTERM,&action,&oldTerm); sigaction(SIGHUP,&action,&oldHup);
  std::cout << "\033[?1049h" << std::flush;
 }
 ~PosixConsole() override {
  tcsetattr(STDIN_FILENO,TCSANOW,&original);
  std::cout << "\033[?25h\033[?1049l" << std::flush;
  sigaction(SIGINT,&oldInt,nullptr); sigaction(SIGTERM,&oldTerm,nullptr); sigaction(SIGHUP,&oldHup,nullptr);
 }
 int readKey() override {
  if(interrupted) return -2;
  fd_set f; FD_ZERO(&f); FD_SET(STDIN_FILENO,&f); timeval t{};
  if(select(STDIN_FILENO+1,&f,nullptr,nullptr,&t)<=0) return -1;
  unsigned char c; auto n=read(STDIN_FILENO,&c,1); if(n==0) return -2; if(n<0) return -1;
  if(c==27) { escape=true; return -1; }
  if(escape) { if(c>=0x40 && c<=0x7e && c!='[' && c!='O') escape=false; return -1; }
  return c;
 }
 std::pair<int,int> dimensions() const override { winsize s{}; if(ioctl(STDOUT_FILENO,TIOCGWINSZ,&s)==0 && s.ws_col && s.ws_row) return {s.ws_col,s.ws_row}; return {80,24}; }
 void cursor(int x,int y) override { std::cout << "\033[" << y+1 << ';' << x+1 << 'H' << std::flush; }
 void visible(bool v) override { std::cout << (v?"\033[?25h":"\033[?25l") << std::flush; }
 void row(int y,const std::string& text) override { cursor(0,y); std::cout << text << std::flush; }
 void clear() override { std::cout << "\033[2J\033[H" << std::flush; }
};
}
std::unique_ptr<Console> makeConsole() { return std::make_unique<PosixConsole>(); }
