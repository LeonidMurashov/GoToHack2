#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <assert.h>
#include"Extra.hpp"
using namespace std;

vector<map<string, string>> eventsRaw;
vector<map<string, string>> structureRaw;
vector<map<string, string>> targetsRaw;
char* eventsAdress1 = "D:\\Windows\\Desktop\\user_activity_test1.csv";
char* eventsAdress2 = "D:\\Windows\\Desktop\\user_activity_test2.csv";
char* structureAdress = "D:\\Windows\\Desktop\\structure.csv";
char* targetsAderss = "D:\\Windows\\Desktop\\targets.csv";
char* outputFileAdress = "D:\\Windows\\Desktop\\features_test.csv";
int line = 0; // Console line

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
		if (visited != 0)
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
	bool passed = false;
	vector<pair<long long, vector<long long>>> visitingStats; // <stepID, <times>>
	map<int, long long> stepPassed; //<stepID, time>
	map<int, long long> stepStartedAttempt;
	map<string, double> features;
	map<string, double> featuresNormalized;
	vector<pair<long long, int>> times; // <time, stepID>
	vector<vector<pair<long long, int>>> days;// <time, stepID>
	long long minTime = 0, maxTime = 0, middleTime = 0;

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
		for (auto& item : visitingStats)
			sort(item.second.begin(), item.second.end());

		// Sort steps by order
		sort(visitingStats.begin(), visitingStats.end(), StatsComparator);
	}

	void PushBackInVisitingStats(int stepID, long long time, string eventAction)
	{
		if (eventAction == "passed")
			stepPassed[stepID] = time;
		else if (eventAction == "started_attempt")
			stepStartedAttempt[stepID] = time;

		for (int i = 0; i < visitingStats.size(); i++)
			if (visitingStats[i].first == stepID)
			{
				visitingStats[i].second.push_back(time);
				return;
			}

		// If not exists yet
		visitingStats.push_back(make_pair(stepID, vector<long long> { time }));
	}

	// Calculate min, max and middle times
	void CalculateTimes()
	{
		minTime = INT_MAX;
		maxTime = 0;
		middleTime = 0;
		for (int i = 0; i < visitingStats.size(); i++)
		{
			if (minTime > visitingStats[i].second[0])
				minTime = visitingStats[i].second[0];
			if (maxTime < visitingStats[i].second[visitingStats[i].second.size() - 1])
				maxTime = visitingStats[i].second[visitingStats[i].second.size() - 1];
		}
		middleTime = (maxTime + minTime) / (double)2;

		for (auto stat : visitingStats)
		{
			for (auto time : stat.second)
			{
				times.push_back(make_pair(time, stat.first));
			}
		}
		
		sort(times.begin(), times.end());
		// Remove duplicates
		for (int i = 0; i < times.size() - 1; i++)
		{
			if (times[i] == times[i + 1])
			{
				times.erase(times.begin() + i + 1);
				i--;
			}
		}

		// Finding day beginning for student
		const int UNIX_DAY = 86400, UNIX_7HOURS = 25200, UNIX_HOUR = 3600;
		int bias = 0, minTimesCount = INT_MAX;
		for (int i = 0; i < UNIX_DAY - UNIX_7HOURS; i += UNIX_HOUR)
		{
			int timesCount = 0;
			for (auto time : times)
			{
				for(int j = 0; minTime + i + j*UNIX_DAY < maxTime;j++)
				{
					if (minTime + i + j*UNIX_DAY < time.first && time.first > minTime + i + j*UNIX_DAY + UNIX_7HOURS)
						timesCount++;
				}
			}

			if (minTimesCount > timesCount)
			{
				minTimesCount = timesCount;
				bias = i;
				if(minTimesCount == 0)
					break;
			}
		}

		for (int i = minTime - bias; i < maxTime; i += UNIX_DAY)
		{
			vector<pair<long long, int>> day;
			for (auto time : times)
			{
				if (i <= time.first && time.first < i + UNIX_DAY)
					day.push_back(time);
			}
			days.push_back(day);
		}
	}

	/* Features calculating functions */

	// Does not work
