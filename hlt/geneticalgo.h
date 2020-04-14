#ifndef GENETICALGO_H
#define GENETICALGO_H
//BILLAUD Pierre / COUTURIEUX Victor STMN P5
#include <fstream>
#include <string>
#include <cstring>
#include <algorithm>
#include <iostream>
#include <vector>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <cassert>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
//Fichiers créés
#include "parameters.h"

using namespace std;

// Algorithme génétique qui s'inspire des parties déjà jouées
// et qui ajuste 5 paramètres en fonction des résultats de celles-ci
// (les 5 paramètres recommandés à faire varier sont définis dans la documentation)
// https://2018.halite.io/learn-programming-challenge/tutorials/ml-ga
class GeneticAlgo
{
public:

    GeneticAlgo();

    void initializePlayers(const int & countPlayers, const int & id);
    Parameters getParamOfPlayer(const int & playerId);
    void writeResults(const int & id, const int & halite, const int & shipNumber, const int & dropoff);

private:

    //Fonctions de lecture/écriture de fichiers
    string filepath(const string& path, const int & id);
    vector<int> readFile(const string& path);
    void writeFile(const string& path, const int & id);
    bool fileExists(const string& path);

    //Fonctions de l'algo génétique
    void runGeneticAlgo(const int & id, const string & path);
    vector<int> crossover(vector <int> parent1, vector<int> parent2);
    vector<int> mutate(vector<int> & parameters);
    int selectionByTournament(const vector<vector<int>> & allResults);

    int countPlayers_;
    Parameters listParam [4];
};

#endif // GENETICALGO_H
