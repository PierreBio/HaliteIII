#ifndef STRATEGY_H
#define STRATEGY_H

#include "constants.hpp"
#include "log.hpp"
#include "game.hpp"
#include "command.hpp"
#include "position.hpp"

#include "shipStatus.hpp"
#include "strategymap.h"
#include "navigator.h"
#include "tunables.h"

#include <vector>
#include <random>
#include <ctime>
#include <queue>
#include <stack>
#include <algorithm>
#include <unordered_map>
#include <string>

using namespace hlt;
using namespace std;

class Strategy
{
public:
    //Constructeur
    Strategy(mt19937 &rng, Game &game,int & countPlayers, int & mapSize, unordered_map<EntityId, ShipStatus>& stateShip);
    //Destructeur
    ~Strategy();

    virtual vector<Command> turnGlobalStrategy();
    bool soonEndGame(const Game & game);
    void determineCurrentShipState(shared_ptr<Ship> ship,
                                   shared_ptr<Player> me,
                                   Game &game, shared_ptr<GameMap>& game_map,
                                   unordered_map <EntityId,
                                   ShipStatus>& shipStatus,
                                   Navigator &navigator,
                                   bool backToShipyard
                                   );
    int calculateCurrentShipCapacity(Game& game);
    bool hasHighSurroundingHalite(Game &game, shared_ptr<Ship> ship);
    bool hasSurroundingDropOffs (Game &game, shared_ptr<Ship> ship);

private:
    Game game_;
    mt19937 rng_;
    int countPlayers_;
    int mapSize_;
    unordered_map<EntityId, ShipStatus>& stateShip_;

    int totalDropoff;
    bool newDropoffThisTurn;

};

#endif // STRATEGY_H
