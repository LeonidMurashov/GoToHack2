#include <iostream>
#include <fstream>
#include <string>
#include <assert.h>
#include <map>
#include <vector>
#include <algorithm>
#include <iterator>
#include <Windows.h>
#include <cstdlib>
#include"Extra.hpp"
using namespace std;

struct Step
{
	map<string, string> fields;
	long long id;
	long long returned = 0, visited = 0;
	double returnability = 0;

	Step(map<string, string> _fields)
	{
		fields = _fields;
		id = stoll(fields["step_id"]);
	}

	double CalculateReturnabily()
	{
		if(visited != 0)
			returnability = returned / (double)visited;
		return returnability;
	}
};

vector<Step*> structure;
Step* FindStepByID(long long _id)
{
	for (int i = 0; i < structure.size(); i++)
		if (structure[i]->id == _id)
			return structure[i];
	return nullptr;
}

map<long long, int> StepAbsPosition; // <step_id, absPosition>
struct Student
{
	long long id;
	vector<pair<long long, vector<long long>>> visitingStats; // <stepID, <times>>
	Student(long long _id)
	{
		id = _id;
	}

	static bool StatsComparator(pair<long long, vector<long long>> a, pair<long long, vector<long long>> b)
	{
		return StepAbsPosition[a.first] < StepAbsPosition[b.first];
	}

	void SortStats()
	{
		// Sort times
		for(auto& item : visitingStats)
			sort(item.second.begin(), item.second.end());

		// Sort steps by order
		sort(visitingStats.begin(), visitingStats.end(), StatsComparator);
	}

	void PushBackInVisitingStats(long long stepID, long long time)
	{
		for(int i = 0; i < visitingStats.size(); i++)
			if (visitingStats[i].first == stepID)
			{
				visitingStats[i].second.push_back(time);
				return;
			}

		// If not exists yet
		visitingStats.push_back(make_pair(stepID, vector<long long> { time }));
	}

	void Analize()
	{
		for (int i = 0; i < visitingStats.size(); i++)
		{
			Step *step = FindStepByID(visitingStats[i].first);
			assert(step);
			step->visited++;
			if (i != visitingStats.size() - 1)
			{
				if (StepAbsPosition[visitingStats[i + 1].first] - StepAbsPosition[step->id] == 1)
				{
					long long minTime = visitingStats[i].second[0],
						maxTime = visitingStats[i].second[visitingStats[i].second.size() - 1];
					for (int j = 0; j < visitingStats[i + 1].second.size(); j++)
					{
						long long nextVisitTime = visitingStats[i + 1].second[j];
						if (minTime < nextVisitTime && nextVisitTime < maxTime)
						{
							step->returned++;
							break;
						}
					}
				}
			}
		}
	}
};

vector<map<string, string>> eventsRaw;
vector<map<string, string>> structureRaw;
vector<Student*> students;
char* eventsAdress = "D:\\Windows\\Desktop\\course-217-events.csv";
char* structureAdress = "D:\\Windows\\Desktop\\course-217-structure.csv";
int i2 = 0;

// Read and remember Raw data
void FillArray(vector<map<string, string>> &myArray, char* fileAdress)
{
	myArray.clear();
	int fileSize = FileSize(fileAdress) / 31; // characters in line average count 
	ifstream file;
	system("cls");
	cout << "Reading files: 0%";
	file.open(fileAdress);
	assert(file.is_open());
	
	string str, prevStr;

	file >> str;
	vector<string> titles = split(str);
	short  titlesSize = titles.size();
	while (!file.eof())
	{
		vector<string> data;
		prevStr = str;
		file >> str;

		data = split(str);

		map<string, string> toArray;
		for (int i = 0; i < titlesSize; i++)
			toArray[titles[i]] = data[i];
		myArray.push_back(toArray);
		i2++;
		if (i2 % 5000 == 0) { SetCursorPosistion(15, 0); cout << to_string(int(i2 / (double)fileSize * 100) + 1) + "%"; }		
	}
	file.close();
	i2 = 0;
	SetCursorPosistion(0, 0);
	cout << "Reading files: 100%";
}

