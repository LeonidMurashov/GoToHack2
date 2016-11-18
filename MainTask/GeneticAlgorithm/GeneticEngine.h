#pragma once
#include <vector>
#include <iostream>
#include "Creature.h"
#include "Extra.hpp"

using namespace NeuralNetworkLibrary;
using namespace std;

class GeneticEngine
{
	// Run network and return F_measure
	typedef double(*TestNetwork)(NeuralNetwork* _Network);
	TestNetwork testFunction;

	// Keeping network initializing parameters
	NetworkProperty networkProperty;
	Creature* bestOne;
public:

	GeneticEngine(TestNetwork _testFunction, NetworkProperty prop);

	int Population_Size = 100;// One generation size
	int Roulett_Size = 100000;	// Count of segments of Roulett, should be much grater than creatures.size()
									///const int Population_Size = 400;// First generation size
	int line;

	// Main circle called every iteration
	Creature* GeneticCircle(int _line);

	// Create start generation
	void CreateFirstGeneration();

	// Spawns current generation
	void Run();

	// Remove previous generation
	void Remove();

	// Run "roulett" randomizer - return a half of creatures (Selection Function)
	vector<Creature*> RunRoulett();

	// Crossing Function
	void Selection(vector<Creature*> halfFinal);

	// Merge Networks data
	Creature* BreedingFunction(Creature* creature1, Creature* creature2);

	// Merge function, which allow to control merging networks process 
	static double MergeFunction(double a, double b);	// Merge weights and biases

	vector<Creature*> creatures;	
	vector<Creature*> oldCreatures;

	int iteration;

};
