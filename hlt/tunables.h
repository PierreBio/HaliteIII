#pragma once

// keeps how to calculate specified values
#include <fstream>
#include <tuple>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <cassert>

using namespace std;

class Tunables {
private:

    // Modify this as we have more independent variables
    // say, halite potential.
    static const int PLAYER_NUMS = 2;
    static const int MAP_SIZES = 5;
    static const int HALITE_AS = 3;

    static std::vector<int> playerNums;
    static std::vector<int> mapSizes;
    static std::vector<int> haliteAs;

    static int playerNumKey_;
    static int mapSizeKey_;
    static int haliteAbundanceKey_;

    static std::unordered_map<std::string, double> tunableDict_[PLAYER_NUMS][MAP_SIZES][HALITE_AS];

public:

    // How far wil a ship look at its surroundings
    static const int SHIP_LOOKS_AHEAD = 4;

    Tunables() {}
    Tunables(std::string &pathToFolder, int playerNum, int mapSize, int haliteAbundance);

    void readAllCsv(std::string &pathToFolder);
    void readCsvToDict(std::string filename, std::unordered_map<std::string, double> &dict);

    double lookUpTunable(std::string name);
};
