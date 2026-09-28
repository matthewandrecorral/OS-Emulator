#include "Marquee.h"

// NOTE: All non-atomic fields (text, posX, posY, dirX, dirY, areaWidth, areaHeight)
// assume single-threaded access (input → tick → redraw in one loop iteration).
// If animation is ever moved to its own thread, these fields need synchronization.

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
    // Intentionally reset position to top-left when text changes.
    // This gives predictable behavior rather than continuing mid-bounce
    // with potentially misaligned text (e.g., longer text clipping at edges).
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
    // Clamp position to new area bounds immediately.
    // This is what prevents drawMarqueeArea from hitting negative maxLen.
    // Safe because setArea is currently the ONLY mutation path for areaWidth/areaHeight.
    // If another path is added, it must also clamp — otherwise posX could exceed
    // the new bounds between here and the next advance() tick.
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

    // Bounce horizontally
    if (posX <= 0) {
        posX = 0;
        dirX = 1;
    } else if (posX >= maxX) {
        posX = maxX;
        dirX = -1;
    }

    // Bounce vertically
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
