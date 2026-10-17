#include "ConsoleUI.h"
#include "Marquee.h"
#include <algorithm>
#include <sstream>
bool ConsoleUI::resize() {
 auto d=console.dimensions();
 if(d.first==width && d.second==height && !previous.empty()) return false;
 width=std::max(1,d.first); height=std::max(1,d.second);
 marqueeHeight=std::max(1,std::min(10,height-18)); previous.clear(); console.clear(); return true;
}
void ConsoleUI::addFeedback(const std::string& message) {
 std::istringstream lines(message); std::string line;
 while(std::getline(lines,line)) feedback.push_back(line);
 if(feedback.size()>200) feedback.erase(feedback.begin(),feedback.end()-200);
}
void ConsoleUI::render(const Marquee& marquee) {
 std::vector<std::string> rows(height);
 auto put=[&](int y,std::string s) { if(y>=0 && y<height) rows[y]=std::move(s); };
 int usable=std::max(0,width-1); // Avoid terminal autowrap, including bottom-right cell.
 int promptY=height-1;
 if(height>=10) {
  put(0,"CSOPESY OS Emulator | Type help for commands");
  put(1,(marquee.isRunning()?"[RUNNING] ":"[STOPPED] ")+std::string("Speed: ")+std::to_string(marquee.getSpeed())+"ms");
  put(2,std::string(usable,'*'));
  int y=3+marquee.getPosY();
  if(y<3+marqueeHeight) put(y,std::string(std::max(0,marquee.getPosX()),' ')+marquee.getText());
  put(3+marqueeHeight,std::string(usable,'*'));
  int start=4+marqueeHeight;
  int count=std::min(static_cast<int>(feedback.size()),std::max(0,promptY-start));
  for(int i=0;i<count;++i) put(start+i,feedback[feedback.size()-count+i]);
 } else {
  put(0,"Resize terminal for marquee");
  int count=std::min(static_cast<int>(feedback.size()),std::max(0,height-2));
  for(int i=0;i<count;++i) put(1+i,feedback[feedback.size()-count+i]);
 }
 std::string prompt="Command> "+input;
 if(static_cast<int>(prompt.size())>usable) prompt=prompt.substr(prompt.size()-usable);
 put(promptY,prompt);
 bool changed=false;
 for(int y=0;y<height;++y) {
  auto& row=rows[y];
  for(char& c:row) if(static_cast<unsigned char>(c)<32 || static_cast<unsigned char>(c)>126) c='?';
  row.resize(std::min(row.size(),static_cast<size_t>(usable))); row.append(usable-row.size(),' ');
  if(previous.size()!=rows.size() || row!=previous[y]) { if(!changed) console.visible(false); changed=true; console.row(y,row); }
 }
 previous=std::move(rows);
 if(!changed) return;
 console.cursor(std::min(static_cast<int>(prompt.size()),usable),promptY); console.visible(true);
}
