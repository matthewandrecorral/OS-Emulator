#pragma once

#include <string>
#include <atomic>

class Marquee {
private:
    std::string text;
    int posX;
    int posY;
    int dirX;
    int dirY;
    int areaWidth;
    int areaHeight;
    int areaStartY;
    std::atomic<int> speed;
    std::atomic<bool> running;
    bool needsRedraw;

public:
    Marquee();

    void setText(const std::string& newText);
    std::string getText() const;

    void setSpeed(int ms);
    int getSpeed() const;

    void start();
    void stop();
    bool isRunning() const;

    void setArea(int width, int height, int startY);
    void advance();

    int getPosX() const;
    int getPosY() const;
    bool consumeRedraw();
};
