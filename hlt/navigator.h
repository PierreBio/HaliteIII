#pragma once

#include "types.hpp"
#include "position.hpp"
#include "ship.hpp"
#include "dropoff.hpp"
#include "game.hpp"
#include "constants.hpp"
#include "comparator.h"
#include "direction.hpp"
#include "tunables.h"
#include "shipStatus.hpp"

#include <random>
#include <set>

using namespace std;
using namespace hlt;

// The navigator stores information about map and navigation
// It make high level decisions about where to go.
// Planning : a navigator will just accept a ship and a mode (say,
// explore or return home). -- It will (1) return a direction or
class Navigator {
public:
    Navigator(shared_ptr<GameMap>& gameMap, shared_ptr<Player>& me,
              unordered_map<EntityId, ShipStatus>& shipStatus, mt19937& rng);

    int getPickUpThreshold() {return lowestHaliteToCollect_;}

    vector<Direction> createShip(shared_ptr<Ship> ship);
    vector<Direction> explore(shared_ptr<Ship> ship);
    vector<Direction> collect(shared_ptr<Ship> ship);
    vector<Direction> dropoffHalite(shared_ptr<Ship> ship);

private:
    mt19937& rng_;
    shared_ptr<GameMap> gameMap_;
    shared_ptr<Player> me_;
    vector<vector<Halite>> bestReturnRoute_;
    unordered_map<EntityId, ShipStatus>& shipStatus_;

    set<Position> usedPosiitons_;
    // what is the highest halite potential (halite/turn) in this map
    double maxHalitePotential_;
    // what is the lowest potential I should still consider going to navigate to
    double halitePotentialNavigateThreshold_;
    // what is the lowest halite I should still consider going to collect
    int lowestHaliteToCollect_;


    double calculateMaxHalitePotential();
    double calculateHalitePotentialNavigateThreshold(double maxHalitePotential);
    int calculateLowestHaliteToCollect();

    vector<Position> getSurroundingPositions(Position middlePos, int lookAhead);
    vector<Direction> wiggleDirectionsMostHalite(shared_ptr<Ship> ship);
    bool confusedExploringShipNearby(shared_ptr<Ship> ship);
};
