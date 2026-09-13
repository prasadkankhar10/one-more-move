#include "SaveSystem.h"
#include <fstream>
#include <iostream>

bool SaveSystem::save(const SaveData& data, const std::string& filename)
{
    std::ofstream outFile(filename);
    if (!outFile.is_open())
    {
        std::cerr << "Failed to open save file for writing: " << filename << std::endl;
        return false;
    }

    outFile << data.highScore << "\n";
    outFile << data.highestLevel << "\n";
    outFile << (data.soundOn ? 1 : 0) << "\n";
    outFile << data.controlMode << "\n";
    outFile << (data.hapticsOn ? 1 : 0) << "\n";
    outFile << (data.campaignCompleted ? 1 : 0) << "\n";
    for (int i = 1; i <= 24; ++i)
    {
        outFile << data.levelStars[i] << " ";
    }
    outFile << "\n";

    outFile.close();
    return true;
}

bool SaveSystem::load(SaveData& data, const std::string& filename)
{
    std::ifstream inFile(filename);
    if (!inFile.is_open())
    {
        // Return default values if file doesn't exist yet
        data.highScore = 0;
        data.highestLevel = 1;
        data.soundOn = true;
        data.controlMode = 0;
        data.hapticsOn = true;
        data.campaignCompleted = false;
        for (int i = 0; i <= 24; ++i) data.levelStars[i] = 0;
        return false;
    }

    if (!(inFile >> data.highScore)) data.highScore = 0;
    if (!(inFile >> data.highestLevel)) data.highestLevel = 1;
    
    int soundVal = 1;
    if (inFile >> soundVal)
    {
        data.soundOn = (soundVal != 0);
    }
    else
    {
        data.soundOn = true;
    }

    if (!(inFile >> data.controlMode))
    {
        data.controlMode = 0;
    }

    int hapticVal = 1;
    if (inFile >> hapticVal)
    {
        data.hapticsOn = (hapticVal != 0);
    }
    else
    {
        data.hapticsOn = true;
    }

    int campVal = 0;
    if (inFile >> campVal)
    {
        data.campaignCompleted = (campVal != 0);
    }
    else
    {
        data.campaignCompleted = false;
    }

    for (int i = 1; i <= 24; ++i)
    {
        int star = 0;
        if (inFile >> star)
        {
            data.levelStars[i] = star;
        }
        else
        {
            data.levelStars[i] = 0;
        }
    }

    inFile.close();
    return true;
}

