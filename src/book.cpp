#include "book.h"
#include "moves.h"
#include "notation.h"
#include "utils.h"

bool ReadPGN(const string& file_in, string& file_out)
{
	FILE* src = fopen(file_in.c_str(), "rt");
	if (src == NULL)
	{
		cout << "Can't open file: " << file_in << endl << endl;
		return false;
	}

	size_t dot = file_in.find(".");
	if (dot != string::npos)
		file_out = file_in.substr(0, dot) + ".mat";
	else
		file_out = file_in + ".mat";

	FILE* dst = fopen(file_out.c_str(), "wt");
	if (dst == NULL)
	{
		cout << "Can't open file: " << file_out << endl << endl;
		fclose(src);
	return false;
	}

	Position pos;
	char buf[256];
	int result = 0, games = 0;
	int delta[5] = {0, 0, 0, 0, 0};
	int deltaPrev[5] = {0, 0, 0, 0, 0};
	int qmoves = 0;

	int numGames[100], numPoints[100];
	memset(numGames, 0, 100 * sizeof(int));
	memset(numPoints, 0, 100 * sizeof(int));

	cout << "Reading file: " << file_in << endl;

	while (fgets(buf, sizeof(buf), src))
	{
		if (buf[strlen(buf) - 1] == '\n') buf[strlen(buf) - 1] = 0;

		string s(buf);
		if (s.empty()) continue;

		if (s.find("[Result") == 0)
		{
			// new game
			pos.SetInitial();
			deltaPrev[0] = deltaPrev[1] = deltaPrev[2] = deltaPrev[3] = deltaPrev[4] = 999;
			qmoves = 0;

			if (s == "[Result \"1-0\"]")
				result = 2;
			else if (s == "[Result \"0-1\"]")
				result = 0;
			else
				result = 1;
			cout << "Games: " << ++games << "\r";
			continue;
		}

		if (s.find("[") == 0) continue;

		vector<string> tokens;
		Split(s, tokens, ". ");
		for (size_t i = 0; i < tokens.size(); ++i)
		{
			string tk = tokens[i];
			if (tk.find(".") != string::npos) continue;

			Move mv = StrToMove(tk, pos);
			if (mv == 0) continue;

			if (pos.MakeMove(mv))
			{
				++qmoves;
				if (mv.Captured() || mv.Promotion() || pos.InCheck()) qmoves = 0;

				delta[0] = pos.Count(PW) - pos.Count(PB);
				delta[1] = pos.Count(NW) - pos.Count(NB);
				delta[2] = pos.Count(BW) - pos.Count(BB);
				delta[3] = pos.Count(RW) - pos.Count(RB);
				delta[4] = pos.Count(QW) - pos.Count(QB);

				if (qmoves >= 2 &&
					(delta[0] != deltaPrev[0] ||
					 delta[1] != deltaPrev[1] ||
					 delta[2] != deltaPrev[2] ||
					 delta[3] != deltaPrev[3] ||
					 delta[4] != deltaPrev[4]))
				{
					fprintf(dst, "%2d %2d %2d %2d %2d %2d\n", delta[0], delta[1], delta[2], delta[3], delta[4], result);
					deltaPrev[0] = delta[0];
					deltaPrev[1] = delta[1];
					deltaPrev[2] = delta[2];
					deltaPrev[3] = delta[3];
					deltaPrev[4] = delta[4];

					int index = (1 * delta[0] + 3 * delta[1] + 3 * delta[2] + 5 * delta[3] + 9 * delta[4]) + 50;
					if (index >= 0 && index < 100)
					{
						numGames[index] += 1;
						numPoints[index] += result;
					}
				}
			}
		}
	}
	cout << endl;
	fclose(src);
	fclose(dst);
	cout << "Created file: " << file_out << endl;

	FILE* dst2 = fopen("probability.txt", "wt");
	for (int index = 0; index < 100; ++index)
	{
		if (numGames[index] > 0)
			fprintf(dst2, "%d, %d, %d, %lf\n", index - 50, numGames[index], numPoints[index], 0.5 * double(numPoints[index]) / numGames[index]);
	}
	fclose(dst2);

	return true;
}
////////////////////////////////////////////////////////////////////////////////

