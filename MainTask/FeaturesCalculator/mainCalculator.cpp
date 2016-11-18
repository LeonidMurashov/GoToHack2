/*
*	Needed for merging two files of Raw Features
*/

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <assert.h>
#include <algorithm>
#include "Extra.hpp"
using namespace std;

char* featuresRaw1FileAdress = "D:\\Windows\\Desktop\\featuresRaw_test.csv";
char* featuresRaw2FileAdress = "D:\\Windows\\Desktop\\featuresRaw_test2.csv";
char* outputFileAdress = "D:\\Windows\\Desktop\\features_test.csv";
double maxAveragePassingTime = 0, maxAverageIdlingTime = 0, maxScore = 0;
vector<map<string, double>> featuresRaw1, featuresRaw2;
int i2 = 0;

struct Student
{
	int id;
	bool passed;
	map<string, double> features;
	map<string, double> featuresNormalized;

	Student(int _id)
	{
		id = _id;
	}

	const char* titles = "id,PassedStepsPart,PassingPercentage,AveragePassingTime,AverageIdlingTime,passed";
	string GetDataForSave()
	{
		string str = "";
		str += to_string(id) + ",";
		str += to_string(featuresNormalized["PassedStepsPart"]) + ",";
		str += to_string(featuresNormalized["PassingPercentage"]) + ",";
		str += to_string(featuresNormalized["AveragePassingTime"]) + ",";
		str += to_string(featuresNormalized["AverageIdlingTime"]) + ",";
		str += to_string(passed);
		return str;
	}
};

vector<Student*> students;

// Normalize non-normalized features(AveragePassingTime and AverageIdlingTime)
void NormalizeFeatures()
{	
	for (int i = 0; i < students.size(); i++)
	{
		students[i]->featuresNormalized["PassedStepsPart"] = students[i]->features["PassedStepsPart"];
		students[i]->featuresNormalized["PassingPercentage"] = students[i]->features["PassingPercentage"];

		double feature_AveragePassingTime = students[i]->features["AveragePassingTime"],
			feature_AverageIdlingTime = students[i]->features["AverageIdlingTime"];

		if (feature_AveragePassingTime != -1)
			students[i]->featuresNormalized["AveragePassingTime"] = feature_AveragePassingTime / (double)maxAveragePassingTime;
		else
			students[i]->featuresNormalized["AveragePassingTime"] = 1;

		if (feature_AverageIdlingTime != -1)
			students[i]->featuresNormalized["AverageIdlingTime"] = feature_AverageIdlingTime / (double)maxAverageIdlingTime;
		else
			students[i]->featuresNormalized["AverageIdlingTime"] = 1;
	}
}

// Read and remember RawFeatures data
void FillArray(vector<map<string, double>> &myArray, char* fileAdress)
{
	myArray.clear();
	int fileSize = FileSize(fileAdress) / 31; // characters in line average count 
	ifstream file;
	std::system("cls");
	cout << "Reading files: 0%";
	file.open(fileAdress);
	assert(file.is_open());

	string str, prevStr;

	file >> str;
	vector<string> titles = split(str);
	short titlesSize = titles.size();
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
		if (i2 % 800 == 0) { SetCursorPosistion(15, 0); cout << to_string(int(i2 / (double)fileSize * 100) + 1) + "%"; }
	}
	file.close();
	i2 = 0;
	SetCursorPosistion(0, 0);
	cout << "Reading files: 100%";
}

Student* FindStudentByID(int _id)
{
	int N = students.size();
	for (int i = 0; i < N; i++)
		if (students[i]->id == _id)
			return students[i];
	return nullptr;
}

int main()
{
	FillArray(featuresRaw1, featuresRaw1FileAdress);
	FillArray(featuresRaw2, featuresRaw2FileAdress);

	// For better output
	maxAveragePassingTime /= 100;

	cout << endl << "Preparing data: 0%";
	// Read in data from featuresRaw1
	int N = featuresRaw1.size();
	for (int i = 0; i < N; i++)
	{
		Student *student = new Student(featuresRaw1[i]["id"]);
		student->features["PassedStepsPart"] = featuresRaw1[i]["PassedStepsPart"];
		student->features["AveragePassingTime"] = featuresRaw1[i]["AveragePassingTime"];
		student->features["AverageIdlingTime"] = featuresRaw1[i]["AverageIdlingTime"];
		student->passed = featuresRaw1[i]["passed"];
		students.push_back(student);

		if (i2 % 100 == 0)
		{
			SetCursorPosistion(16, 1);
			cout << to_string(int(i2 / (double)N * 100)) + "%";
		}
		i2++;
	}
	SetCursorPosistion(0, 1);
	cout << "Preparing data: 100%";
	i2 = 0;

	// Merge with data from featuresRaw2
	cout << endl <<"Merging data: 0%";
	N = featuresRaw2.size();
	for (int i = 0; i < N; i++)
	{
		int userID = featuresRaw2[i]["id"];
		Student* student = FindStudentByID(userID);
		if (student)
		{
			// Merging
			student->features["PassedStepsPart"] = student->features["PassedStepsPart"] + featuresRaw2[i]["PassedStepsPart"];
			student->features["PassingPercentage"] = (student->features["PassingPercentage"] + featuresRaw2[i]["PassingPercentage"]) / 2;
			
				 if(min(student->features["AveragePassingTime"], featuresRaw2[i]["AveragePassingTime"]) != -1) student->features["AveragePassingTime"] = (student->features["AveragePassingTime"] + featuresRaw2[i]["AveragePassingTime"]) / 2;
			else student->features["AveragePassingTime"] = max(student->features["AveragePassingTime"], featuresRaw2[i]["AveragePassingTime"]);
			
				 if (min(student->features["AverageIdlingTime"], featuresRaw2[i]["AverageIdlingTime"]) != -1) student->features["AverageIdlingTime"] = (student->features["AverageIdlingTime"] + featuresRaw2[i]["AverageIdlingTime"]) / 2;
			else student->features["AverageIdlingTime"] = max(student->features["AverageIdlingTime"], featuresRaw2[i]["AverageIdlingTime"]);
		}
		else
		{
			// Just push-backing
			student = new Student(featuresRaw2[i]["id"]);
			student->features["PassedStepsPart"] = featuresRaw2[i]["PassedStepsPart"];
			student->features["AveragePassingTime"] = featuresRaw2[i]["AveragePassingTime"];
			student->features["AverageIdlingTime"] = featuresRaw2[i]["AverageIdlingTime"];
			student->passed = featuresRaw2[i]["passed"];
			students.push_back(student);
		}
		
		if (i2 % 100 == 0)
		{
			SetCursorPosistion(14, 2);
			cout << to_string(int(i2 / (double)N * 100)) + "%";
		}
		i2++;
	}
	SetCursorPosistion(0, 2);
	cout << "Merging data: 100%";

	// Normalizing data
	NormalizeFeatures();

	// Sort students
	sort(students.begin(), students.end(), StudentsIDComparator);

	// Saving results to file
	ofstream fout(outputFileAdress);
	fout << students[0]->titles << endl;
	for (int i = 0; i < students.size(); i++)
		fout << students[i]->GetDataForSave() << endl;
	fout.close();

	cout << endl;
	std::system("pause");
	return 0;
}