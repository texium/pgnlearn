#include "book.h"
#include "regression.h"

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		printf("Usage: pgnlearn.exe <filename.pgn>\n");
		return 1;
	}

	// MakeFakeFile("fake.mat");
	// Solve("fake.mat");

	InitBitboards();
	string file_mat;
	// ReadPGNAndSaveMatTable(argv[1], file_mat);

	if (!ReadPGN(argv[1], file_mat)) return 1;
	if (!SolveGradient(file_mat)) return 1;
	// if (!SolveSelection(file_mat)) return 1;

	cout << "Press ENTER to finish" << endl;
	getchar();
	return 0;
}