//BILLAUD Pierre / COUTURIEUX Victor STMN P5
#include "strategy.h"
#include <unordered_set>

Strategy::Strategy(mt19937 &rng, Game &game,int & countPlayers, int & mapSize, unordered_map<EntityId, State>& stateShip, GeneticAlgo * geneticAlgo)
    :game_(game),
      stateShip_(stateShip),
      rng_(rng),
      countPlayers_(countPlayers),
      mapSize_(mapSize),
      totalDropoff(0),
      newDropoffThisTurn(false),
      geneticAlgo_(geneticAlgo)
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
    shared_ptr<GameMap>& gameMap = game_.game_map;

    //Tous les bateaux reviennent si la partie est bientôt finie
    bool backToDropoff = soonEndGame(game_);

    //StrategyMap qui va définir les meilleures zones à collecter
    StrategyMap * strategyMap = new StrategyMap(gameMap, me, game_.players.size(), rng_, backToDropoff);

    std::string sX = std::to_string(strategyMap->getTarget()->getPosition().x);
    std::string sY = std::to_string(strategyMap->getTarget()->getPosition().y);
    std::string sAreaHalite = std::to_string(strategyMap->getTarget()->getTotalHaliteArea());

    log::log("X : " +
             sX +
             ". Y : " + sY+ " Halite in area : " + sAreaHalite);

    //Navigator qui contient toutes les fonctions d'action
    Navigator navigator(gameMap, me, stateShip_, rng_, geneticAlgo_ );

    //On veut déterminer l'état actuel de chaque bateau pour prendre les bonnes décisions ensuites
    for (const auto& ship_iterator : me->ships) {
        shared_ptr<Ship> ship = ship_iterator.second;
        determineCurrentShipState(ship, me, game_, gameMap, stateShip_, navigator, backToDropoff);
    }

    newDropoffThisTurn = false;

    for (const auto& ship_iterator : me->ships) {
        shared_ptr<Ship> ship = ship_iterator.second;
        vector<Direction> futureDirections;

    int counter = 0;

    //Si toutes les conditions suivantes sont remplies, alors le bateau se transforme en base de collecte (on passe au bateau suivant)
    //Remarque : des erreurs surviennent pendant la compilation avec des and ou des &&, d'où l'enchevêtrement de conditions
    //Cause : inconnue (malgré de nombreuses recherches)
    if(totalDropoff < me->ships.size()){
            if(game_.turn_number < geneticAlgo_->getParamOfPlayer(game_.my_id).getTurnWhenBotStopSpawn()) {
                if(me->halite >= 5000) {
                    if(!newDropoffThisTurn) {
                        if(!closeDropoffs(game_, ship)){
                            if(gameMap->calculate_distance(ship->position, me->shipyard->position) >= (int)(gameMap->width/4)) {
                                strategyMap->createDropoff(ship);
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

            Position targetPos = Position(strategyMap->getTarget()->getPosition().x, strategyMap->getTarget()->getPosition().y);

            if(strategyMap->getTarget()->getPosition().x == 0) {
                targetPos = Position(strategyMap->getTarget()->getPosition().x+1,strategyMap->getTarget()->getPosition().y);
            }

            if(strategyMap->getTarget()->getPosition().y == 0) {
                targetPos = Position(strategyMap->getTarget()->getPosition().x,strategyMap->getTarget()->getPosition().y+1);
            }

            if(stateShip_[ship->id] == State::NEW) {
                futureDirections = navigator.explore(targetPos, ship);
            }
            else if (stateShip_[ship->id] == State::EXPLORE) {
                futureDirections = navigator.explore(targetPos, ship);
            }
            else if (stateShip_[ship->id] == State::COLLECT) {
                futureDirections = navigator.collect(ship);
            }
            else if (stateShip_[ship->id] == State::RETURN) {
                futureDirections = navigator.goToClosestDropoff(ship);
            }
        }

        //On stocke la décision dans la carte stratégique pour chaque bateau
        strategyMap->stockDecision(ship, futureDirections);
    }

    int counter = 0 ;

    //Une fois toutes les décision prises pour les bateaux : on décide de créer un nouveau bateau ou de ne rien faire
    if (game_.turn_number <= geneticAlgo_->getParamOfPlayer(game_.my_id).getTurnWhenBotStopSpawn() ) {
        if(me->halite >= constants::SHIP_COST) {
            if(me->ships.size() >=geneticAlgo_->getParamOfPlayer(game_.my_id).getNumberShipStopProduction()-5 ) {
                if(game_.turn_number > 110 ) {
                    if(totalDropoff < 4 ) {
                        if(me->halite < 4000) {
                            //On attend pour économiser les halites
                            counter +=1;
                        }
                    }
                }
            }

            if(counter == 0) {
                if(me->ships.size()*1000 <= me->halite ) {
                    strategyMap->createShip();
                }
            }
        }
    }

    // On regroupe toutes les décisions finales, on vérifie leur faisabilité et on les stocke dans "result"
    vector<Command> decisions = strategyMap->verifyAndRetrieveAllCommands(game_, me);

    // On retourne toutes les décisions
    return decisions;
}

//Vérifie si la partie est bientôt terminée
bool Strategy::soonEndGame(const Game & game)
{

    // Numéro du tour actuel
    int turn = game.turn_number;

    // Nombre maximum de tours que la partie accepte
    int maxturn = constants::MAX_TURNS;

    // Tours restants
    int remainingTurns = maxturn - turn;

    //Taille de la carte
    int mapsize = game.game_map->height;

    // Si le nombre de tours restants équivaut à/est plus petit qu'un tiers
    // de la taille de la carte alors la partie va bientôt terminer
    // (utile pour que les bateaux aient le temps de rentrer)
    if (remainingTurns <= mapsize / 3) {
        return true;
    }

    return false;
}

//Vérifie si le bateau doit collecter la case sur laquelle il est ou non
int Strategy::canCollectHalite(Game &game, shared_ptr<Ship> ship)
{
    shared_ptr<GameMap>& gameMap = game_.game_map;

    //On oublie pas : 25% de la case est récoltée lorsqu'on collecte
    int capacityMaxBeforeReturn = geneticAlgo_->getParamOfPlayer(game_.my_id).getMaxHaliteShipReturn();
    int capacityMaxBeforeMove = geneticAlgo_->getParamOfPlayer(game_.my_id).getMaxHaliteShipMove();

    if(ship->halite >= capacityMaxBeforeReturn) {
        return 0; //RETURN
    }

    if(ship->halite < (gameMap->at(ship->position)->halite*0.1)) {
        return 1; //COLLECT
    }

    if(gameMap->at(ship->position)->halite > capacityMaxBeforeMove) {
        return 1; //COLLECT
    }

    return 2; //EXPLORE

}

//Vérifie s'il y a des bases de collecte pas loin (selon la portée de l'algo génétique
bool Strategy::closeDropoffs (Game &game, shared_ptr<Ship> ship)
{
    int radius = geneticAlgo_->getParamOfPlayer(game_.my_id).getMaxDistShipSee();
    vector <Position> dropoffsPos;
    dropoffsPos.push_back(game.me->shipyard->position);

    for(auto dropoffs : game.me->dropoffs) {
        dropoffsPos.push_back(dropoffs.second->position);
    }

    for(Position pos : dropoffsPos) {
        if(game.game_map->calculate_distance(ship->position, pos) < radius) {
            return true;
        }
    }

    return false;
}

//Vérifie l'état de chaque bateau et définie leur état pour ensuite choisir leur action
void Strategy::determineCurrentShipState(shared_ptr<Ship> ship, shared_ptr<Player> me, Game &game, shared_ptr<GameMap>& game_map, unordered_map <EntityId,
                 State>& stateShip, Navigator &navigator, bool backToShipyard )
{

    //Si la partie est bientôt terminée alors chaque bateau passe à l'état retour
    if(backToShipyard) {
        stateShip[ship->id] = State::RETURN;
    }

    //Si le bateau était nouveau au tour précédent, alors il va maintenant être actif
    if(stateShip[ship->id] == State::NEW) {
        stateShip[ship->id] = State::EXPLORE;
    }

    //Si le statut du bateau n'est pas référencé alors on le passe en nouveau
    if(!stateShip.count(ship->id)){
        stateShip[ship->id] = State::NEW;
    }

    //Si le bateau est situé sur le chantier de départ, alors c'est un nouveau bateau
    if(ship->position == me->shipyard->position) {
        stateShip[ship->id] = State::NEW;
    }

    //Si le bateau est sur une base de collecte, on passe sont statut en nouveau
    for(auto dropoffs : me->dropoffs) {
        Position dropoffPos = dropoffs.second->position;

        if(ship->position == dropoffPos) {
            stateShip[ship->id] = State::NEW;
        }
    }

    //Si le bateau explore déjà
    if(stateShip[ship->id] == State::EXPLORE || stateShip[ship->id] == State::COLLECT) {
        if(canCollectHalite(game, ship) == 0) {
            stateShip[ship->id] = State::RETURN;
        }
        else if (canCollectHalite(game, ship) == 1) {
            stateShip[ship->id] = State::COLLECT;
        }
        else if (canCollectHalite(game, ship) == 2) {
            stateShip[ship->id] = State::EXPLORE;
        }
    }
}


