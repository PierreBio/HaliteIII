#pragma once
//BILLAUD Pierre / COUTURIEUX Victor STMN P5
//Fichiers initiaux
#include "hlt/types.hpp"
#include "hlt/position.hpp"
#include "hlt/ship.hpp"
#include "hlt/dropoff.hpp"
#include "hlt/game.hpp"
#include "hlt/constants.hpp"
//Fichiers créés
#include "cellfriendliness.h"

#include <queue>
#include <stack>
#include <algorithm>
#include <random>
#include <cassert>

using namespace std;
using namespace hlt;

// Carte stratégique qui analyse la carte, les positions de chaque bateau, leur prochaine action
// et qui les ajuste en conséquence (résoud les possibles problèmes de collision, de placement)
// avant de les renvoyer pour qu'elles soient effectuées
class StrategyMap {
public:

    StrategyMap(const shared_ptr<GameMap>& gameMap, const shared_ptr<Player>& me, const int & nPlayers_, mt19937& rng,const bool & backToDropoff);

    void createDropoff(const shared_ptr<Ship> & ship);
    bool closeToTarget(const shared_ptr<Ship> & ship);
    CellFriendliness * getTarget();
    void stockDecision(const shared_ptr<Ship> & ship, vector<Direction> preferredDirs);
    void createShip();
    vector<Command> verifyAndRetrieveAllCommands(const Game& game,
                                                 const shared_ptr<Player> & me);

private:

    void determineHaliteAreas();
    bool noIntentToCreateShip(Position pos);
    Direction destination(const shared_ptr<Ship> & ship);
    Position destinationPos(const shared_ptr<Ship> & ship);

    void solveIssues();
    void solveIssue(const Position & middlePos);
    void iterateAndResolveConflicts();
    bool hasEnemyPresence( Position pos);
    bool conflict(const Position& pos);
    bool shipConflict(const shared_ptr<Ship> & ship);
    bool hasEnemyShip(const Position& pos);
    void redirectShip(const shared_ptr<Ship> & ship);
    void redirectShips(vector<shared_ptr<Ship>> ships);
    void changeToNextDirection(const shared_ptr<Ship> & ship);
    static bool shipHasLessHalite(const shared_ptr<Ship>& ship1, const shared_ptr<Ship>& ship2);

    shared_ptr<GameMap> gameMap_;
    shared_ptr<Player> me_;
    int nPlayers_;
    vector<Command> command_queue_;

    unordered_map<Position, vector<shared_ptr<Ship>>> shipsComingtoPos_;
    unordered_map<Position, queue<Direction>> shipDirectionQueue_;
    unordered_map<Position, bool> shipIgnoresOpponent_;
    queue<Position> allConflicts_;

    mt19937& rng_;
    bool shouldMakeShip_;
    bool backToDropoff_;
    CellFriendliness * targetArea;
};