/*	double GetPassingPercentage() // For costable
	{
		int count = 0, // Count of costable steps
			countPassed = 0; // Count of costable steps passed
		for (int i = 0; i < visitingStats.size(); i++)
		{
			int stepCost = stoi(FindStepByID(visitingStats[i].first)->fields["step_cost"]);
			if (stepCost > 0)
			{
				count++;
				if (stepPassed[visitingStats[i].first] != 0)
					countPassed++;
			}
		}
		if (count == 0)
			return 0;
		return countPassed / (double)count;
	}*/

	double GetCostablePercentage() // For passed
	{
		int count = 0, // Count of costable steps
			countCostable = 0; // Count of costable steps passed

		for (auto pass : stepPassed)
		{
			count++; 
			int stepCost = stoi(FindStepByID(pass.first)->fields["step_cost"]);
			if (stepCost > 0)
				countCostable++;
		}

		if (count == 0)
			return 0;
		return countCostable / (double)count;
	}

	int GetScore()
	{
		int sum = 0;
		for (auto pass : stepPassed)
		{
			int stepCost = stoi(FindStepByID(pass.first)->fields["step_cost"]);
			sum += stepCost;
		}
		return sum;
	}

	// Viewed but not started
	// Does not work
	/*double GetSkippedPercentage()
	{
		int count = 0, // Count of costable steps
			countNotStarted = 0; // Count of costable steps passed
		for (int i = 0; i < visitingStats.size(); i++)
		{
			int stepCost = stoi(FindStepByID(visitingStats[i].first)->fields["step_cost"]);
			if (stepCost > 0)
			{
				count++;
				if (stepStartedAttempt[visitingStats[i].first == 0])
					countNotStarted++;
			}
		}		
		
		if (count == 0) return -1;
		return countNotStarted / (double)count;
	}*/

	// Average difference between StartedAttempt time and passed time(not for every step)
	double GetAveragePassingTime()
	{
		long long sum = 0, count = 0;
		for (auto i : stepPassed)
		{
			if (stepStartedAttempt[i.first] == 0 || i.second == 0)
				continue;
			count++;
			sum += abs(i.second - stepStartedAttempt[i.first]);
		}
		if (count == 0) return -1;
		return sum / (double)count;
	}

	// Average difference between max and min time on one step
	double GetAverageIdlingTime()
	{
		long long sum = 0, count = 0;
		for (auto stat : visitingStats)
		{
			if (stat.second.size() < 2 || stat.second[stat.second.size() - 1] - stat.second[0] == 0)
				continue;
			count++;
			sum += abs(stat.second[stat.second.size() - 1] - stat.second[0]);
		}
		if (count == 0) return -1;
		return sum / (double)count;
	}

	double GetPassedStepsPart() // Part of all steps count
	{
		return stepPassed.size() / (double)structure.size();
	}

	double GetSkippedPersentage()
	{
		int countVisited = visitingStats.size(),
			count = StepAbsPosition[visitingStats[visitingStats.size() - 1].first] - StepAbsPosition[visitingStats[0].first] + 1;
		if (countVisited == 0 || count == 0)
			return 1;
		return 1 - (countVisited / (double)count);
	}

	// Time Deviation from "time center" 
	double GetTimeDeviation() // 1 / Deviation from distribution of visiting times
	{
		long long leftSum = 0, rightSum = 0;
		for (int i = 0; i < visitingStats.size(); i++)
		{
			for (int j = 0; j < visitingStats[i].second.size(); j++)
			{
				long long time = visitingStats[i].second[j];
				if (time < middleTime)
					leftSum += middleTime - time;
				else
					rightSum += time - middleTime;
			}
		}
		if (rightSum + leftSum == 0) return 0;
		return rightSum / (double)(rightSum + leftSum) - 0.5;
	}

	// Score deviation from "time center" 
	double GetScoreDeviation()
	{
		long long leftSum = 0, rightSum = 0;
		for (int i = 0; i < visitingStats.size(); i++)
		{
			for (auto pass : stepPassed)
			{
				long long time = pass.second;
				if (time < middleTime)
					leftSum += middleTime - time;
				else
					rightSum += time - middleTime;
			}
		}
		if (rightSum + leftSum == 0) return 0;
		return rightSum / (double)(rightSum + leftSum) - 0.5;
	}

	double GetScoredUnscoredRatio()
	{
		if (1 - features["CostablePercentage"] == 0) return 1;
		return features["CostablePercentage"] / (double)(1 - features["CostablePercentage"]);
	}

	//Events count
	double GetEventsCount()
	{
		int sum = 0;
		for (auto visit : visitingStats)
			for (auto time : visit.second)
				sum++;
		return sum;
	}

	/* Some high-level features */

	double GetWorkDaysPercentage()
	{
		int count = 0;
		for (auto day : days)
		{
			if (!day.empty())
				count++;
		}
		if (days.size() == 0)
			return 0;
		return count / (double)days.size();
	}

	// If day has at least one visiting
	double GetDayAverage()
	{
		int stepsCount = 0, dayCount = 0;
		for (auto day : days)
		{
			if (day.size() == 0) continue;
			vector<int> visited;
			int stepsDayCount = 0;
			for (pair<long long, int> time : day)
			{
				bool isExists = false;
				for (int i = 0; i < visited.size(); i++)
					if (visited[i] == time.second)
					{
						isExists = true;
					}
				if (isExists) continue;
				
				stepsCount++;
				visited.push_back(time.second);
			}
			stepsCount += stepsDayCount;
			dayCount++;
		}
		if (dayCount == 0)
			return 0;
		return stepsCount / (double)dayCount;
	}

	double GetMaxIdleDays_Normalized()
	{
		const int week = 14;
		int counter = 0, maxIdle = 0;
		for (auto day : days)
		{
			if (day.empty())
				counter++;
			else
			{
				if (maxIdle < counter)
					maxIdle = counter;
				counter = 0;
			}
		}
		if (maxIdle < counter)
			maxIdle = counter;

		return maxIdle / (double)week;
	}

	double GetMaxVisitingDays_Normalized()
	{
		const int week = 14;
		int counter = 0, maxVisiting = 0;
		for (auto day : days)
		{
			if (!day.empty())
				counter++;
			else
			{
				if (maxVisiting < counter)
					maxVisiting = counter;
				counter = 0;
			}
		}
		if (maxVisiting < counter)
			maxVisiting = counter;

		return maxVisiting / (double)week;
	}
	
	// If day has at least one visiting
	double GetAverageDayPart()
	{
		const int UNIX_DAY = 86400;
		long long dayCount = 0;
		double ratiosSum = 0;

		for (auto day : days)
		{
			if (day.empty()) continue;
			dayCount++;
			ratiosSum += abs(day[day.size() - 1].first - day[0].first) / (double) UNIX_DAY;
		}
		if (dayCount == 0)
			return 0;
		return ratiosSum / (double)dayCount;
	}	

	void CalculateFeatures()
	{
		CalculateTimes();
		features["Score"] = GetScore();
		features["PassedStepsPart"] = GetPassedStepsPart();
		features["CostablePercentage"] = GetCostablePercentage();
		features["ScoredUnscoredRatio"] = GetScoredUnscoredRatio(); // Must stay after CostablePercentage
		features["AveragePassingTime"] = GetAveragePassingTime();
		features["AverageIdlingTime"] = GetAverageIdlingTime();
		features["SkippedPersentage"] = GetSkippedPersentage();
		features["TimeDeviation"] = GetTimeDeviation();
		features["ScoreDeviation"] = GetScoreDeviation();
		features["EventsCount"] = GetEventsCount();
		/* Some high-level features */
		features["WorkDaysPercentage"] = GetWorkDaysPercentage();
		features["DayAverage"] = GetDayAverage();
		features["MaxIdleDays"] = GetMaxIdleDays_Normalized();
		features["MaxVisitingDays"] = GetMaxVisitingDays_Normalized();
		features["AverageDayPart"] = GetAverageDayPart();
	}

	const char* titles = "id,PassedStepsPart,CostablePercentage,ScoredUnscoredRatio,Score,AveragePassingTime,AverageIdlingTime,SkippedPersentage,TimeDeviation,ScoreDeviation,EventsCount,WorkDaysPercentage,DayAverage,MaxIdleDays,MaxVisitingDays,AverageDayPart,passed";
	string GetDataForSave()
	{
		string str = "";
		str += to_string(id) + ",";
		str += to_string(featuresNormalized["PassedStepsPart"]) + ",";
		str += to_string(featuresNormalized["CostablePercentage"]) + ",";
		str += to_string(featuresNormalized["ScoredUnscoredRatio"]) + ",";
		str += to_string(featuresNormalized["Score"]) + ",";
		str += to_string(featuresNormalized["AveragePassingTime"]) + ",";
		str += to_string(featuresNormalized["AverageIdlingTime"]) + ",";
		str += to_string(featuresNormalized["SkippedPersentage"]) + ",";
		str += to_string(featuresNormalized["TimeDeviation"]) + ",";
		str += to_string(featuresNormalized["ScoreDeviation"]) + ",";
		str += to_string(featuresNormalized["EventsCount"]) + ",";
		str += to_string(featuresNormalized["WorkDaysPercentage"]) + ",";
		str += to_string(featuresNormalized["DayAverage"]) + ",";
		str += to_string(featuresNormalized["MaxIdleDays"]) + ",";
		str += to_string(featuresNormalized["MaxVisitingDays"]) + ",";
		str += to_string(featuresNormalized["AverageDayPart"]) + ",";
		str += to_string(passed); // In case of _test files is filled but will not be read
		return str;
	}
};