void MakeFakeFile(const string& file_out)
{
	FILE* dst = fopen(file_out.c_str(), "wt");
	if (!dst) return;

	for (int i = 0; i < 10000; ++i)
	{
		int qw = rand() % 2;
		int rw = rand() % 3;
		int bw = rand() % 3;
		int nw = rand() % 3;
		int pw = rand() % 8;

		int qb = rand() % 2;
		int rb = rand() % 3;
		int bb = rand() % 3;
		int nb = rand() % 3;
		int pb = rand() % 8;

		int result;
		int mat = 900 * (qw - qb) + 550 * (rw - rb) + 350 * (bw - bb) + 300 * (nw - nb) + 100 * (pw - pb);
		if (mat > 200)
			result = 2;
		else if (mat < 200)
			result = 0;
		else
			result = rand() % 3;

		fprintf(dst, "%2d %2d %2d %2d %2d %2d\n",
			pw - pb,
			nw - nb,
			bw - bb,
			rw - rb,
			qw - qb,
			result);
	}
	fclose(dst);
}
////////////////////////////////////////////////////////////////////////////////

bool ReadPGNAndSaveMatTable(const string& file_in, string& file_out)
{
	FILE* src = fopen(file_in.c_str(), "rt");
	if (src == NULL)
	{
		cout << "Can't open file: " << file_in << endl << endl;
		return false;
	}

	size_t dot = file_in.find(".");
	if (dot != string::npos)
		file_out = file_in.substr(0, dot) + ".mtbl";
	else
		file_out = file_in + ".mat";

	FILE* dst = fopen(file_out.c_str(), "wb");
	if (dst == NULL)
	{
		cout << "Can't open file: " << file_out << endl << endl;
		fclose(src);
	return false;
	}

	Position pos;
	char buf[256];
	int result = 0, qmoves = 0, games = 0;

	int* wins = new int[212868];
	int* losses = new int[212868];
	int* draws = new int[212868];

	memset(wins, 0, 212868 * sizeof(int));
	memset(losses, 0, 212868 * sizeof(int));
	memset(draws, 0, 212868 * sizeof(int));

	cout << "Reading file: " << file_in << endl;

	while (fgets(buf, sizeof(buf), src))
	{
		if (buf[strlen(buf) - 1] == '\n') buf[strlen(buf) - 1] = 0;

		string s(buf);
		if (s.empty()) continue;

		if (s.find("[Result") == 0)
		{
			// new game
			pos.SetInitial();
			qmoves = 0;

			if (s == "[Result \"1-0\"]")
				result = 2;
			else if (s == "[Result \"0-1\"]")
				result = 0;
			else
				result = 1;
			cout << "Games: " << ++games << "\r";
			continue;
		}

		if (s.find("[") == 0) continue;

		vector<string> tokens;
		Split(s, tokens, ". ");
		for (size_t i = 0; i < tokens.size(); ++i)
		{
			string tk = tokens[i];
			if (tk.find(".") != string::npos) continue;

			Move mv = StrToMove(tk, pos);
			if (mv == 0) continue;

			if (pos.MakeMove(mv))
			{
				++qmoves;
				if (mv.Captured() || mv.Promotion()) qmoves = 0;
				if (qmoves == 2)
				{
					int index = pos.MtblIndex();
					if (index != -1)
					{
						if (result == 2)
							wins[index]++;
						else if (result == 1)
							draws[index]++;
						else if (result == 0)
							losses[index]++;
					}
				}
			}
		}
	}

	int combs = 0;
	for (int i = 0; i < 212868; ++i)
	{
		int games = wins[i] + draws[i] + losses[i];
		if (games > 1)
		{
			double prob = 100 * (wins[i] + 0.5 * draws[i]) / games;
			char chProb = (char)prob;
			fwrite(&chProb, 1, 1, dst);
			++combs;
		}
		else
		{
			unsigned char chProb = 0xff;
			fwrite(&chProb, 1, 1, dst);
		}
	}

	cout << endl;
	fclose(src);
	fclose(dst);

	cout << "Combinations: " << combs << endl;
	cout << "Created file: " << file_out << endl;
	return true;
}
////////////////////////////////////////////////////////////////////////////////
