#include "tunables.h"
#include <vector>
#include<sstream>

using namespace std;

std::vector<int> Tunables::playerNums;
std::vector<int> Tunables::mapSizes;
std::vector<int> Tunables::haliteAs;
int Tunables::playerNumKey_;
int Tunables::mapSizeKey_;
int Tunables::haliteAbundanceKey_;
std::unordered_map<std::string, double> Tunables::tunableDict_[Tunables::PLAYER_NUMS][Tunables::MAP_SIZES][Tunables::HALITE_AS];


Tunables::Tunables(std::string &pathToFolder, int playerNum, int mapSize, int haliteAbundance)
{
    playerNums = {2, 4};
    mapSizes = {32, 40, 48, 56, 64};
    haliteAs = {0, 1, 2};

    playerNumKey_ = -1;
    mapSizeKey_ = -1;
    haliteAbundanceKey_ = -1;
    for (unsigned int i = 0; i < playerNums.size(); i++) {
        if (playerNums[i] == playerNum)
            playerNumKey_ = i;
    }
    for (unsigned int i = 0; i < mapSizes.size(); i++) {
        if (mapSizes[i] == mapSize)
            mapSizeKey_ = i;
    }
    for (unsigned int i = 0; i < haliteAs.size(); i++) {
        if (haliteAs[i] == haliteAbundance)
            haliteAbundanceKey_ = i;
    }

    assert(playerNumKey_ != -1);
    assert(mapSizeKey_ != -1);
    assert(haliteAbundanceKey_ != -1);

    readAllCsv(pathToFolder);
}

void Tunables::readAllCsv(std::string &pathToFolder) {
    std::string startpath = pathToFolder + "/csv";

    for (unsigned int p = 0; p < playerNums.size(); p++) {
        for (unsigned int s = 0; s < mapSizes.size(); s++) {
            for (unsigned int h = 0; h < haliteAs.size(); h++) {
                std::string path = startpath + '/' + std::to_string(playerNums[p]) + '-' +
                                    std::to_string(mapSizes[s]) + '-' + std::to_string(haliteAs[h]) +
                                    ".csv";
                // std::cout << path << std::endl;
                readCsvToDict(path, tunableDict_[p][s][h]);
            }
        }
    }
}

void Tunables::readCsvToDict(std::string filename, std::unordered_map<std::string, double> &dict) {
    std::string val;
    std::ifstream file;
    file.open(filename);

    getline(file, val, ',');
    if (val == "") {
        std::ofstream outfile (filename);
    }
    getline(file, val, ',');
    getline(file, val, ',');
    getline(file, val, ',');
    getline(file, val, ',');
    getline(file, val);
    while(getline(file, val, ',')) {
            // name,val,mut,min,max,type,remark
            std::string name;
            double value;
            name = val;
            getline(file, val, ',');
            value = std::stof(val);
            dict[name] = value;
            getline(file, val, ',');
            getline(file, val, ',');
            getline(file, val, ',');
            getline(file, val, ',');
            getline(file, val);
            // std::cout << name << ' ' << dict[name] << std::endl;
    }
}

double Tunables::lookUpTunable(std::string name) {
    return tunableDict_[playerNumKey_][mapSizeKey_][haliteAbundanceKey_][name];
}