vector<Student*> students;
int i2 = 0;

// Read and remember Raw data
void FillArray(vector<map<string, string>> &myArray, char* fileAdress)
{
	myArray.clear();
	ifstream file;
	int fileSize = FileSize(fileAdress);
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

		if(i2 == 0)
			fileSize /= str.size(); // characters in line average count 

		data = split(str);

		map<string, string> toArray;
		for (int i = 0; i < titlesSize; i++)
			toArray[titles[i]] = data[i];
		myArray.push_back(toArray);
		i2++;
		if (i2 % (fileSize / 100) == 0) { SetCursorPosistion(15, line); cout << to_string(int(i2 / (double)fileSize * 100)) + "%"; }
	}
	file.close();
	i2 = 0;
	SetCursorPosistion(0, line);
	cout << "Reading files: 100%" << endl;
	line++;
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

Student* FindStudentByID(int _id)
{
	int N = students.size();
	for (int i = 0; i < N; i++)
		if (students[i]->id == _id)
			return students[i];
	return nullptr;
}

bool StudentsIDComparator(Student* a, Student* b)
{
	return a->id < b->id;
}

// Process Raw data(eventsRaw) to raw structures(students)
void ProcessData()
{
	// Writting in visited by students events
	cout << "Structuring data: 0%";
	long long N = eventsRaw.size();
	for (int i = 0; i < N; i++)
	{
		long long userID = stoll(eventsRaw[i]["user_id"]),
			stepID = stoll(eventsRaw[i]["step_id"]),
			eventTime = stoll(eventsRaw[i]["time"]);
		string eventAction = eventsRaw[i]["action"];
		Student* student = FindStudentByID(userID);
		if (student)
		{
			student->PushBackInVisitingStats(stepID, eventTime, eventAction);
		}
		else
		{
			student = new Student(userID);
			student->PushBackInVisitingStats(stepID, eventTime, eventAction);
			students.push_back(student);
		}

		if (i2 % 1000 == 0)
		{
			SetCursorPosistion(18, line);
			cout << to_string(int(i2 / (double)N * 100)) + "%";
		}
		i2++;
	}
	SetCursorPosistion(0, line);	
	cout << "Structuring data: 100%" << endl;
	i2 = 0;
	line++;
}

