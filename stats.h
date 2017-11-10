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
	static void readKValuesFromFile();
	static void writeKValuesToFile();
	static std::string getexepath();

	static void updateKCount();

	static void printMap(std::multimap<int, int> map, std::string fileName);

	static void printMap(std::map<int, int> map, std::string fileName);

	static void transformKCount();

	

	static void sortKCount();
	static int getNonce(int i);
	static int getNonce();
	static void resetNonceCounter();
	static void readAndBuildStats();
	static void reBuildStats();
	static void addtoKList(int k);
	static std::mutex mtx;

private:
	
	static int maxNonce;
	static int nonceIndex;

	static const int kListSize = 16384;
	static std::vector<int> kList;
	static int kListIndex;

	static std::map<int, int> kCount;
	static const int kMapSize = 2048;
	static std::multimap<int, int> kCountTrans;
	static std::map<int, int> kSorted;

};

