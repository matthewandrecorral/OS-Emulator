#include "ConsoleUI.h"
#include "CommandInterpreter.h"
#include "Marquee.h"

#include <windows.h>
#include <conio.h>
#include <chrono>

int main() {
    ConsoleUI ui;
    Marquee marquee;
    CommandInterpreter interpreter(marquee);

    ui.initialize();

    // Configure marquee area dimensions based on console
    marquee.setArea(ui.getConsoleWidth(), ui.getMarqueeAreaHeight(), ui.getMarqueeStartY());

    // Start marquee by default
    marquee.start();

    // Initial full draw
    ui.fullRedraw(marquee);

    auto lastMarqueeTick = std::chrono::steady_clock::now();
    bool shouldExit = false;

    while (!shouldExit) {
        auto now = std::chrono::steady_clock::now();

        // --- Input polling (non-blocking) ---
        if (_kbhit()) {
            int ch = _getch();

            if (ch == 13) {
                // Enter key: submit the command
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
                        // Update area in case console was resized
                        marquee.setArea(ui.getConsoleWidth(), ui.getMarqueeAreaHeight(), ui.getMarqueeStartY());
                        ui.fullRedraw(marquee);
                    } else if (!feedback.empty()) {
                        ui.addFeedback(feedback);
                        ui.drawFeedback();
                    }
                }

                ui.drawPrompt();
            } else if (ch == 8) {
                // Backspace
                ui.removeLastChar();
                ui.drawPrompt();
            } else if (ch == 0 || ch == 224) {
                // Special key (arrow keys, function keys, etc.) - consume the second byte and ignore
                _getch();
            } else if (ch >= 32 && ch <= 126) {
                // Printable ASCII character
                ui.appendChar(static_cast<char>(ch));
                ui.drawPrompt();
            }
        }

        // --- Marquee animation tick ---
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastMarqueeTick).count();
        if (marquee.isRunning() && elapsed >= marquee.getSpeed()) {
            marquee.advance();
            lastMarqueeTick = now;
        }

        // --- Redraw marquee area if needed ---
        if (marquee.consumeRedraw()) {
            ui.drawMarqueeArea(marquee);
            // Re-draw prompt to keep cursor position consistent
            ui.drawPrompt();
        }

        // Brief sleep to prevent CPU spinning (1ms)
        Sleep(1);
    }

    // Clean exit
    system("cls");
    return 0;
}