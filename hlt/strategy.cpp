#include "strategy.h"
#include <unordered_set>

Strategy::Strategy(mt19937 &rng, Game &game,int & countPlayers, int & mapSize, unordered_map<EntityId, ShipStatus>& stateShip)
    :game_(game),
      stateShip_(stateShip),
      rng_(rng),
      countPlayers_(countPlayers),
      mapSize_(mapSize),
      totalDropoff(0),
      newDropoffThisTurn(false)
{

}

Strategy::~Strategy()
{

}

// Tour de jeu (durée maximum : 2 secondes)
vector<Command> Strategy::turnGlobalStrategy()
{
    // Récupérer les données du moteur de jeu
    shared_ptr<Player> me = game_.me;
    shared_ptr<GameMap>& game_map = game_.game_map;

    //Nouveau tour donc on initialise les variables à 0
    newDropoffThisTurn = false;

    //Tous les bateaux reviennent si la partie est bientôt finie
    bool backToShipyard = soonEndGame(game_);

    //Initialisation de la carte stratégique (prise de décisions)
    StrategyMap movementMap = StrategyMap(game_map, me, game_.players.size(), rng_, backToShipyard);

    //Initialisation de la navigation
    Navigator navigator = Navigator(game_map, me, stateShip_, rng_);

    //On veut déterminer l'état actuel de chaque bateau pour prendre les bonnes décisions ensuites
    for (const auto& ship_iterator : me->ships) {
        shared_ptr<Ship> ship = ship_iterator.second;
        determineCurrentShipState(ship, me, game_, game_map, stateShip_, navigator, backToShipyard);
    }

    Tunables tunables;

    //Pour chaque bateau, on va statuer sur le choix d'une action
    for (const auto& ship_iterator : me->ships) {
        shared_ptr<Ship> ship = ship_iterator.second;
        vector <Direction> nextDirs;

    int counter = 0;

    //Si toutes les conditions suivantes sont remplies, alors le bateau se transforme en base de collecte (on passe au bateau suivant)
        if(totalDropoff < int(me->ships.size() / 13)){
            if(game_.turn_number <= constants::MAX_TURNS - tunables.lookUpTunable("noProdTurn")+50) {
                if(me->halite >= 5000) {
                    if(!newDropoffThisTurn) {
                        if(hasHighSurroundingHalite(game_, ship)){
                            if(!hasSurroundingDropOffs(game_, ship)){

                                movementMap.createDropoff(ship);
                                newDropoffThisTurn = true;
                                totalDropoff += 1;
                                counter += 1;
                                continue;
                            }
                        }
                    }
                }
            }
        }
    //Sinon on regarde l'état de chaque bateau et on prend une décision en conséquence
        if(counter == 0) {
            if(stateShip_[ship->id] == ShipStatus::NEW) {
                nextDirs = navigator.createShip(ship);
            }
            else if (stateShip_[ship->id] == ShipStatus::EXPLORE) {
                nextDirs = navigator.explore(ship);
            }
            else if (stateShip_[ship->id] == ShipStatus::COLLECT) {
                nextDirs = navigator.collect(ship);
            }
            else if (stateShip_[ship->id] == ShipStatus::RETURN) {
                nextDirs = navigator.dropoffHalite(ship);
            }
        }

        //On stock la décision dans carte stratégique pour chaque bateau
        movementMap.addIntent(ship, nextDirs);
    }

    int counter = 0 ;

    //Une fois toutes les décision prises pour les bateaux : on décide de créer un nouveau bateau ou de ne rien faire
    if (game_.turn_number <= constants::MAX_TURNS - tunables.lookUpTunable("noProdTurn") ) {
        if(me->halite >= constants::SHIP_COST) {
            if(me->ships.size() >= 16 ){
                if(totalDropoff < int(me->ships.size() / 13)) {
                    if(game_.turn_number > 110 ) {
                        if(totalDropoff < 4 ) {
                            if(me->halite < 5000) {
                                //On attend pour économiser les halites
                                counter +=1;
                            }
                        }
                    }
                }
            }

            if(counter == 0) {
                movementMap.createShip();
            }
        }
    }

    //On regroupe toutes les décisions finales, on vérifie leur faisabilité et on les stocke dans "result"
    vector<Command> result = movementMap.processOutputsAndEndTurn(game_, me);

    return result;

}

