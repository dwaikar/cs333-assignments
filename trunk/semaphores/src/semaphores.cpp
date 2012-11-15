#include <pthread.h>
#include <semaphore.h>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <queue>
using namespace std;

#define N_COUNTR 10 + 2
#define N 1
#define M 1
#define C 2
#define B 3

sem_t semCount, semBuf, semPrint;
long long count, buf[B], writeI, readI;

// COUNTER
void* counter(void* a) {
	int x = (int) a, t;
	while (!sleep(N)) {
		sem_wait(&semCount);
		t = ++count;
		sem_post(&semCount);
		sem_wait(&semPrint), cout << "N" << x << " - Message " << t << "." << endl, sem_post(&semPrint);
	}
	sem_wait(&semPrint), cout << "N" << x << " - Termminated." << endl, sem_post(&semPrint);
	pthread_exit(NULL);
}

// MONITOR
void* monitor(void* a) {
	int s, t;
	while (!sleep(M)) {
		// sem_wait(&semPrint), cout << "M - Waiting to read counter." << endl, sem_post(&semPrint);
		sem_wait(&semCount);
		t = count;
		sem_post(&semCount);
		// sem_wait(&semPrint), cout << "M - Reading a count of value " << t << "." << endl, sem_post(&semPrint);
		sem_wait(&semBuf);
		bool write = (writeI != readI);
		if (write)
			buf[s = (writeI - 1 + B) % B] = t, writeI = (writeI + 1) % B;
		sem_post(&semBuf);
		if (write) {
			sem_wait(&semPrint), cout << "M  - Writing " << t << " to [" << s << "]." << endl << endl, sem_post(&semPrint);
			sem_wait(&semCount);
			count = 0;
			sem_post(&semCount);

		} else
			sem_wait(&semPrint), cout << "M  - Buffer full!" << endl << endl, sem_post(&semPrint);
	}
	sem_wait(&semPrint), cout << "M  - Termminating." << endl, sem_post(&semPrint);
	pthread_exit(NULL);
}

// COLLECTOR
void* collector(void* a) {
	int s, t;
	while (!sleep(C)) {
		sem_wait(&semBuf);
		bool read = ((readI + 1) % B != writeI);
		if (read)
			t = buf[s = readI], readI = (readI + 1) % B;
		sem_post(&semBuf);
		if (read)
			sem_wait(&semPrint), cout << "C  - Reading " << t << " from [" << s << "]." << endl, sem_post(&semPrint);
		else
			sem_wait(&semPrint), cout << "C  - Buffer empty!" << endl, sem_post(&semPrint);
	}
	sem_wait(&semPrint), cout << "C  - Termminating." << endl, sem_post(&semPrint);
	pthread_exit(NULL);
}

// MAIN
int main(int argc, char * argv[]) {
	sem_init(&semCount, NULL, 1);
	sem_init(&semBuf, NULL, 1);
	sem_init(&semPrint, NULL, 1);
	pthread_t tid[N_COUNTR];
	memset(buf, -1, sizeof writeI);
	readI = count = 0;
	writeI = 1;
	pthread_create(&tid[0], NULL, monitor, NULL);
	pthread_create(&tid[1], NULL, collector, NULL);
	for (int i = 2; i < N_COUNTR; ++i)
		pthread_create(&tid[i], NULL, counter, (void*) (i - 2));
	pthread_exit(NULL);
}
