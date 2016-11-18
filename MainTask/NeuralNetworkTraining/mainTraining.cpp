#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <assert.h>
#include "Extra.hpp"
#include "NeuralNetwork.h"
using namespace NeuralNetworkLibrary;
using namespace std;

char* featuresFileAdress = "D:\\Windows\\Desktop\\features.csv";
char* networkFileAdress = "D:\\Windows\\Desktop\\NeuralNetwork.xml";
vector<map<string, double>> features;
short featuresCount;
double F_measureRecord = 0;
NeuralNetwork *Network;

int i2 = 0, line = 0;
// Read and remember Raw data
void FillArray(vector<map<string, double>> &myArray, char* fileAdress)
{
	myArray.clear();
	int fileSize = FileSize(fileAdress) / 140; // characters in line average count 
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
/*
vector<double> GenerateInput(int i)
{
	vector<double> input;
	input.push_back(features[i]["PassedStepsPart"]);
	input.push_back(features[i]["CostablePercentage"]);
	input.push_back(features[i]["Score"]);
	input.push_back(features[i]["AveragePassingTime"]);
	input.push_back(features[i]["AverageIdlingTime"]);
	input.push_back(features[i]["SkippedPersentage"]);
	input.push_back(features[i]["TimeDeviation"]);
	input.push_back(features[i]["ScoreDeviation"]);
	return input;
}*/

void TestNetwork(int testsCount)
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
		Network->Name = "F_measureRecord:" + to_string(F_measure);
		Network->SaveToFile(networkFileAdress);
	}
	if (F_measureRecord < F_measure)
	{
		F_measureRecord = F_measure;
		Network->Name = "F_measureRecord:" + to_string(F_measure);
		Network->SaveToFile(networkFileAdress);
	}
}

int main()
{
	srand(time(NULL));
	FillArray(features, featuresFileAdress);

	vector<int> sizes = { (int)features[0].size() - 2, 15, 1 };
	//int i = 0;
	vector<TransferFunction> tFuncs = { TransferFunction::None,
		TransferFunction::Sigmoid,
		TransferFunction::Sigmoid
	};
	Network = new NeuralNetwork(sizes, tFuncs);

	int goodCount = 0;
	double countRatio = 0;
	for (auto feature : features)
		if (feature["passed"])
			goodCount++;
	countRatio = (double)(features.size() - goodCount) / goodCount;

	cout << "Training network: 0%";
	int N = features.size();
	while(F_measureRecord < 0.9)
	{
		for (int i = 1500; i < N; i++)
		{
			vector<double> input, desired;
			for (auto in : features[i])
			{
				if (in.first != "passed" && in.first != "id")
					input.push_back(in.second);
			}

			desired.push_back(features[i]["passed"]);
			if(features[i]["passed"])
				Network->Train(input, desired, 0.005*countRatio, 0.005*countRatio);
			else
				Network->Train(input, desired, 0.005, 0.005);

			if (i2 % 500 == 0)
			{
				SetCursorPosistion(18, line);
				cout << to_string(int(i2 / (double)(N * 30) * 100)) + "%";
				
				//if (F_measureRecord > 0.40)
				//	TestNetwork(1500);
			}
			i2++;

		}
		TestNetwork(1500);
		cout << "Training network: 0%";
		line++;
	}
	SetCursorPosistion(0, line);
	i2 = 0;
	line++;
	cout << "Training network: 100%";
	//cout << endl << "Network trained." << endl;

	TestNetwork(features.size());

	system("pause");
	return 0;
}


/*
double NetworkTest(double rate, double momentum)
{
vector<int> sizes = { featuresCount, 6, 1 };
//int i = 0;
vector<TransferFunction> tFuncs = { TransferFunction::None,
TransferFunction::Gaussian,
TransferFunction::Sigmoid
};
NeuralNetwork *Network = new NeuralNetwork(sizes, tFuncs);

cout << "Training network: 0%";
int N = features.size();
for (int j = 0; j < 10; j++)
for (int i = 0; i < N; i++)
{
vector<double> input = GenerateInput(i), desired;
desired.push_back(features[i]["passed"]);

Network->Train(input, desired, rate, momentum);

if (i2 % 500 == 0)
{
SetCursorPosistion(18, line);
cout << to_string(int(i2 / (double)(N * 10) * 100)) + "%";
}
i2++;

}
SetCursorPosistion(0, line);
i2 = 0;
line++;
cout << "Training network: 100%";
//cout << endl << "Network trained." << endl;

// Run and calculate accuracy
int TP = 0, TN = 0, FP = 0, FN = 0;
for (int i = 0; i < N; i++)
{
vector<double> input = GenerateInput(i), output;
Network->Run(input, output);

bool answer = output[0] > 0.5,
rightAnswer = features[i]["passed"];

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
cout << "F_measure: " << F_measure << endl;
cout << endl;
line += 4;

if (F_measureRecord < F_measure)
{
F_measureRecord = F_measure;
Network->Name = "F_measureRecord:" + to_string(F_measure);
Network->SaveToFile(networkFileAdress);
}
return F_measure;
}*/