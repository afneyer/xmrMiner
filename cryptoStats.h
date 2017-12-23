#pragma once

#include <map>
#include <list>

class cryptoStats
{
public:
	cryptoStats();
	static void readFromFile();
	static void buildHashNonceMaps();
	static void writeToFile();
	static void addPoint(int hashBits, int nonce);
	static void writeStats();
	static void resetNonceValues();

	~cryptoStats(); // not sure this one is needed

private:
	static const int numHashBits = 10;
	static const int numHashes = 2 ^ (numHashBits);
	static const int numNonces = 2 ^ (numHashBits + 1);
	
	// list of values
	static std::list<int> hashNonceList[2];
	
	// for statistics
	static int savedValues;
	static int ignoredValues;
	
	static std::map<int,int> fMap;
	static std::map<int, int> fValue; // key=nonce, value=count (needs sort by value), element zero is the last nonce
};