// Normalize non-normalized features
void NormalizeFeatures()
{
	double maxAveragePassingTime = 0, maxAverageIdlingTime = 0, maxScoredUnscoredRatio = 0, maxScore = 0, maxEventsCount = 0, maxDayAverage = 0;
	// Finding maximums
	for (int i = 0; i < students.size(); i++)
	{
		double feature_AveragePassingTime = students[i]->features["AveragePassingTime"],
			feature_AverageIdlingTime = students[i]->features["AverageIdlingTime"],
			feature_Score = students[i]->features["Score"],
			feature_ScoredUnscoredRatio = students[i]->features["ScoredUnscoredRatio"],
			feature_EventsCount = students[i]->features["EventsCount"],
			feature_DayAverage = students[i]->features["DayAverage"];

		if (maxAveragePassingTime < feature_AveragePassingTime)
			maxAveragePassingTime = feature_AveragePassingTime;

		if (maxAverageIdlingTime < feature_AverageIdlingTime)
			maxAverageIdlingTime = feature_AverageIdlingTime;

		if (maxScoredUnscoredRatio < feature_ScoredUnscoredRatio)
			maxScoredUnscoredRatio = feature_ScoredUnscoredRatio;

		if (maxScore < feature_Score)
			maxScore = feature_Score;

		if (maxEventsCount < feature_EventsCount)
			maxEventsCount = feature_EventsCount;

		if (maxDayAverage < feature_DayAverage)
			maxDayAverage = feature_DayAverage;
	}

	// "Fake" normalizing because of few students have enormous values
	// Let us to get better features
	 double maxAverageIdlingTime_FAKE = maxAverageIdlingTime / (double)10;
	 double maxAveragePassingTime_FAKE = maxAveragePassingTime / (double)100;

	// Normalizing
	for (int i = 0; i < students.size(); i++)
	{
		// This parameters just copying
		students[i]->featuresNormalized["PassedStepsPart"] = students[i]->features["PassedStepsPart"];
		students[i]->featuresNormalized["PassingPercentage"] = students[i]->features["PassingPercentage"];
		students[i]->featuresNormalized["CostablePercentage"] = students[i]->features["CostablePercentage"];
		students[i]->featuresNormalized["SkippedPersentage"] = students[i]->features["SkippedPersentage"];
		students[i]->featuresNormalized["TimeDeviation"] = students[i]->features["TimeDeviation"];
		students[i]->featuresNormalized["ScoreDeviation"] = students[i]->features["ScoreDeviation"];
		students[i]->featuresNormalized["WorkDaysPercentage"] = students[i]->features["WorkDaysPercentage"];
		students[i]->featuresNormalized["MaxIdleDays"] = students[i]->features["MaxIdleDays"];
		students[i]->featuresNormalized["MaxVisitingDays"] = students[i]->features["MaxVisitingDays"];
		students[i]->featuresNormalized["AverageDayPart"] = students[i]->features["AverageDayPart"];

		double feature_AveragePassingTime = students[i]->features["AveragePassingTime"],
			feature_AverageIdlingTime = students[i]->features["AverageIdlingTime"];

		if (feature_AveragePassingTime != -1 && feature_AveragePassingTime <= maxAveragePassingTime_FAKE)
			students[i]->featuresNormalized["AveragePassingTime"] = feature_AveragePassingTime / (double)maxAveragePassingTime_FAKE;
		else
			students[i]->featuresNormalized["AveragePassingTime"] = 1;

		if (feature_AverageIdlingTime != -1 && feature_AverageIdlingTime <= maxAverageIdlingTime_FAKE)
			students[i]->featuresNormalized["AverageIdlingTime"] = feature_AverageIdlingTime / (double)maxAverageIdlingTime_FAKE;
		else
			students[i]->featuresNormalized["AverageIdlingTime"] = 1;

		students[i]->featuresNormalized["Score"] = students[i]->features["Score"] / (double)maxScore;
		students[i]->featuresNormalized["ScoredUnscoredRatio"] = students[i]->features["ScoredUnscoredRatio"] / (double)maxScoredUnscoredRatio;
		students[i]->featuresNormalized["EventsCount"] = students[i]->features["EventsCount"] / (double)maxEventsCount;
		students[i]->featuresNormalized["DayAverage"] = students[i]->features["DayAverage"] / (double)maxDayAverage;
	}
}

