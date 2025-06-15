#include "stdafx.h"
int MaxThreadLimit;


void ThreadTest (atomic_int &ActiveThreads)
{
	int s = 0;
	float k = 0;
	for (double counter = 0; counter < 10; counter += 0.00001)
	{
        s++;
		k += 1.144564542315345f;
	}
	ActiveThreads--;
}


void FindBestThreadNumber ()
{
	int nThreads = thread::hardware_concurrency();										// Rileva il numero massimo di threads consentiti dal sistema
	vector <long long> times(nThreads);
	do
	{
		atomic_int ActiveThreads = 0;
		vector <thread> TestArray;
		chrono::steady_clock::time_point begin = chrono::steady_clock::now();			// Inizia a contare il tempo
		for (unsigned char i = 0; i < 70;)												// Crea 70 tasks
		{
			if (ActiveThreads < nThreads)												// Controlla che il numero di threads attualmente attivi non superi il massimo consentito
			{
				ActiveThreads++;
				TestArray.push_back(thread(ThreadTest, ref(ActiveThreads)));			// Associa un task ad un thread
				i++;
			}
		}
		for (unsigned int t = 0; t < TestArray.size(); t++)								// Aspetta che tutti i threads abbiano finito
			TestArray[t].join();
		chrono::steady_clock::time_point end= chrono::steady_clock::now();				// Interrompe il contatore del tempo
		times[nThreads - 1] = chrono::duration_cast<chrono::microseconds>(end - begin).count();
		nThreads--;
	} while (nThreads > 0);
	MaxThreadLimit = 1 + distance(times.begin(), min_element(times.begin(), times.end()));
	msg(msg::TGT::FILE_CONS, msg::TYP::DBG) << "For optimal performance, TRAODLE will use " << MaxThreadLimit << " out of " << nThreads << " threads available in your system.";
}