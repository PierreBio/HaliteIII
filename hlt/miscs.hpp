#pragma once

int findAverageHalite(shared_ptr<GameMap> gameMap) {
    int totalHalite = 0;
    int size = gameMap->width;
    for(int x = 0; x < size; x++) {
        for(int y = 0; y < size; y++) {
            Position pos = Position(x, y);
            totalHalite += gameMap->at(pos)->halite;
        }
    }
    int averageHalite = totalHalite / size / size;
    return averageHalite;
}
