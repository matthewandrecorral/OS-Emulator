#include "Scheduler.h"
#include <fstream>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <stdexcept>
namespace {
std::string timestamp() {
 // std::gmtime uses shared storage; serialize conversion and copy/format.
 static std::mutex timeMutex;
 std::lock_guard<std::mutex> guard(timeMutex);
 auto t=std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
 auto tm=std::gmtime(&t); if(!tm) throw std::runtime_error("Cannot generate timestamp");
 std::ostringstream out; out << std::put_time(tm,"%Y-%m-%dT%H:%M:%SZ"); return out.str();
}
}
Scheduler::Scheduler(std::filesystem::path logs,std::chrono::milliseconds instructionDelay):directory(std::move(logs)),delay(instructionDelay) {
 try { for(int i=0;i<4;++i) workers[i]=std::thread(&Scheduler::workerLoop,this,i); dispatcher=std::thread(&Scheduler::dispatchLoop,this); }
 catch(...) { shutdown(); throw; }
}
Scheduler::~Scheduler() { shutdown(); }
void Scheduler::startBatch() {
 std::lock_guard<std::mutex> lock(mutex);
 if(stopping) throw std::runtime_error("Scheduler is shutting down");
 if(!processes.empty()) throw std::runtime_error("The ten-process batch has already been started");
 std::filesystem::create_directories(directory);
 processes.reserve(10);
 for(int i=0;i<10;++i) { Process p; p.id=i+1; processes.push_back(p); ready.push_back(i); }
 changed.notify_all();
}
void Scheduler::dispatchLoop() {
 std::unique_lock<std::mutex> lock(mutex);
 while(true) {
  changed.wait(lock,[&] { if(stopping) return true; if(ready.empty()) return false; for(int s:slots) if(s==-1) return true; return false; });
  if(stopping) return;
  for(int core=0;core<4 && !ready.empty();++core) if(slots[core]==-1) {
   int index=ready.front(); ready.pop_front(); slots[core]=index;
   auto& p=processes[index]; p.core=core; p.state="Running"; dispatched.push_back(p.id);
  }
  changed.notify_all();
 }
}
void Scheduler::workerLoop(int core) {
 std::unique_lock<std::mutex> lock(mutex);
 while(true) {
  changed.wait(lock,[&] { return stopping || slots[core]!=-1; });
  if(stopping) return;
  int index=slots[core]; int id=processes[index].id; int total=processes[index].total;
  lock.unlock();
  std::string failure;
  try {
   std::ofstream file(directory / ("process_"+std::to_string(id)+".txt"));
   file.exceptions(std::ios::failbit|std::ios::badbit);
   file << "Process " << id << " | Core " << core << " | " << timestamp() << '\n';
   for(int instruction=1;instruction<=total;++instruction) {
    lock.lock();
    bool cancelled=changed.wait_for(lock,delay,[&] { return stopping; });
    lock.unlock();
    if(cancelled) break;
    file << timestamp() << " Core " << core << " instruction " << instruction << "/" << total << " Hello world from process " << id << '\n';
    file.flush();
    lock.lock(); processes[index].completed=instruction; lock.unlock();
   }
   file.close();
  } catch(const std::exception& e) { failure=e.what(); }
  lock.lock();
  auto& p=processes[index]; p.error=failure;
  p.state=!failure.empty()?"Failed":(p.completed==p.total?"Finished":"Cancelled");
  slots[core]=-1; changed.notify_all();
  if(stopping) return;
 }
}
void Scheduler::shutdown() {
 { std::lock_guard<std::mutex> lock(mutex); stopping=true; for(int i:ready) processes[i].state="Cancelled"; ready.clear(); }
 changed.notify_all();
 if(dispatcher.joinable()) dispatcher.join();
 for(auto& worker:workers) if(worker.joinable()) worker.join();
 // Work assigned immediately before shutdown may not have entered a worker.
 std::lock_guard<std::mutex> lock(mutex);
 for(auto& p:processes) if(p.state=="Running") p.state="Cancelled";
}
bool Scheduler::waitForCompletion(std::chrono::milliseconds timeout) {
 std::unique_lock<std::mutex> lock(mutex);
 return changed.wait_for(lock,timeout,[&] { if(processes.empty()) return false; for(const auto& p:processes) if(p.state=="Ready" || p.state=="Running") return false; return true; });
}
std::vector<Process> Scheduler::snapshot() const { std::lock_guard<std::mutex> lock(mutex); return processes; }
std::vector<int> Scheduler::dispatchOrder() const { std::lock_guard<std::mutex> lock(mutex); return dispatched; }
std::string Scheduler::report() const {
 auto list=snapshot(); int busy=0; for(const auto& p:list) if(p.state=="Running") ++busy;
 std::ostringstream out; out << "CPU utilization: " << busy*25 << "% | Cores used: " << busy << " | Cores available: " << 4-busy << '\n';
 if(list.empty()) out << "No processes. Use scheduler-test to start 10 processes.\n";
 for(const auto& p:list) { out << "process_" << p.id << " " << p.state << " Core " << p.core << " " << p.completed << '/' << p.total; if(!p.error.empty()) out << " ERROR: " << p.error; out << '\n'; }
 return out.str();
}
