#include "stats.h"
#include "console.h"
#include <cstring>
#include <string>
#include <fstream>
#include <windows.h>
#include <algorithm>
#include <iterator>
#include <random>

std::vector<int> stats::nonceList;
std::map<int, int> stats::nonceCountMap;
std::multimap<int, int> stats::nonceCountTrans;
std::map<int, int> stats::nonceSorted;
std::map<int, int> stats::randomMap;
int stats::maxNonce;
int stats::nonceIndex;
int stats::lastNonce = 0;
int stats::nonceListIndex = 0;
std::mutex stats::mtx;
int stats::goodNonceCount;
int stats::totalHashCount;

uint64_t stats::targetSum = 0;
uint64_t stats::difficultyCounter = 0;
uint64_t stats::difficultyLow = 0;
uint64_t stats::difficultyLowSum = 0;

// todo uint64_t stats::difficultyCounter;
// todo uint64_t stats::difficultyAverage;

template<typename A, typename B>
std::pair<B, A> flip_pair(const std::pair<A, B> &p)
{
	return std::pair<B, A>(p.second, p.first);
}

template<typename A, typename B>
std::multimap<B, A> flip_map(const std::map<A, B> &src)
{
	std::multimap<B, A> dst;
	std::transform(src.begin(), src.end(), std::inserter(dst, dst.begin()),
		flip_pair<A, B>);
	return dst;
}

void stats::readNoncesFromFile()
{
	std::ifstream inFile;

	inFile.open("kValues.txt");
	if (!inFile) {
		printer::inst()->print_msg(L1, "Unable to open file kValues.txt");
		printer::inst()->print_msg(L1, "Path =%s", getexepath());
		exit(1);   // call system to stop
	}
	int k = 0;
	int i = 0;
	inFile >> stats::nonceListIndex;
	while (inFile >> k && i < stats::nonceListSize) {
		nonceList.push_back(k);
		// printer::inst()->print_msg(L1, "k=%i size=%i i=%i", k, stats::nonceListSize, i);
		i++;
	}
	inFile.close();
}

void stats::writeNonceValuesToFile()
{
	std::ofstream ofile;

	printer::inst()->print_msg(L4, "Writing nonces to kValues.txt");
	ofile.open("kValues.txt");
	std::string s = std::to_string(stats::nonceListIndex) + "\n";
	ofile << s;
	for (int i = 0; i < stats::nonceList.size(); i++) {
		std::string s = std::to_string(stats::nonceList[i]) + "\n";
		ofile << s;
	}
	ofile.close();
}

std::string stats::getexepath()
{
	char result[MAX_PATH];
	return std::string(result, GetModuleFileName(NULL, result, MAX_PATH));
}

void stats::updateNonceValue() {

	std::vector<int> kList;

	printer::inst()->print_msg(L4, "updating count map");

	// initialize the full size map
	for (int i = 0; i < stats::kMapSize; i++) {
		stats::nonceCountMap[i] = 0;
	}

	for (int i = 0; i < stats::nonceList.size(); i++) {

		// get the element
		int k = stats::nonceList[i];
		auto indexIter = stats::nonceCountMap.find(k);
		if (indexIter != stats::nonceCountMap.end()) {
			stats::nonceCountMap[k]++;
		}
	}

	stats::printMap(nonceCountMap, "kValues02.txt");
}

void stats::printMap(std::multimap<int, int> map, std::string fileName) {
	// for debugging print out map
	std::ofstream ofile;
	ofile.open(fileName);
	for (std::map<int, int>::iterator it = map.begin(); it != map.end(); ++it) {
		ofile << it->first << " => " << it->second << '\n';
	}
	ofile.close();
}

void stats::printMap(std::map<int, int> map, std::string fileName) {
	// for debugging print out map
	std::ofstream ofile;
	ofile.open(fileName);
	for (std::map<int, int>::iterator it = map.begin(); it != map.end(); ++it) {
		ofile << it->first << " => " << it->second << '\n';
	}
	ofile.close();
}


void stats::transformNonceCount() {
	stats::nonceCountTrans = flip_map(stats::nonceCountMap);
	stats::printMap(stats::nonceCountTrans, "kValues03.txt");
}

void stats::sortNonceCount() {
	int i = 0;
	stats::maxNonce = 0;
	for (std::multimap<int, int>::reverse_iterator it = stats::nonceCountTrans.rbegin(); it != stats::nonceCountTrans.rend(); ++it) {
		stats::nonceSorted[i] = it->second;
		maxNonce = max(stats::maxNonce, it->second);
		i++;
	}
	stats::printMap(stats::nonceSorted, "kValues04.txt");
}

int stats::getNonce(int i)
{
	if (i < stats::nonceSorted.size()) {
		return nonceSorted[i];
	}
	else {
		return i;
	}
}

int stats::getSequentialNonce() {
	lastNonce++;
	return lastNonce;
}

int stats::getNonceFromList() {
	int nonce = 0;
	if (stats::nonceIndex < stats::nonceList.size()) {
		nonce = stats::nonceList[stats::nonceIndex];
		stats::nonceIndex++;
	}
	// printer::inst()->print_msg(L4, "Index = %i   Nonce = %i", stats::nonceIndex, nonce);
	return nonce;
}

