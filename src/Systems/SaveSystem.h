#pragma once

#include <string>

struct SaveData
{
    int highScore = 0;
    int highestLevel = 1;
    bool soundOn = true;
    int controlMode = 0; // 0 = Both, 1 = Swipe Only, 2 = D-Pad Only
    bool hapticsOn = true;
    bool campaignCompleted = false;
    int levelStars[25] = { 0 }; // Stars for levels 1 to 24

    int getTotalStars() const
    {
        int total = 0;
        for (int i = 1; i <= 24; ++i)
        {
            total += levelStars[i];
        }
        return total;
    }
};


class SaveSystem
{
public:
    static bool save(const SaveData& data, const std::string& filename = "save_data.txt");
    static bool load(SaveData& data, const std::string& filename = "save_data.txt");
};
