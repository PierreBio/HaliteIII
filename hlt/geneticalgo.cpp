//BILLAUD Pierre / COUTURIEUX Victor STMN P5
#include "geneticalgo.h"

using namespace std;

GeneticAlgo::GeneticAlgo()
{

}

void GeneticAlgo::initializePlayers(const int &countPlayers, const int & id)
{
    //Renouvelle le seed de la fonction rand pour chaque bot
    srand ( time(NULL) );
    if(id > 0) {
        srand ( time(NULL) - id*2 );
    }
    if(id > 1) {
        srand ( time(NULL)- id*2 );
    }
    if(id > 2) {
        srand ( time(NULL)- id*2 );
    }

    countPlayers_ = countPlayers;

    //Initialise les données des nouveaux joueurs
    string path;

    if(countPlayers_ == 4) {
        path = "saves/save4p_1";
    }
    else if (countPlayers_ == 2) {
        path = "saves/save2p_1";
    }

    // Toute la mécanique de l'algorithme génétique se lance
    // et a pour but de définir les 5 paramètres en fonction des
    // résultats obtenus dans les précédentes parties
    runGeneticAlgo(id, path);

    string full_path = filepath(path, id);

    writeFile(full_path, id);
}

string GeneticAlgo::filepath(const string& path, const int & id)
{
    string full_path = path + ".txt";

    //Si le fichier existe, on va déterminer un nouveau nom de fichier inutilisé
    if (fileExists(full_path)) {

        //On récupère le numéro du fichier, on le convertit en entier et on lui ajoute +1
        string currentNumberString = path.substr(path.find("_") + 1);
        int currentNumber = std::stoi( currentNumberString ) + 1;

        //On récupère la base du chemin passé en paramètre
        string basepath = path.substr(0, path.find("_"));

        //On convertit le nouveau numéro int en string
        string number = std::to_string(currentNumber);

        //On recompose un nouveau chemin à partir des nouveau éléments
        string new_path = basepath + "_" + number;

        //On teste si le nouveau chemin est valide
        return filepath(new_path, id);
    }

    listParam[id].setFilepath(path);

    //Si le fichier n'existe pas, alors on renvoie un chemin valide pour le créer
    return full_path;
}

vector<int> GeneticAlgo::readFile(const string& path)
{
    vector<int> array;

    std::ifstream file(path);
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            string currentNumberString = line.substr(line.find(":") + 1);
            int n = std::stoi( currentNumberString );
            array.push_back(n);
        }
        file.close();
    }

    return array;
}

void GeneticAlgo::writeFile(const string& path, const int & id)
{
    //convert string path into char[]
    char charPath[1024];
    strncpy(charPath, path.c_str(), sizeof(charPath));
    charPath[sizeof(charPath) - 1] = 0;

    //convert int into char
    char str[10];
    sprintf(str, "%d", id);

    //Create file and write informations issued from genetic algorithm on it
    std::ofstream f( charPath );
     if ( f ) {
        f << "shipVision:" << listParam[id].getMaxDistShipSee() << endl
        << "shipHaliteMove:" << listParam[id].getMaxHaliteShipMove() << endl
        << "shipHaliteReturn:" << listParam[id].getMaxHaliteShipReturn()  << endl
        << "noShipProduction:" << listParam[id].getNumberShipStopProduction()  << endl
        << "turnBotStopSpawn:" << listParam[id].getTurnWhenBotStopSpawn() ;
     }
}

void GeneticAlgo::writeResults(const int & id, const int & halite, const int & shipNumber, const int & dropoff)
{
    string full_path = listParam[id].getFilepath() + "_result.txt";

    //convert int into char
    char str[10];
    sprintf(str, "%d", id);

    //convert string path into char[]
    char charPath[1024];
    strncpy(charPath, full_path.c_str(), sizeof(charPath));
    charPath[sizeof(charPath) - 1] = 0;

    //convert int into char
    char haliteString[10];
    sprintf(haliteString, "%d", halite);

    //convert int into char
    char shipString[10];
    sprintf(shipString, "%d", shipNumber);

    //convert int into char
    char dropoffString[10];
    sprintf(dropoffString, "%d", dropoff);

    //Create file and write informations issued from genetic algorithm on it
    std::ofstream f( charPath );
     if ( f ) {
        f << "playerId:" << str << endl
        << "haliteAmount:" << haliteString << endl
        << "shipCreated:" << shipString << endl
        << "dropoffCreated:" << dropoffString;
     }
}

bool GeneticAlgo::fileExists(const string & full_path)
{
    ifstream f(full_path.c_str());
    return f.good();
}

Parameters GeneticAlgo::getParamOfPlayer(const int & playerId)
{
    return listParam[playerId];
}