int stats::getRandomNonce() {
	int nonce = 0;
	if (stats::nonceIndex < stats::randomMap.size()) {
		nonce = stats::randomMap[stats::nonceIndex];
		
	}
	else {
		nonce = stats::nonceIndex;
	}
	stats::nonceIndex++;
	return nonce;
}

int stats::getStatisticalNonce() {
	int nonce = stats::getNonce(stats::nonceIndex);
	stats::nonceIndex++;
	return nonce;
}

int stats::getNonceFromAlgo() {
		int i, j, c, n;
		int l = stats::lastNonce++;
		stats::nonceIndex++;
		for (i = l+1; 1; i++) {
			c=0;
			for (j = 2; j < i; j++) {
				if (i%j==0) {
					c++;
					break;  
				}
			}
			if (c == 0) {
				// printer::inst()->print_msg(L4, "Index = %i   Nonce = %i", stats::nonceIndex, i);
				stats::lastNonce = i;
				return i;
				
			}
		}
}


int stats::getNonce() {
	stats::mtx.lock();
	int nonce = stats::getNonceFromAlgo();
	stats::mtx.unlock();
	return nonce;
}

void stats::resetNonceStats() {
	stats::mtx.lock();
	if (stats::nonceIndex != 0) {
		printer::inst()->print_msg(L4, "Block: nonceIndex=%i   lastNonce=%i", stats::nonceIndex, stats::lastNonce);
		stats::targetSum = 0;
		stats::difficultyCounter = 0;
		stats::difficultyLow = UINT64_MAX;
		stats::difficultyLowSum = 0;
		stats::nonceIndex = 0;
		stats::lastNonce = 0;


		stats::writeNonceValuesToFile();
		stats::reBuildStats();
	}
	stats::mtx.unlock();
}

/*
* hashcount since last good nonce
*/
void stats::addtoNonceList(int k, int hashCount)
{
	stats::mtx.lock();
	if (stats::nonceListIndex >= stats::nonceListSize) {
		stats::nonceListIndex = 0;
	}

	if (stats::nonceList.size() == stats::nonceListSize) {
		stats::nonceList[stats::nonceListIndex] = k;
	}
	else {
		stats::nonceList.push_back(k);
	}
	printer::inst()->print_msg(L4, "Updating goodNonceList[%i] = %i", stats::nonceListIndex, k);


	stats::nonceListIndex++;
	if (nonceListIndex % 8 == 0) {

	}
	stats::totalHashCount += hashCount;
	stats::goodNonceCount++;
	printer::inst()->print_msg(L4, "Good nonces count         = %i", goodNonceCount);
	printer::inst()->print_msg(L4, "Total hash count          = %i", stats::totalHashCount);
	printer::inst()->print_msg(L4, "Hashes per good Nonce     = %i", stats::totalHashCount / goodNonceCount);
	stats::mtx.unlock();
}

void stats::fillRandomMap()
{
	std::random_device rd;  //Will be used to obtain a seed for the random number engine
	std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
	std::uniform_real_distribution<> dis(0.0, stats::randomSize);
	for (int n = 0; n < stats::randomSize; ++n) {
		int rand = round(dis(gen));
		auto exists = stats::randomMap.find(rand);
		while (exists == stats::nonceCountMap.end() ) {
			rand = round(dis(gen));
			exists = stats::randomMap.find(rand);
		}
		stats::randomMap[n] = rand;
		// printer::inst()->print_msg(L4, "Updating randomMap[%i] = %i", n, rand);
	}
}

void stats::readAndBuildStats()
{
	stats::nonceIndex = 0;
	// stats::fillRandomMap();
	stats::readNoncesFromFile();
	reBuildStats();
}

void stats::reBuildStats()
{
	stats::updateNonceValue();
	stats::transformNonceCount();
	stats::sortNonceCount();
}

void stats::difficultyStats(uint64_t target, uint64_t hash) {
	
	unsigned long long t = target;
	unsigned long long h = hash;
	stats::mtx.lock();

	unsigned long long n = difficultyCounter;
	unsigned long long t1 = target * 100 / hash;
	unsigned long long ts = targetSum; 
	unsigned long long ta;
	unsigned long long tl1 = difficultyLow;
	unsigned long long tls = difficultyLowSum;
	unsigned long long tla;
	

	// normalize hashes
	n = n + 1;
	ts = ts + t1;
	ta = t1 / n;
	tl1 = max(tl1, t1);
	tls = tls + tl1;
	tla = tls / n;


	difficultyCounter = n;
	targetSum = t;
	difficultyLow = tl1;
	difficultyLowSum = tls;

	std::string format = "Difficulty target=%llu,hash0=%llu,counter=%llu,";
	printer::inst()->print_msg(L4, format.c_str(), t, h, n);

	format = "Difficulty targetOverHash=%llu,targetOverHashLow=%llu,tovAverage=%llu,tovLowAverage=%llu";
	printer::inst()->print_msg(L4, format.c_str(), t1, tl1, ta, tla);

	stats::mtx.unlock();

}




