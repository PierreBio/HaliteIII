#pragma once

#include "hlt/types.hpp"
#include "hlt/position.hpp"
#include "hlt/ship.hpp"
#include "hlt/dropoff.hpp"
#include "hlt/game.hpp"
#include "hlt/constants.hpp"

#include <queue>
#include <stack>
#include <algorithm>
#include <random>

using namespace std;
using namespace hlt;


class StrategyMap {
public:

    // Create a movement map that keeps track of immediate movement and safety.
    StrategyMap(shared_ptr<GameMap>& gameMap, shared_ptr<Player>& me, int nPlayers_, mt19937& rng, bool collisionCenterOkay);

    void createDropoff(shared_ptr<Ship> ship);

    // Tell the map that the ship is intending to move in the following direction(s)
    // Adding a direction here implies that the second direction is
    // almost as good as the first one.
    void addIntent(shared_ptr<Ship> ship, vector<Direction> preferredDirs, bool ignoreOpponentFlag = false);

    // Create an intention to make ship
    void createShip();

    // Flush the outputs to Halite game engine
    vector<Command> processOutputsAndEndTurn(Game& game, shared_ptr<Player> me);

private:

    // The direction a ship is intending to go
    Direction currentDirection(shared_ptr<Ship> ship);

    // The immediate destination (next block) the ship is intending to
    // go.
    Position destinationPos(shared_ptr<Ship> ship);

    // Resolve the conflicts at all positions
    // by changing the intent of ships surrounding those positions.
    void solveIssues();

    // Is there an enemy ship in the surrounding positions
    bool hasEnemyPresence(Position& pos);

    // Does this block has conflict?
    bool conflict(Position& pos);

    // Does this ship has conflict?
    bool shipConflict(shared_ptr<Ship> ship);

    // Is there an ememy ship at position
    bool hasEnemyShip(Position& pos);

    // resolve all conflicts
    void iterateAndResolveConflicts();

    // resolve a conflict at the position middlePos
    void solveIssue(Position middlePos);

    // Redirect a ship until it has no conflict or has stopped.
    void redirectShip(shared_ptr<Ship> ship);

    // Change the direction of the ship to the next possible move.
    void changeToNextDirection(shared_ptr<Ship> ship);

    void redirectShips(vector<shared_ptr<Ship>> ships);

    static bool shipHasLessHalite(const shared_ptr<Ship>& ship1, const shared_ptr<Ship>& ship2);

    // Only call after resolve all conflicts()
    bool isFreeSpace(Position pos);

    shared_ptr<GameMap> gameMap_;
    shared_ptr<Player> me_;
    int nPlayers_;
    unordered_map<Position, vector<shared_ptr<Ship>>> shipsComingtoPos_;
    unordered_map<Position, queue<Direction>> shipDirectionQueue_;
    queue<Position> allConflicts_;
    vector<Command> command_queue_;
    mt19937& rng_;
    bool shouldMakeShip_;
    bool collisionCenterOkay_;

    unordered_map<Position, bool> shipIgnoresOpponent_;
};
