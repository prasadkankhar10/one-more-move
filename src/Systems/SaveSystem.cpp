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

    inFile.close();
    return true;
}
