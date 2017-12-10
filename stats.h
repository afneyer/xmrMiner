#pragma once
#include <thread>
#include <atomic>
#include <vector>
#include <map>
#include <mutex>
#include "crypto/cryptonight.h"

class stats
{
public:
	// read kValues from file
	static void readNoncesFromFile();
	static void writeNonceValuesToFile();
	static std::string getexepath();

	static void updateNonceValue();

	static void printMap(std::multimap<int, int> map, std::string fileName);

	static void printMap(std::map<int, int> map, std::string fileName);

	static void transformNonceCount();

	

	static void sortNonceCount();
	static int getNonce(int i);
	static int getSequentialNonce();
	static int getNonce();
	static int getNonceFromList();
	static int getRandomNonce();
	static int getStatisticalNonce();
	static int getNonceFromAlgo();
	static void resetNonceStats();
	static void readAndBuildStats();
	static void reBuildStats();
	static void difficultyStats(uint64_t target, uint64_t hash);
	static void addtoNonceList(int k, int hashCount);
	static void fillRandomMap();
	static std::mutex mtx;

private:
	
	static int maxNonce;
	static int nonceIndex;
	static int lastNonce;

	static const int nonceListSize = 65444;
	static std::vector<int> nonceList;
	static int nonceListIndex;

	static std::map<int, int> nonceCountMap;
	static const int kMapSize = 16386;
	static std::multimap<int, int> nonceCountTrans;
	static std::map<int, int> nonceSorted;

	static int totalHashCount;
	static int goodNonceCount;

	static const int randomSize = 16384;
	static std::map<int, int> randomMap;

	static uint64_t stats::difficultyCounter;
	static uint64_t stats::targetSum;
	static uint64_t stats::difficultyLow;
	static uint64_t stats::difficultyLowSum;

};