// Needed in ordering steps
bool CompareStepsPosition(Step* a, Step* b)
{
	long long mPA = stoll(a->fields["module_position"]), mPB = stoll(b->fields["module_position"]),
		lPA = stoll(a->fields["lesson_position"]), lPB = stoll(b->fields["lesson_position"]),
		sPA = stoll(a->fields["step_position"]), sPB = stoll(b->fields["step_position"]);
	if (mPA != mPB)
		return mPA < mPB;
	else if (lPA != lPB)
		return lPA < lPB;
	else
		return sPA < sPB;
}

// Needed for getting final result
bool CompareStepsReturnability(Step* a, Step* b)
{
	return a->returnability > b->returnability;
}

Student* FindStudentByID(long long _id)
{
	int N = students.size();
	for (int i = 0; i < N; i++)
		if (students[i]->id == _id)
			return students[i];
	return nullptr;
}

int main()
{
	// Fill in raws
	FillArray(structureRaw, structureAdress);
	FillArray(eventsRaw, eventsAdress);

	// Ordering steps and create fast access to steps' order with StepAbsPosition array
	cout << endl << "Preparing data: 0%";
	SetCursorPosistion(15, 2);
	for (int i = 0; i < structureRaw.size(); i++)
		structure.push_back(new Step(structureRaw[i]));
	sort(structure.begin(), structure.end(), CompareStepsPosition);
	for (int i = 0; i < structure.size(); i++)
		StepAbsPosition[stoll(structure[i]->fields["step_id"])] = i;	
	
	// Writting in visited by students events
	long long N = eventsRaw.size();
	for (int i = 0; i < N; i++)
	{
		long long userID = stoll(eventsRaw[i]["user_id"]),
			stepID = stoll(eventsRaw[i]["step_id"]),
			eventTime = stoll(eventsRaw[i]["time"]);
		Student* student = FindStudentByID(userID);
		if (student)
		{
			student->PushBackInVisitingStats(stepID, eventTime);
		}
		else
		{
			student = new Student(userID);
			student->visitingStats.push_back(make_pair(stepID, vector<long long> { eventTime }));
			students.push_back(student);
		}

		if (i2 % 1000 == 0)
		{
			SetCursorPosistion(16, 1);
			cout << to_string(int(i2 / (double)N * 100)) + "%";
		}
		i2++;
	}
	SetCursorPosistion(0, 1);
	cout << "Preparing data: 100%";

	// Sort visitedStats in students to analize them in next step
	cout << endl << "Sorting stats: 0%";
	N = students.size();
	for (int i = 0; i < N; i++)
	{
		students[i]->SortStats();
		if (i % 100 == 0)
		{
			SetCursorPosistion(15, 2);
			cout << to_string(int((i / (double)N * 100))) + "%";
		}
	}
	SetCursorPosistion(0, 2);
	cout << "Sorting stats: 100%";

	// Calculating returns and visits of steps
	cout << endl << "Analizing data: 0%";
	N = students.size();
	for (int i = 0; i < N; i++)
	{
		students[i]->Analize();
		if (i % 100 == 0)
		{
			SetCursorPosistion(16, 3);
			cout << to_string(int((i / (double)N * 100))) + "%";
		}
	}
	SetCursorPosistion(0, 3);
	cout << "Analizing data: 100%";

	// Calculating result
	for (int i = 0; i < structure.size(); i++)
		structure[i]->CalculateReturnabily();
	sort(structure.begin(), structure.end(), CompareStepsReturnability);

	cout << endl;
	for (int i = 0; i < min(10, structure.size()); i++)
	{
		cout << structure[i]->id;
		if (i != min(10, structure.size())-1)
			cout << ",";
	}
	// Check if original file used
	cout << endl;
	if (eventsAdress != "D:\\Windows\\Desktop\\course-217-events.csv")
		cout << "-----------WARNING! NOT AN ORIGINAL FILE ADRESS USED!-----------" << endl;
	
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



/*	cout << "Parsing passed students..." << endl;
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
		cout << "-----------WARNING! NOT AN ORIGINAL FILE ADRESS USED!-----------" << endl;

	/*for (int i = 1; i < passedStudentsId.size(); i++)
	{
		int a = passedStudentsId[0], b = passedStudentsId[i];
		cout  << (abs(students[a][2] - students[a][1]) > abs(students[b][2] - students[b][1])) << " ";
		}*/