//Vérifie sir la partie est bientôt terminée
bool Strategy::soonEndGame(const Game & game) {

    // Numéro du tour actuel
    int turn = game.turn_number;

    // Nombre maximum de tours que la partie accepte
    int maxturn = constants::MAX_TURNS;

    // Tours restants
    int remainingTurns = maxturn - turn;

    //Taille de la carte
    int mapsize = game.game_map->height;

    // Si le nombre de tours restants équivaut à/est plus petit que la moitié
    // de la taille de la carte alors la partie va bientôt terminer
    // (utile pour que les bateaux aient le temps de rentrer)
    if (remainingTurns <= mapsize / 2) {
        return true;
    }

    return false;
}


//Vérifie l'état de chaque bateau et change ce dernier si besoin
void Strategy::determineCurrentShipState(shared_ptr<Ship> ship, shared_ptr<Player> me, Game &game, shared_ptr<GameMap>& game_map, unordered_map <EntityId,
                 ShipStatus>& shipStatus, Navigator &navigator, bool backToShipyard ) {

    //Si la partie est bientôt terminée alors chaque bateau passe à l'état retour
    if(backToShipyard) {
        shipStatus[ship->id] = ShipStatus::RETURN;
    }

    if(shipStatus[ship->id] == ShipStatus::NEW) {
        shipStatus[ship->id] = ShipStatus::EXPLORE;
    }

    if(!shipStatus.count(ship->id)){
        shipStatus[ship->id] = ShipStatus::NEW;
    }

    if(ship->position == me->shipyard->position) {
        shipStatus[ship->id] = ShipStatus::NEW;
    }

    for(auto dropoffPair : me->dropoffs) {
        Position dropoffPos = dropoffPair.second->position;

        if(ship->position == dropoffPos) {
            shipStatus[ship->id] = ShipStatus::NEW;
        }
    }

    if(shipStatus[ship->id] == ShipStatus::EXPLORE) {
        if(game_map->at(ship->position)->halite
                >= navigator.getPickUpThreshold()) {
            if(!ship->is_full()) {
                shipStatus[ship->id] = ShipStatus::COLLECT;
            }
            else if (ship->halite >= calculateCurrentShipCapacity(game)){
                shipStatus[ship->id] = ShipStatus::RETURN;
            }
        }
    }

    if(shipStatus[ship->id] == ShipStatus::COLLECT) {
        if(game_map->at(ship->position)->halite >= navigator.getPickUpThreshold()*2) {
            if(!(ship->halite >= calculateCurrentShipCapacity(game)+20)) {
                shipStatus[ship->id] = ShipStatus::COLLECT;
            }
            else if (ship->halite >= calculateCurrentShipCapacity(game)) {
                shipStatus[ship->id] = ShipStatus::RETURN;
            }
            else {
                shipStatus[ship->id] = ShipStatus::EXPLORE;
            }
        }
    }

}

//Vérifie si le bateau a la capacité ou non de recevoir des halites
int Strategy::calculateCurrentShipCapacity(Game& game) {
    Tunables tunables;

    //calcul du pourcentage de partie qui est effectué
    double turnRatio= (double)game.turn_number / constants::MAX_TURNS;

    int start = tunables.lookUpTunable("ShipCapStart");
    int end = tunables.lookUpTunable("ShipCapEnd");
    int diff = start - end;
    int shipCapacity = start - int(turnRatio * diff);
    return shipCapacity;
}

//Vérifie s'il y a beaucoup de halites autour du bateau
bool Strategy::hasHighSurroundingHalite(Game &game, shared_ptr<Ship> ship) {
    shared_ptr<GameMap>& game_map = game.game_map;

    int totalHalite = 0;
    for(int i = 0; i <=4; i++) {
        for (int j = 0; j <= 4 ; j++) {
            Position currPos = Position(ship->position.x + Position(i-2, j-2).x, ship->position.y + Position(i-2, j-2).y );
            totalHalite += game_map->at(currPos)->halite;
        }
    }

    return totalHalite >= 200*25;
}

//Vérifie s'il y a des bases de collecte pas loin : rayon = 10
bool Strategy::hasSurroundingDropOffs (Game &game, shared_ptr<Ship> ship) {
    int radius = 10;
    vector <Position> basePositions;
    basePositions.push_back(game.me->shipyard->position);

    for(auto dropoffPair : game.me->dropoffs) {
        basePositions.push_back(dropoffPair.second->position);
    }

    for(Position pos : basePositions) {
        if(game.game_map->calculate_distance(ship->position, pos) < radius) {
            return true;
        }
    }

    return false;
}


