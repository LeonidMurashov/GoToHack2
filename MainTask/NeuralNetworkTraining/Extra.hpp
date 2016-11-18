using namespace std;
#include <Windows.h>
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

void SetCursorPosistion(int x, int y)
{
	HANDLE hOut;
	COORD Position;

	hOut = GetStdHandle(STD_OUTPUT_HANDLE);

	Position.X = x;
	Position.Y = y;
	SetConsoleCursorPosition(hOut, Position);
}

vector<std::string> split(const std::string& str, const char* delim = ",")
{
	vector<string> dest;
	char** a = new char*;
	char* pTempStr = _strdup(str.c_str());
	char* pWord = strtok_s(pTempStr, delim, a);
	while (pWord != NULL)
	{
		dest.push_back(pWord);
		pWord = strtok_s(NULL, delim, a);
	}
	free(pTempStr);
	return dest;
}