#pragma once

#include <string>

struct SaveData
{
    int highScore = 0;
    int highestLevel = 1;
    bool soundOn = true;
    int controlMode = 0; // 0 = Both, 1 = Swipe Only, 2 = D-Pad Only
};

class SaveSystem
{
public:
    static bool save(const SaveData& data, const std::string& filename = "save_data.txt");
    static bool load(SaveData& data, const std::string& filename = "save_data.txt");
};