void GeneticAlgo::runGeneticAlgo(const int & id, const string & path)
{
    //Tous les résultats seront stockés dans ce vecteur de vecteur
    vector<vector<int>> allResults;
    //Toutes les parties avec leurs paramètres respectifs
    vector<vector<int>> allGamesWithParameters;

    string incremental_path = path;
    string result_path = path + "_result.txt";

    if(fileExists(incremental_path + "_result.txt")) {
        //On lit tous les fichiers results que l'on trouve jusqu'à ce qu'il n'y en ait plus
        while(fileExists(incremental_path + "_result.txt") && fileExists(incremental_path + ".txt")) {
            //On récupère toutes les informations du fichier résultat
            vector<int> entityResult = readFile(incremental_path + "_result.txt");
            allResults.push_back(entityResult);
            //On récupère les paramètres associés à ces résultats
            vector<int> entityParameters = readFile(incremental_path + ".txt");
            allGamesWithParameters.push_back(entityParameters);

            //On récupère le numéro du fichier, on le convertit en entier et on lui ajoute +1
            string currentNumberString = incremental_path.substr(incremental_path.find("_") + 1);
            int currentNumber = std::stoi( currentNumberString ) + 1;

            //On récupère la base du chemin passé en paramètre
            string basepath = incremental_path.substr(0, incremental_path.find("_"));

            //On convertit le nouveau numéro int en string
            string number = std::to_string(currentNumber);

            //On recompose un nouveau chemin à partir des nouveau éléments
            incremental_path = basepath + "_" + number;
        }
    } else {

        listParam[id].setMaxDistShipSee(8);
        listParam[id].setMaxHaliteShipMove(300);
        listParam[id].setMaxHaliteShipReturn(650);
        listParam[id].setNumberShipStopProduction(13);
        listParam[id].setTurnWhenBotStopSpawn(300);

        return;
    }

    int indexResultParent1 = selectionByTournament(allResults);
    int indexResultParent2 = selectionByTournament(allResults);

    //On récupère deux parties qui ont gagné la sélection par tournoi
    vector<int> parent1 = allGamesWithParameters[indexResultParent1];
    vector<int> parent2 = allGamesWithParameters[indexResultParent2];
    //On applique le mélange / crossover sur les deux parents
    vector<int> enfant = crossover(parent1, parent2);

    //On applique la fonction mutation
    vector<int> result = mutate(enfant);

    listParam[id].setMaxDistShipSee(result[0]);
    listParam[id].setMaxHaliteShipMove(result[1]);
    listParam[id].setMaxHaliteShipReturn(result[2]);
    listParam[id].setNumberShipStopProduction(result[3]);
    listParam[id].setTurnWhenBotStopSpawn(result[4]);

}

//Mélange deux parents ce qui donne un enfant
vector<int> GeneticAlgo::crossover(vector <int> parent1, vector<int> parent2)
{
    vector<int> enfant(5,-1);

    int startPos = rand() % parent1.size();
    int endPos = rand() % parent1.size();

    for(int i = 0 ; i < enfant.size() ; i++ ) {
        if(startPos < endPos && i < endPos) {
            enfant.at(i) = parent1[i];
        } else if (startPos > endPos) {
            if(i > startPos || i < endPos) {
                enfant.at(i) = parent1[i];
            }
        }
    }

    for(int i = 0 ; i < parent2.size() ; i++ ) {
        if(enfant[i] == -1) {
            enfant.at(i) = parent2[i];
        }
    }

    return enfant;
}

vector<int> GeneticAlgo::mutate(vector<int> & parameters)
{
    int mutationRate = 10;

    assert(parameters.size() == 5);

    //On fait le choix de muter tous les paramètres lorsqu'il y a mutation
    if(rand() % 100 > 100 - mutationRate) {

        //Va muter le paramètre en fonction de sa spécificité
        int randVal;

         // Paramètre : Vision du bateau
        randVal = rand() % 32 +1;
        parameters.at(0) = randVal;

        //Paramètre : halite qui décide le bateau à bouger / déposer sa cargaison
        randVal = rand() % 500;
        parameters.at(1) = randVal;

        randVal = 400+rand() % 520;
        parameters.at(2) = randVal;

        // nombre de bateau à produire
        randVal = rand() % 100 + 1;
        parameters.at(3) = randVal;

        // nombre de tours avant d'arrêter la prod de bateaux
        randVal = 200 + rand() % 120;
        parameters.at(4) = randVal;
    }
    else {
        //On laisse une légère mutation obligatoire pour chaque individu (qui ne touche qu'un seul paramètre)
        int parameterToMutate = rand() % parameters.size();
        int randVal;

        if(parameterToMutate == 0) {
            randVal = 12 + rand() % 8;
        }

        if(parameterToMutate == 1) {
            randVal = 200 + rand() % 200;
        }

        if( parameterToMutate == 2) {
            randVal = 400 + rand() % 520;
        }

        if(parameterToMutate == 3) {
            randVal = 10 + rand() % 40;
        }

        if(parameterToMutate == 4) {
            randVal = 250 + rand() % 80;
        }

        parameters.at(parameterToMutate) = randVal;
    }

    return parameters;
}

int GeneticAlgo::selectionByTournament(const vector<vector<int>> & allResults)
{
    int tournamentSize = allResults.size();
    int indexWinner=0;

    for(int i = 0 ; i < tournamentSize-1 ; i++ ) {
        // compare le nombre final d'halite et conserve l'index ayant le plus grand taux
        if(allResults[i][1] >allResults[i+1][1]) {
            indexWinner = i;
        } else {
            indexWinner = i+1;
        }
    }

    return indexWinner;
}

