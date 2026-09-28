#include "Marquee.h"

Marquee::Marquee()
    : text("Hello world in marquee!"),
      posX(0), posY(0),
      dirX(1), dirY(1),
      areaWidth(80), areaHeight(10), areaStartY(8),
      speed(100),
      running(false),
      needsRedraw(true) {
}

void Marquee::setText(const std::string& newText) {
    text = newText;
    posX = 0;
    posY = 0;
    dirX = 1;
    dirY = 1;
    needsRedraw = true;
}

std::string Marquee::getText() const {
    return text;
}

void Marquee::setSpeed(int ms) {
    if (ms < 1) ms = 1;
    speed.store(ms);
}

int Marquee::getSpeed() const {
    return speed.load();
}

void Marquee::start() {
    running.store(true);
    needsRedraw = true;
}

void Marquee::stop() {
    running.store(false);
    needsRedraw = true;
}

bool Marquee::isRunning() const {
    return running.load();
}

void Marquee::setArea(int width, int height, int startY) {
    areaWidth = width;
    areaHeight = height;
    areaStartY = startY;
    int maxX = areaWidth - static_cast<int>(text.size());
    if (maxX < 0) maxX = 0;
    if (posX > maxX) posX = maxX;
    int maxY = areaHeight - 1;
    if (maxY < 0) maxY = 0;
    if (posY > maxY) posY = maxY;
    needsRedraw = true;
}

void Marquee::advance() {
    if (!running.load()) return;

    int textLen = static_cast<int>(text.size());
    int maxX = areaWidth - textLen;
    if (maxX < 1) maxX = 1;
    int maxY = areaHeight - 1;
    if (maxY < 1) maxY = 1;

    posX += dirX;
    posY += dirY;

    if (posX <= 0) {
        posX = 0;
        dirX = 1;
    } else if (posX >= maxX) {
        posX = maxX;
        dirX = -1;
    }

    if (posY <= 0) {
        posY = 0;
        dirY = 1;
    } else if (posY >= maxY) {
        posY = maxY;
        dirY = -1;
    }

    needsRedraw = true;
}

int Marquee::getPosX() const {
    return posX;
}

int Marquee::getPosY() const {
    return posY;
}

bool Marquee::consumeRedraw() {
    if (needsRedraw) {
        needsRedraw = false;
        return true;
    }
    return false;
}
