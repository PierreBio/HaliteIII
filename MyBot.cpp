//BILLAUD Pierre / COUTURIEUX Victor STMN P5
//Fichiers initiaux
#include "hlt/game.hpp"
#include "hlt/constants.hpp"
#include "hlt/log.hpp"
#include "hlt/position.hpp"
//Fichiers créés
#include "hlt/state.hpp"
#include "hlt/strategy.h"
#include "hlt/geneticalgo.h"

#include <random>
#include <ctime>

using namespace std;
using namespace hlt;

int main(int argc, char* argv[]) {

    //Algorithme génétique
    GeneticAlgo * geneticAlgo = new GeneticAlgo();

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
    unordered_map<EntityId, State> stateShip;
    int mapSize = game.game_map->width;
    int countPlayers = game.players.size();

    //Initialisation de nouvelles données pour chaque joueur pour l'algorithme génétique
    geneticAlgo->initializePlayers(countPlayers, game.my_id);

    game.ready("Pierre_Victor");
    log::log("Successfully created bot! My Player ID is " +
             to_string(game.my_id) +
             ". Bot rng seed is " + to_string(rng_seed) +
             ". Filepath for genetics is : " +
             geneticAlgo->getParamOfPlayer(game.my_id).getFilepath()
             );

    //Stratégie initialisée
    std::shared_ptr<Strategy> strategy;
    strategy = make_shared<Strategy>(rng, game, countPlayers, mapSize, stateShip, geneticAlgo);

    for (;;) {
        game.update_frame();
        game.end_turn(strategy->turnGlobalStrategy());
        geneticAlgo->writeResults(game.my_id, game.me->halite, game.me->ships.size(), game.me->dropoffs.size() );
    }

    return 0;
}

