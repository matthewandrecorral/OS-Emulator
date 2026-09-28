#include "ConsoleUI.h"
#include "CommandInterpreter.h"
#include "Marquee.h"
#include "ConfigLoader.h"

#include <windows.h>
#include <conio.h>
#include <chrono>

int main() {
    ConsoleUI ui;
    Marquee marquee;
    CommandInterpreter interpreter(marquee);

    Config config = ConfigLoader::load("config.txt");
    marquee.setText(config.marqueeText);
    marquee.setSpeed(config.marqueeSpeed);

    ui.initialize();

    marquee.setArea(ui.getConsoleWidth(), ui.getMarqueeAreaHeight(), ui.getMarqueeStartY());

    if (config.marqueeRunning) {
        marquee.start();
    } else {
        marquee.stop();
    }

    ui.fullRedraw(marquee);

    auto lastMarqueeTick = std::chrono::steady_clock::now();
    bool shouldExit = false;

    while (!shouldExit) {
        auto now = std::chrono::steady_clock::now();

        if (_kbhit()) {
            int ch = _getch();

            if (ch == 13) {
                std::string input = ui.submitInput();

                if (!input.empty()) {
                    CommandResult result = interpreter.dispatch(input);
                    std::string feedback = interpreter.consumeFeedback();

                    if (result == CommandResult::EXIT) {
                        shouldExit = true;
                        continue;
                    }

                    if (feedback == "__CLEAR__") {
                        ui.clearFeedback();
                        system("cls");
                        marquee.setArea(ui.getConsoleWidth(), ui.getMarqueeAreaHeight(), ui.getMarqueeStartY());
                        ui.fullRedraw(marquee);
                    } else if (!feedback.empty()) {
                        ui.addFeedback(feedback);
                        ui.drawFeedback();
                    }
                }

                ui.drawPrompt();
            } else if (ch == 8) {
                ui.removeLastChar();
                ui.drawPrompt();
            } else if (ch == 0 || ch == 224) {
                _getch();
            } else if (ch >= 32 && ch <= 126) {
                ui.appendChar(static_cast<char>(ch));
                ui.drawPrompt();
            }
        }

        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastMarqueeTick).count();
        if (marquee.isRunning() && elapsed >= marquee.getSpeed()) {
            marquee.advance();
            lastMarqueeTick = now;
        }

        if (marquee.consumeRedraw()) {
            ui.drawMarqueeArea(marquee);
            ui.drawPrompt();
        }

        Sleep(1);
    }

    system("cls");
    return 0;
}