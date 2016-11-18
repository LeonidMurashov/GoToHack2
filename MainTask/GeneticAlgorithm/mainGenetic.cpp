
/*
*	works too bad	
*/

#include <iostream>
#include <map>
#include <fstream>
#include "Extra.hpp"
#include "Creature.h"
#include "GeneticEngine.h"

char* featuresFileAdress = "D:\\Windows\\Desktop\\features.csv";
char* networkFileAdress = "D:\\Windows\\Desktop\\NeuralNetwork.xml";
vector<map<string, double>> features;
short featuresCount;
double F_measureRecord = 0;
NeuralNetwork *mainNetwork;

int i2 = 0, line = 0;
// Read and remember Raw data
void FillArray(vector<map<string, double>> &myArray, char* fileAdress)
{
	myArray.clear();
	int fileSize = FileSize(fileAdress) / 42; // characters in line average count 
	ifstream file;
	cout << "Reading files: 0%";
	file.open(fileAdress);
	assert(file.is_open());

	string str, prevStr;

	file >> str;
	vector<string> titles = split(str);
	featuresCount = titles.size() - 2;
	short  titlesSize = titles.size();
	while (!file.eof())
	{
		vector<string> data;
		prevStr = str;
		file >> str;

		data = split(str);

		map<string, double> toArray;
		for (int i = 0; i < titlesSize; i++)
			toArray[titles[i]] = stod(data[i]);
		myArray.push_back(toArray);
		i2++;
		if (i2 % 500 == 0) { SetCursorPosistion(15, line); cout << to_string(int(i2 / (double)fileSize * 100) + 1) + "%"; }
	}
	file.close();
	i2 = 0;
	SetCursorPosistion(0, line);
	cout << "Reading files: 100%" << endl;
	line++;
}

double TestNetwork(NeuralNetwork* Network)
{
	// Run and calculate accuracy
	int TP = 0, TN = 0, FP = 0, FN = 0;
	double error = 0;
	for (int i = 0; i < 5000; i++)
	{
		vector<double> input, output;
		for (auto in : features[i])
		{
			if (in.first != "passed" && in.first != "id")
				input.push_back(in.second);
		}

		Network->Run(input, output);

		bool answer = output[0] > 0.5,
			rightAnswer = features[i]["passed"];
		error += rightAnswer - output[0];
		if (answer)
		{
			if (rightAnswer) TP++;
			else FP++;
		}
		else
		{
			if (rightAnswer) FN++;
			else TN++;
		}
	}

	double precision = (TP + FP) != 0 ? (TP / (double)(TP + FP)) : 0;
	double recall = (TP + FN) != 0 ? (TP / (double)(TP + FN)) : 0;
	double F_measure = (precision + recall != 0) ? 2 * ((precision*recall) / (precision + recall)) : 0;
	return F_measure;
}


void MainTestNetwork(int testsCount)
{
	// Run and calculate accuracy
	int TP = 0, TN = 0, FP = 0, FN = 0;
	double error = 0;
	for (int i = 0; i < testsCount; i++)
	{
		vector<double> input, output;
		for (auto in : features[i])
		{
			if (in.first != "passed" && in.first != "id")
				input.push_back(in.second);
		}

		mainNetwork->Run(input, output);

		bool answer = output[0] > 0.5,
			rightAnswer = features[i]["passed"];
		error += rightAnswer - output[0];
		if (answer)
		{
			if (rightAnswer) TP++;
			else FP++;
		}
		else
		{
			if (rightAnswer) FN++;
			else TN++;
		}
	}

	cout << endl;
	cout << endl;
	double precision = (TP + FP) != 0 ? (TP / (double)(TP + FP)) : 0;
	double recall = (TP + FN) != 0 ? (TP / (double)(TP + FN)) : 0;
	double F_measure = (precision + recall != 0) ? 2 * ((precision*recall) / (precision + recall)) : 0;
	cout << "Precision: " << precision << "," << " Recall: " << recall << endl;
	cout << "F_measure: " << F_measure << "," << " Error: " << error / (double)testsCount << endl;
	cout << endl;
	line += 4;

	if (F_measureRecord < F_measure)
	{
		F_measureRecord = F_measure;
		mainNetwork->Name = "F_measureRecord:" + to_string(F_measure);
		mainNetwork->SaveToFile(networkFileAdress);
	}
	if (F_measureRecord < F_measure)
	{
		F_measureRecord = F_measure;
		mainNetwork->Name = "F_measureRecord:" + to_string(F_measure);
		mainNetwork->SaveToFile(networkFileAdress);
	}
}

int main()
{
	FillArray(features, featuresFileAdress);
	vector<int> sizes = { featuresCount, 15, 1 };
	//int i = 0;
	vector<TransferFunction> tFuncs = { TransferFunction::None,
		TransferFunction::Sigmoid,
		TransferFunction::Sigmoid
	};
	GeneticEngine *engine = new GeneticEngine(TestNetwork, NetworkProperty(sizes, tFuncs));
	int iteration = -1;

	while (true)
	{
		iteration++;
		Creature *creature = engine->GeneticCircle(line);		
		if (creature)
		{
			cout << "Iteration: " << iteration << " F_measure: " << creature->GetFitness() << endl;
			if (F_measureRecord < creature->GetFitness())
			{
				F_measureRecord = creature->GetFitness();
				creature->GetNetwork()->Name = "F_measureRecord:" + to_string(creature->GetFitness()) + " Iteration: " + to_string(iteration);
				creature->GetNetwork()->SaveToFile(networkFileAdress);
			}
		}
		else
			cout << "Iteration: " << iteration << endl;

	}
}