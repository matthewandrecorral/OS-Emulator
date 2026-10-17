#pragma once
#include <array>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <filesystem>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
struct Process {
 int id=0, completed=0, total=100, core=-1;
 std::string state="Ready", error;
};
class Scheduler {
 mutable std::mutex mutex;
 std::condition_variable changed;
 std::deque<int> ready;
 std::array<int,4> slots{{-1,-1,-1,-1}};
 std::vector<Process> processes;
 std::vector<int> dispatched;
 std::thread dispatcher;
 std::array<std::thread,4> workers;
 bool stopping=false;
 std::filesystem::path directory;
 std::chrono::milliseconds delay;
 void dispatchLoop();
 void workerLoop(int core);
public:
 explicit Scheduler(std::filesystem::path logs="logs", std::chrono::milliseconds instructionDelay=std::chrono::milliseconds(10));
 ~Scheduler();
 Scheduler(const Scheduler&)=delete;
 Scheduler& operator=(const Scheduler&)=delete;
 void startBatch();
 void shutdown();
 bool waitForCompletion(std::chrono::milliseconds timeout);
 std::vector<Process> snapshot() const;
 std::vector<int> dispatchOrder() const;
 std::string report() const;
};
