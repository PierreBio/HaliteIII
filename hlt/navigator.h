#pragma once
//BILLAUD Pierre / COUTURIEUX Victor STMN P5
//Fichiers initiaux
#include "types.hpp"
#include "position.hpp"
#include "ship.hpp"
#include "dropoff.hpp"
#include "game.hpp"
#include "constants.hpp"
//Fichiers créés
#include "direction.hpp"
#include "state.hpp"
#include "geneticalgo.h"

#include <random>
#include <set>
#include <stdlib.h>

using namespace std;
using namespace hlt;

// Classe permettant de prendre les décision globales liées à la navigation,
// à l'exploration et la collecte en fonction du potentiel des cases et du bateau
class Navigator {
public:
    Navigator(shared_ptr<GameMap>& gameMap, shared_ptr<Player>& me,
              unordered_map<EntityId, State>& stateShip, mt19937& rng,
              GeneticAlgo * geneticAlgo);

    vector<Direction> explore(const Position & position, shared_ptr<Ship> ship);
    vector<Direction> collect(shared_ptr<Ship> ship);
    vector<Direction> findDirection(const Position & position, shared_ptr<Ship> ship);
    vector<Direction> goToClosestDropoff(shared_ptr<Ship> ship);

private:
    mt19937& rng_;
    shared_ptr<GameMap> gameMap_;
    shared_ptr<Player> me_;
    unordered_map<EntityId, State>& stateShip_;
    GeneticAlgo * geneticAlgo_;
};
