#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <assert.h>
#include "Extra.hpp"
#include "NeuralNetwork.h"
using namespace std;
using namespace NeuralNetworkLibrary;

char* featuresFileAdress = "D:\\Windows\\Desktop\\features_test.csv";
char* networkFileAdress = "D:\\Windows\\Desktop\\NeuralNetwork_test.xml";
char* outputFileAdress = "D:\\Windows\\Desktop\\NetworkPrecision.csv";
vector<map<string, double>> features;
short featuresCount;

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
		if (str == prevStr)
			continue;

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

/*vector<double> GenerateInput(int i)
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

int main()
{
	srand(time(NULL));
	FillArray(features, featuresFileAdress);

	NeuralNetwork *Network = NeuralNetwork::LoadFromFile(networkFileAdress);
	cout << Network->Name << " loaded." << endl;
	fstream fout(outputFileAdress);
	fout << "user_id,passed" << endl;
	for (int i = 0; i < features.size(); i++)
	{
		vector<double> input, output;
		for (auto in : features[i])
		{
			if (in.first != "passed" && in.first != "id")
				input.push_back(in.second);
		}

		Network->Run(input, output);
		bool answer = output[0] > 0.5;
		fout << features[i]["id"] << "," << answer << endl;
	}
	fout.close();

	system("pause");
	return 0;
}