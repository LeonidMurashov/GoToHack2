#include <iostream>
#include <fstream>
#include <string>
#include <assert.h>
#include <map>
#include <vector>
#include <algorithm>
#include <iterator>
#include <Windows.h>

using namespace std;

int FileSize(char* FileName)
{
	HANDLE hFile;
	DWORD lcFileSize;
	hFile = CreateFile(FileName, GENERIC_READ, 0, 0, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, 0);
	lcFileSize = GetFileSize(hFile, 0);
	CloseHandle(hFile);
	return lcFileSize;
}

vector<map<string, string>> events;
vector<map<string, string>> structure;
map<int, int> steps_cost;
char* eventsAdress = "D:\\Windows\\Desktop\\course-217-events.csv";
char* structureAdress = "D:\\Windows\\Desktop\\course-217-structure.csv";
map<int, vector<long long>> students; //id, [score, first_time, last_time]
int i2 = 0;

vector<string> ParseString(string str)
{
	vector<string> strings;
	string currStr = "";
	unsigned short strSize = str.size(), currStrSize = 0;
	for (int i = 0; i < strSize; i++)
	{
		if (str[i] != ',' && i != strSize)
		{
			currStr += str[i];
			currStrSize++;
		}
		else
		{
			strings.push_back(currStr);
			currStr = "";
		}
	}
	strings.push_back(currStr);
	currStr = "";
	return strings;
}

void FillArray(vector<map<string, string>> &myArray, char* fileAdress)
{
	myArray.clear();
	int fileSize = FileSize(fileAdress) / 31; // characters in line average count 
	ifstream file, file2(fileAdress);
	assert(file2.is_open());
	system("cls");
	cout << "Calculating size...";	

	file2.close();
	file.open(fileAdress);
	
	string str;

	file >> str;
	vector<string> titles = ParseString(str);
	while (!file.eof())
	{
		vector<string> data;
		file >> str;
		data = ParseString(str);

		/*delme*/
		// Check if event is about passing
		if (fileAdress == structureAdress)
			steps_cost[stoi(data[5])] = stoi((string)data[8]);

		map<string, string> toArray;
		for (int i = 0; i < titles.size(); i++)
			toArray[titles[i]] = data[i];
		myArray.push_back(toArray);
		i2++;
		if (i2 % 1000 == 0) { system("cls"); cout << "Reading complete: " + to_string(int(i2 / (double)fileSize * 100) + 1) + "%" << endl; }
	}
	file.close();
}

bool CompareTimeValues(vector<long long> a, vector<long long> b)
{
	return abs(a[1] - a[2]) < abs(b[1] - b [2]);
}

int main()
{
	vector<vector<long long>> passedStudents;
	// Fill in structure
	FillArray(structure, structureAdress);
	FillArray(events, eventsAdress);


	//string id = events[i]["user_id"];
	//auto student = students.find(id);
	//(*student).second += steps_cost[events[i]["step_id"]];

	cout << "Parsing passed students..." << endl;
	for (int i = events.size()-1; i >= 0 ; i--)
	{
		int user_id = stoi(events[i]["user_id"]),
			step_id = stoi(events[i]["step_id"]);

		auto student = students.find(user_id);
		if (student != students.end()) 
		{
			long long currEventTime = stoll(events[i]["time"]);
			if ((*student).second[2] < currEventTime)
				(*student).second[2] = currEventTime;

			if ((*student).second[1] > currEventTime)
				(*student).second[1] = currEventTime;

			if (events[i]["action"] == "passed")
			{
				(*student).second[0] += steps_cost[step_id];
				if ((*student).second[0] >= 24)
				{
					(*student).second[0] = -999999;
					passedStudents.push_back(vector<long long>{ (*student).first, (*student).second[1], (*student).second[2] });
				}
			}
		}
		else
		{
			vector<long long> info = { steps_cost[step_id], stoll(events[i]["time"]), stoll(events[i]["time"]) }; //id, [score, first_time, last_time]
			students[user_id] = info;
		}
	}

	cout << "Parsing finalists..." << endl;
	sort(passedStudents.begin(), passedStudents.end(), CompareTimeValues);


	for (int i = 0; i < min(passedStudents.size(), 10); i++)
	{
		cout << passedStudents[i][0];
		if (i != min(passedStudents.size(), 10) - 1)
			cout << ",";
	}
	cout << endl;
	if (eventsAdress != "D:\\Windows\\Desktop\\course-217-events.csv")
		cout << "WARNING! NOT AN ORIGINAL FILE USED!" << endl;

	/*for (int i = 1; i < passedStudentsId.size(); i++)
	{
		int a = passedStudentsId[0], b = passedStudentsId[i];
		cout  << (abs(students[a][2] - students[a][1]) > abs(students[b][2] - students[b][1])) << " ";
	}*/
	
	system("pause");
	return 0;
}	

// Finding top-10
	/*ifstream eventsfile(eventsAdress);
	assert(eventsfile.is_open());
	string str;

	eventsfile >> str;
	vector<string> titles = ParseString(str);
	while (!eventsfile.eof())
	{
	vector<string> data;
	eventsfile >> str;
	data = ParseString(str);

	map<string, string> toArray;
	for (int i = 0; i < titles.size(); i++)
	toArray[titles[i]] = data[i];

	//Addition
	toArray.find("")

	}*/

/*	for (int i = events.size() - 1; i >= 0 && studentsPassed.size() <= 10; i--)
	{
		string id = events[i]["user_id"];
		auto student = students.find(id);
		if (student != students.end())
		{
			(*student).second += steps_cost[events[i]["step_id"]];
		}
		else
		{
			students[id] = steps_cost[events[i]["step_id"]];
		}

		//passing code
		if (students[id] >= 24)
		{
			studentsPassed.push_back(id);
			students[id] = -LONG_MAX;
		}
	}

	if (studentsPassed.size() != 0)
	{
		for (int i = 0; i < studentsPassed.size() - 1; i++)
			cout << (string)(studentsPassed[i]) + ",";
		cout << studentsPassed[studentsPassed.size() - 1] << endl;
	}
	else 
	{
		cout << "No students passed" << endl;
	}*/
