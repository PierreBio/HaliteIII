#ifndef STRATEGY_H
#define STRATEGY_H
//BILLAUD Pierre / COUTURIEUX Victor STMN P5
//Fichiers initiaux
#include "constants.hpp"
#include "log.hpp"
#include "game.hpp"
#include "command.hpp"
#include "position.hpp"
//Fichiers créés
#include "state.hpp"
#include "strategymap.h"
#include "navigator.h"
#include "geneticalgo.h"

#include <vector>
#include <random>
#include <ctime>
#include <queue>
#include <stack>
#include <algorithm>
#include <unordered_map>
#include <string>
#include <cassert>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

using namespace hlt;
using namespace std;

//Stratégie globale : Définit les états de chaque bateau et prend ensuite les décisions les plus globales selon ces états
class Strategy
{
public:
    Strategy(mt19937 &rng, Game &game,int & countPlayers, int & mapSize, unordered_map<EntityId, State>& stateShip, GeneticAlgo * geneticAlgo);
    ~Strategy();

    //Stratégie globale pour chaque tour
    virtual vector<Command> turnGlobalStrategy();

    //Vérifie si la fin du jeu est proche
    bool soonEndGame(const Game & game);

    //Vérifie si le bateau a la capacité ou non de recevoir des halites
    int canCollectHalite(Game &game, shared_ptr<Ship> ship);

    //Vérifie s'il y a des bases de collecte autour du bateau
    bool closeDropoffs (Game &game, shared_ptr<Ship> ship);

    //Détermine l'état du bateau pour chaque tour
    void determineCurrentShipState(shared_ptr<Ship> ship,
                                   shared_ptr<Player> me,
                                   Game &game, shared_ptr<GameMap>& game_map,
                                   unordered_map <EntityId,
                                   State>& stateShip,
                                   Navigator &navigator,
                                   bool backToShipyard
                                   );
private:
    Game game_;
    mt19937 rng_;
    int countPlayers_;
    int mapSize_;
    unordered_map<EntityId, State>& stateShip_;
    GeneticAlgo * geneticAlgo_;

    int totalDropoff;
    bool newDropoffThisTurn;

};

#endif // STRATEGY_H