int main()
{
	// Fill in raws
	FillArray(structureRaw, structureAdress);
	FillArray(targetsRaw, targetsAderss);

	// Ordering steps and create fast access to steps' order with StepAbsPosition array
	for (int i = 0; i < structureRaw.size(); i++)
		structure.push_back(new Step(structureRaw[i]));
	sort(structure.begin(), structure.end(), CompareStepsPosition);
	for (int i = 0; i < structure.size(); i++)
		StepAbsPosition[stoll(structure[i]->fields["step_id"])] = i;
	
	// Processing raws separatly
	FillArray(eventsRaw, eventsAdress1);
	ProcessData();
	cout << "Deleting raws1..." << endl;
	line++;
	eventsRaw.clear();

	FillArray(eventsRaw, eventsAdress2);
	ProcessData();
	cout << "Deleting raws2..." << endl;
	line++;
	eventsRaw.clear();

	// Sort students
	sort(students.begin(), students.end(), StudentsIDComparator);

	// Writting in is student passed
	for (int i = 0; i < targetsRaw.size(); i++)
	{
		bool isPassed = stoi(targetsRaw[i]["passed"]);
		if (!isPassed) continue;
		int userID = stoi(targetsRaw[i]["user_id"]);
		Student *student = FindStudentByID(userID);
		if (student)
			student->passed = true;
	}

	// Sort visitedStats in students
	cout << "Sorting stats: 0%";
	int N = students.size();
	for (int i = 0; i < N; i++)
	{
		students[i]->SortStats();
		if (i % 100 == 0)
		{
			SetCursorPosistion(15, line);
			cout << to_string(int((i / (double)N * 100))) + "%";
		}
	}
	SetCursorPosistion(0, line);
	cout << "Sorting stats: 100%" << endl;
	line++;

	//Calculating features
	cout << "Calculating features: 0%" << endl;
	N = students.size();
	for (int i = 0; i < students.size(); i++)
	{
		students[i]->CalculateFeatures();
		if (i % 100 == 0)
		{
			SetCursorPosistion(22, line);
			cout << to_string(int((i / (double)N * 100))) + "%";
		}
	}
	SetCursorPosistion(0, line);
	cout << "Calculating features: 100%" << endl;
	line++;

	// Normalize data
	cout << "Normalizing features..." << endl;
	line++;
	NormalizeFeatures();

	// Saving normalized features to file
	cout << "Saving normalized features..." << endl;
	line++;
	ofstream fout(outputFileAdress);
	fout << students[0]->titles << endl;
	for (int i = 0; i < students.size(); i++)
		fout << students[i]->GetDataForSave() << endl;
	fout.close();

	// Check if original file used
	cout << endl;
	system("pause");
	return 0;
}