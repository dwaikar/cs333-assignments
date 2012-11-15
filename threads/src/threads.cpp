#include <pthread.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstring>
#include <vector>
#include <ctime>
using namespace std;

ifstream in;
ofstream out;
pthread_attr_t attr;
// MLTIPLICATION
vector<vector<double> > m[3];
void *mulCell(void* p) {
	int x = *((int*) p) & 0xFFFF;
	int y = (*((int*) p) >> 16) & 0xFFFF;
	for (size_t i = 0; i < m[1].size(); ++i)
		m[2][y][x] += m[0][y][i] * m[1][i][x];
	pthread_exit(0);
}
void* mulRow(void* p) {
	int y = *((int*) p) & 0xFFFF;
	for (size_t x = 0; x < m[1][0].size(); ++x)
		for (size_t i = 0; i < m[1].size(); ++i)
			m[2][y][x] += m[0][y][i] * m[1][i][x];
	pthread_exit(0);
}

// SORT
vector<double> lst;
void* sort(void* p) {
	vector<double>* A = ((vector<double>*) p);
	if ((*A).size() < 2)
		pthread_exit(0);
	// divide
	pthread_t thread[2];
	vector<double> a[2];
	a[0] = vector<double>((*A).begin(), (*A).begin() + (*A).size() / 2);
	a[1] = vector<double>((*A).begin() + (*A).size() / 2, (*A).end());
	vector<double>* arg[2];
	arg[0] = &a[0];
	arg[1] = &a[1];
	for (int i = 0; i < 2; ++i)
		pthread_create(&thread[i], &attr, sort, (void*) arg[i]);
	// join
	void* status;
	for (int i = 0; i < 2; ++i)
		pthread_join(thread[i], &status);
	// merge
	size_t i = 0, j = 0;
	while (i < a[0].size() && j < a[1].size())
		if (a[0][i] <= a[1][j])
			(*A)[i + j] = a[0][i], ++i;
		else
			(*A)[i + j] = a[1][j], ++j;
	while (i < a[0].size())
		(*A)[i + j] = a[0][i], ++i;
	while (j < a[1].size())
		(*A)[i + j] = a[1][j], ++j;
	pthread_exit(0);
}

// CALLS
void callMulCell() {
	cout << "multiplying..." << endl;
	int h = m[2].size(), w = m[2][0].size();
	int arg[h][w];
	pthread_t thread[h][w];
	for (int y = 0; y < h; ++y)
		for (int x = 0; x < w; ++x)
			arg[y][x] = x | (y << 16), pthread_create(&thread[y][x], &attr, mulCell, (void*) &arg[y][x]);
	void *status;
	for (int y = 0; y < h; ++y)
		for (int x = 0; x < w; ++x)
			pthread_join(thread[y][x], &status);
}
void callMulRow() {
	cout << "multiplying..." << endl;
	int h = m[2].size();
	int arg[h];
	pthread_t thread[h];
	for (int y = 0; y < h; ++y)
		arg[y] = y, pthread_create(&thread[y], &attr, mulRow, (void*) &arg[y]);
	void* status;
	for (int y = 0; y < h; ++y)
		pthread_join(thread[y], &status);
}
void callSort() {
	cout << "sorting..." << endl;
	pthread_t thread;
	vector<double>* arg = &lst;
	pthread_create(&thread, &attr, sort, (void*) arg);
	void* status;
	pthread_join(thread, &status);
}

// INPUT
void readMatrices() {
	cout << "reading..." << endl;
	int h[3], w[3];
	for (int i = 0; i < 2; ++i) {
		in >> h[i] >> w[i];
		m[i] = vector<vector<double> >(h[i]);
		for (int y = 0; y < h[i]; ++y) {
			m[i][y] = vector<double>(w[i]);
			for (int x = 0; x < w[i]; ++x)
				in >> m[i][y][x];
		}
	}
	h[2] = h[0], w[2] = w[1];
	m[2] = vector<vector<double> >(h[2]);
	for (int y = 0; y < h[2]; ++y)
		m[2][y] = vector<double>(w[2], 0);
}
void readList() {
	cout << "reading..." << endl;
	int n;
	in >> n;
	lst = vector<double>(n);
	for (int i = 0; i < n; ++i)
		in >> lst[i];
}

// OUTPUT
void writeMatrices() {
	cout << "writing..." << endl;
	out.precision(2);
	for (size_t y = 0; y < m[2].size(); ++y) {
		for (size_t x = 0; x < m[2][y].size(); ++x)
			out << fixed << setw(8) << m[2][y][x];
		out << endl;
	}
}
void writeList() {
	cout << "writing..." << endl;
	for (size_t i = 0; i < lst.size(); ++i)
		out << lst[i] << " ";
	out << endl;
}

// M(*A)IN
int handleArgs(int argc, char** arg) {
	cout << "\"" << arg[1] << "\"" << endl;
	if (argc == 4)
		if (strcmp(arg[1], "-mulCell") == 0)
			return 0;
		else if (strcmp(arg[1], "-mulRow") == 0)
			return 1;
		else if (strcmp(arg[1], "-sort") == 0)
			return 2;
	return -1;
}
int main(int argc, char** arg) {
	in.open(arg[2]), out.open(arg[3]);
	pthread_attr_init(&attr), pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	int start, end;
	//
	switch (handleArgs(argc, arg)) {
	case 0:
		readMatrices();
		start = clock(), callMulCell(), end = clock();
		writeMatrices();
		break;
	case 1:
		readMatrices();
		start = clock(), callMulRow(), end = clock();
		writeMatrices();
		break;
	case 2:
		readList();
		start = clock(), callSort(), end = clock();
		writeList();
		break;
	default:
		cout << "wrong input" << endl;
		cout << "usage: ./threads <options> <input_file> <output_file>" << endl;
		cout << "	<options>:" << endl;
		cout << "		-mulCell	: a thread for each cell" << endl;
		cout << "		-mulRow		: a thread for each row" << endl;
		cout << "		-sort		: merge sort" << endl;
		return -1;
	}
	cout.precision(3);
	cout << fixed << ((end - start) * 1000.0 / CLOCKS_PER_SEC) << " ms" << endl;
	//
	pthread_attr_destroy(&attr);
	in.close(), out.close();
	return 0;
}
