#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

struct Config {
    std::string marqueeText = "Hello world in marquee!";
    int marqueeSpeed = 100;
    bool marqueeRunning = true;
};

class ConfigLoader {
public:
    static Config load(const std::string& filepath) {
        Config config;
        std::ifstream file(filepath);

        if (!file.is_open()) {
            return config;
        }

        std::string line;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;

            size_t eqPos = line.find('=');
            if (eqPos == std::string::npos) continue;

            std::string key = line.substr(0, eqPos);
            std::string value = line.substr(eqPos + 1);

            key = trim(key);
            value = trim(value);

            std::string lowerKey = key;
            std::transform(lowerKey.begin(), lowerKey.end(), lowerKey.begin(),
                           [](unsigned char c) { return std::tolower(c); });

            if (lowerKey == "marquee_text") {
                if (!value.empty()) {
                    config.marqueeText = value;
                }
            } else if (lowerKey == "marquee_speed") {
                try {
                    int speed = std::stoi(value);
                    if (speed > 0) {
                        config.marqueeSpeed = speed;
                    }
                } catch (...) {
                }
            } else if (lowerKey == "marquee_running") {
                std::string lowerVal = value;
                std::transform(lowerVal.begin(), lowerVal.end(), lowerVal.begin(),
                               [](unsigned char c) { return std::tolower(c); });
                config.marqueeRunning = (lowerVal == "true" || lowerVal == "1" || lowerVal == "yes");
            }
        }

        return config;
    }

private:
    static std::string trim(const std::string& str) {
        size_t start = str.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) return "";
        size_t end = str.find_last_not_of(" \t\r\n");
        return str.substr(start, end - start + 1);
    }
};
