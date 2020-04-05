#include "hlt/game.hpp"
#include "hlt/constants.hpp"
#include "hlt/log.hpp"
#include "hlt/position.hpp"

#include "hlt/shipStatus.hpp"
#include "hlt/tunables.h"
#include "hlt/strategy.h"

#include <random>
#include <ctime>

using namespace std;
using namespace hlt;


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

int findHaliteAbundanceKey(Game game) {
    int averageHalite = findAverageHalite(game.game_map);
    if (averageHalite < 116) {
        return 0;
    }
    else if (averageHalite < 194) {
        return 1;
    }
    else {
        return 2;
    }
}

int main(int argc, char* argv[]) {
    string s = argv[0];
    string pathToFolder = s.substr(0, s.find_last_of("\\/"));

    unsigned int rng_seed;

    if (argc > 1) {
        rng_seed = static_cast<unsigned int>(stoul(argv[1]));
    } else {
        rng_seed = static_cast<unsigned int>(time(nullptr));
    }

    //générateur de nombre pseudo aléatoire
    //https://fr.wikipedia.org/wiki/Mersenne_Twister : mt19937
    mt19937 rng(rng_seed);

    //Création du jeu
    Game game;

    //Informations initiales
    unordered_map<EntityId, ShipStatus> stateShip;
    int mapSize = game.game_map->width;
    int countPlayers = game.players.size();
    Tunables(pathToFolder, countPlayers, mapSize, findHaliteAbundanceKey(game));

    // At this point "game" variable is populated with initial map data.
    // This is a good place to do computationally expensive start-up pre-processing.
    // As soon as you call "ready" function below, the 2 second per turn timer will start.
    game.ready("Pierre_Victor");
    log::log("Successfully created bot! My Player ID is " + to_string(game.my_id) + ". Bot rng seed is " + to_string(rng_seed) + ".");

    //Stratégie initialisée
    std::shared_ptr<Strategy> strategy;
    strategy = make_shared<Strategy>(rng, game, countPlayers, mapSize, stateShip);

    for (;;) {
        game.update_frame();
        game.end_turn(strategy->turnGlobalStrategy());
    }

    return 0;
}

