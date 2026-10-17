#include "Scheduler.h"
#include "CommandInterpreter.h"
#include "Marquee.h"
#include "ConsoleUI.h"
#include <fstream>
#include <iostream>
#include <stdexcept>
void require(bool ok,const char* message) { if(!ok) throw std::runtime_error(message); }
struct FakeConsole : Console {
 int width=80,height=24,writes=0;
 std::vector<std::string> rows=std::vector<std::string>(24);
 int readKey() override {return -1;}
 std::pair<int,int> dimensions() const override {return {width,height};}
 void clear() override {rows.assign(height,"");}
 void cursor(int x,int y) override {require(x>=0 && x<width && y>=0 && y<height,"cursor bounds");}
 void visible(bool) override {}
 void row(int y,const std::string& text) override {require(y>=0 && y<height && text.size()<static_cast<size_t>(width),"render bounds"); rows[y]=text; ++writes;}
};
int main() {
 auto dir=std::filesystem::temp_directory_path() / ("csopesy-test-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
 try {
  Scheduler scheduler(dir,std::chrono::milliseconds(1));
  Marquee marquee;
  FakeConsole console; ConsoleUI ui(console); ui.resize();
  marquee.setArea(79,ui.getMarqueeAreaHeight(),3);
  ui.appendChar('x'); ui.render(marquee); auto prompt=console.rows.back(); int writes=console.writes;
  ui.render(marquee); require(console.writes==writes,"unchanged frame writes");
  marquee.start(); marquee.advance(); ui.render(marquee); require(console.rows.back()==prompt,"prompt preservation");
  for(auto size: {std::pair<int,int>{20,10},{1,1},{120,40}}) {console.width=size.first; console.height=size.second; ui.resize(); marquee.setArea(size.first,ui.getMarqueeAreaHeight(),3); ui.render(marquee);}
  marquee.stop(); CommandInterpreter commands(marquee,scheduler);
  require(commands.dispatch("start_marquee")==CommandResult::OK && marquee.isRunning(),"start");
  commands.dispatch("stop_marquee"); require(!marquee.isRunning(),"stop");
  commands.dispatch("set_text portable"); require(marquee.getText()=="portable","text");
  commands.dispatch("set_speed 5"); require(marquee.getSpeed()==5,"speed");
  require(commands.dispatch("set_speed invalid")==CommandResult::BAD_ARGS,"bad speed");
  commands.dispatch("clear"); require(commands.consumeFeedback()=="__CLEAR__","clear");
  require(commands.dispatch("exit")==CommandResult::EXIT,"exit");
  commands.dispatch("help"); require(commands.consumeFeedback().find("scheduler-test")!=std::string::npos,"help");
  require(commands.dispatch("scheduler-test")==CommandResult::OK,"batch");
  bool four=false;
  for(int attempt=0;attempt<100 && !four;++attempt) {
   auto initial=scheduler.snapshot(); int running=0; for(auto p:initial) if(p.state=="Running") ++running;
   require(running<=4,"core limit"); four=running==4;
   if(!four) std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
  require(four,"four active cores");
  require(scheduler.waitForCompletion(std::chrono::seconds(10)),"completion timeout");
  auto order=scheduler.dispatchOrder(); require(order.size()==10,"dispatch count");
  auto states=scheduler.snapshot();
  for(int i=0;i<10;++i) {
   require(order[i]==i+1,"FCFS order");
   require(states[i].state=="Finished" && states[i].completed==100 && states[i].core>=0 && states[i].core<4,"process completion");
   std::ifstream file(dir/("process_"+std::to_string(i+1)+".txt")); std::string line; int count=0;
   require(static_cast<bool>(std::getline(file,line)),"log header");
   while(std::getline(file,line)) { ++count; require(line.find("instruction "+std::to_string(count)+"/100")!=std::string::npos,"log instruction"); }
   require(count==100,"log count");
  }
  commands.dispatch("screen -ls"); auto report=commands.consumeFeedback(); require(report.find("Cores available: 4")!=std::string::npos && report.find("process_10 Finished")!=std::string::npos,"screen output");
  scheduler.shutdown(); scheduler.shutdown();
  { Scheduler cancelled(dir/"cancelled",std::chrono::seconds(10)); cancelled.startBatch(); auto begin=std::chrono::steady_clock::now(); cancelled.shutdown(); require(std::chrono::steady_clock::now()-begin<std::chrono::seconds(2),"shutdown latency"); }
  auto blocker=dir/"blocked"; { std::ofstream f(blocker); f << "file"; }
  { Scheduler failed(blocker); bool rejected=false; try { failed.startBatch(); } catch(...) { rejected=true; } require(rejected,"file creation errors"); }
  std::filesystem::remove_all(dir);
  std::cout << "PASS: FCFS, ten 100-instruction logs, commands, I/O errors, shutdown\n";
 } catch(const std::exception& e) { std::cerr << e.what() << " | artifacts: " << dir << '\n'; return 1; }
}
