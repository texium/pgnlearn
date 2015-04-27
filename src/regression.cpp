#include <math.h>
#include <time.h>
#include "regression.h"
#include "utils.h"

size_t M = 0;
size_t N = 0;
vector< vector<double> > X;
vector<int> y;

const double alpha = 1;           // gradient method step size
const double lambda = 0.000001;   // regularization parameter

double h_theta(const vector<double>& theta, const vector<double>& x)
{
	assert(x.size() == theta.size());

	double z = 0;
	for (size_t i = 0; i < x.size(); ++i) z += theta[i] * x[i];
	return 1. / (1. + exp(-z));
}

double CostFunction(const vector<double>& theta, vector<double>& grad)
{
	double r = 0;
	grad.resize(N);
	for (size_t j = 0; j < N; ++j) grad[j] = 0;

	int M1 = 0;
	for (size_t i = 0; i < M; ++i)
	{
		if (y[i] == 1) continue;

		const vector<double>& x = X[i];
		double h = h_theta(theta, x);

		double yi = 0;
		if (y[i] == 2)
		{
			r += -log(h);
			yi = 1;
		}
		else if (y[i] == 0)
		{
			r += -log(1 - h);
			yi = 0;
		}

		for (size_t j = 0; j < N; ++j)
			grad[j] += (h - yi) * x[j];

		++M1;
	}
	
	for (size_t j = 0; j < N; ++j) grad[j] /= M1;

	// regularization
	double reg = 0;
	for (size_t j = 0; j < N; ++j)
	{
		reg += lambda * theta[j] * theta[j];
		grad[j] += 2 * lambda * theta[j];
	}

	return r / M1 + reg;
}

bool LoadDataset(const string& file_in)
{
	FILE* src = fopen(file_in.c_str(), "rt");
	if (!src)
	{
		cout << "Can't open file: " << file_in << endl;
		return false;
	}

	cout << "Loading dataset..." << endl;
	char buf[4096];
	while (fgets(buf, sizeof(buf), src))
	{
		vector<string> tokens;
		Split(buf, tokens);

		++M;
		if (M == 1)
			N = tokens.size() - 1;
		else
		{
			if (tokens.size() != N + 1)
			{
				cout << "Incorrect number of parameters in line " << M << endl;
				return false;
			}
		}

		vector<double> line;
		for (size_t i = 0; i < N; ++i) line.push_back(atof(tokens[i].c_str()));
		X.push_back(line);
		y.push_back(atoi(tokens[N].c_str()));

		cout << "[ " << M << " x " << N << " ]\r";
	}
	fclose(src);
	cout << endl;

	return true;
}

ostream& operator<< (ostream& os, vector<double> v)
{
	os << "[ ";
	for (size_t i = 0; i < v.size(); ++i) os << v[i] << " ";
	os << "]";
	return os;
}

bool SolveGradient(const string& file_in)
{
	if (!LoadDataset(file_in)) return false;

	cout << "Solving (gradient method)..." << endl;

	double J = 0;
	vector<double> grad;
	vector<double> theta;
	theta.resize(N);

	FILE* dst = fopen("iterations.txt", "wt");
	int iter;
	for (iter = 0; iter < 10000; ++iter)
	{
		J = CostFunction(theta, grad);
		if (iter % 1000 == 0)
			cout << "Iter " << iter << ": " << theta << " -> " << J << endl;
		for (size_t j = 0; j < N; ++j) theta[j] -= alpha * grad[j];

		// if (fabs(J - Jprev) < 1e-6) break;
		// Jprev = J;

		if (dst && theta[0] > 0)
		{
			fprintf(dst, "%d\t%lf\t%lf\t%lf\t%lf\t%lf\t%lf\n",
				iter,
				100 * theta[0] / theta[0],
				100 * theta[1] / theta[0],
				100 * theta[2] / theta[0],
				100 * theta[3] / theta[0],
				100 * theta[4] / theta[0],
				J);
		}
	}
	if (dst) fclose(dst);
	cout << "Iter " << iter << ": " << theta << " -> " << J << endl;

	cout << endl;
	cout << "PIECE VALUES:" << endl << endl;

	cout << "Pawn:   " << 100 * theta[0] / theta[0] << endl;
	cout << "Knight: " << 100 * theta[1] / theta[0] << endl;
	cout << "Bishop: " << 100 * theta[2] / theta[0] << endl;
	cout << "Rook:   " << 100 * theta[3] / theta[0] << endl;
	cout << "Queen:  " << 100 * theta[4] / theta[0] << endl;

	return true;
}

bool SolveSelection(const string& file_in)
{
	if (!LoadDataset(file_in)) return false;

	cout << "Solving (selection method)..." << endl;
	RandSeed(time(0));

	vector<double> theta, thetaBest;
	double J, Jbest = 0;
	vector<double> grad;

	theta.resize(N);
	thetaBest.resize(N);

	Jbest = CostFunction(thetaBest, grad);

	FILE* dst = fopen("J-selection.txt", "wt");
	int iter;
	for (iter = 0; iter < 10000; ++iter)
	{
		theta[0] = RandDouble(0, 2);
		theta[1] = RandDouble(0, 5);
		theta[2] = RandDouble(0, 5);
		theta[3] = RandDouble(0, 7);
		theta[4] = RandDouble(0, 12);
		theta[5] = RandDouble(0, 1);
		theta[6] = RandDouble(0, 1);

		J = CostFunction(theta, grad);
		if (J < Jbest)
		{
			thetaBest = theta;
			Jbest = J;
			if (dst) fprintf(dst, "%d %lf\n", iter, Jbest);
		}

		if (iter % 1000 == 0)
			cout << "Iter " << iter << ": " << thetaBest << " -> " << Jbest << endl;
	}
	if (dst) fclose(dst);

	theta = thetaBest;
	cout << "Iter " << iter << ": " << thetaBest << " -> " << Jbest << endl;

	cout << endl;
	cout << "PIECE VALUES:" << endl << endl;
	cout << "Pawn:   " << 100 * theta[0] / theta[0] << endl;
	cout << "Knight: " << 100 * theta[1] / theta[0] << endl;
	cout << "Bishop: " << 100 * theta[2] / theta[0] << endl;
	cout << "Rook:   " << 100 * theta[3] / theta[0] << endl;
	cout << "Queen:  " << 100 * theta[4] / theta[0] << endl;
	cout << "bpair:  " << 100 * theta[5] / theta[0] << endl;
	cout << "bpairE: " << 100 * theta[6] / theta[0] << endl;
	cout << endl;

	return true;
}