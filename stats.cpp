#include "stats.h"
#include "console.h"
#include <cstring>
#include <string>
#include <fstream>
#include <windows.h>
#include <algorithm>
#include <iterator>

std::vector<int> stats::kList;
std::map<int, int> stats::kCount;
std::multimap<int, int> stats::kCountTrans;
std::map<int, int> stats::kSorted;
int stats::maxNonce;
int stats::nonceIndex;
int stats::kListIndex = 0;
std::mutex stats::mtx;

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
void stats::readKValuesFromFile()
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
	inFile >> stats::kListIndex;
	while (inFile >> k && i<stats::kListSize) {
		kList.push_back(k);
		// stats::addtoKList(k);
		i++;
	}
	inFile.close();
}

void stats::writeKValuesToFile()
{
	std::ofstream ofile;

	printer::inst()->print_msg(L4, "writing kList");
	ofile.open("kValues.txt");
	std::string s = std::to_string(stats::kListIndex) + "\n";
	ofile << s;
	for (int i = 0; i < stats::kList.size(); i++) {
		std::string s = std::to_string(stats::kList[i]) + "\n";
		ofile << s;
	}
	ofile.close();
}

std::string stats::getexepath()
{
	char result[MAX_PATH];
	return std::string(result, GetModuleFileName(NULL, result, MAX_PATH));
}

void stats::updateKCount() {

	std::vector<int> kList;

	printer::inst()->print_msg(L4, "updating count map");

	// initialize the full size map
	for (int i = 0; i < stats::kMapSize; i++) {
		stats::kCount[i] = 0;
	}

	for (int i = 0; i < stats::kList.size(); i++) {

		// get the element
		int k = stats::kList[i];
		auto indexIter = stats::kCount.find(k);
		if (indexIter != stats::kCount.end()) {
			stats::kCount[k]++;
		}
	}

	stats::printMap(kCount, "kValues02.txt");
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


void stats::transformKCount() {
	stats::kCountTrans = flip_map(stats::kCount);
	stats::printMap(stats::kCountTrans, "kValues03.txt");
}

void stats::sortKCount() {
	int i = 0;
	stats::maxNonce = 0;
	for (std::multimap<int, int>::reverse_iterator it = stats::kCountTrans.rbegin(); it != stats::kCountTrans.rend(); ++it) {
		stats::kSorted[i] = it->second;
		maxNonce = max(stats::maxNonce, it->second);
		i++;
	}
	stats::printMap(stats::kSorted, "kValues04.txt");
}

int stats::getNonce(int i)
{
	if (i < stats::kSorted.size()) {
		return kSorted[i];
	}
	else {
		return i;
	}
}

int stats::getNonce() {
	stats::mtx.lock();
	int nonce = stats::getNonce(stats::nonceIndex);
	stats::nonceIndex++;
	stats::mtx.unlock();
	return nonce;
}

void stats::resetNonceCounter() {
	stats::mtx.lock();
	nonceIndex = 0;
	stats::mtx.unlock();
}

void stats::addtoKList(int k)
{
	stats::mtx.lock();
	if (stats::kListIndex >= stats::kListSize) {
		stats::kListIndex = 0;
	}

	if (stats::kList.size() == stats::kListSize) {
		stats::kList[stats::kListIndex] = k;
		printer::inst()->print_msg(L4, "Updating kList[%i] = %i", stats::kListIndex, k);
	}
	else {
		stats::kList.push_back(k);
		printer::inst()->print_msg(L4, "Updating kList[%i] = %i", stats::kListIndex, k);
	}
	stats::kListIndex++;
	if (kListIndex % 8 == 0) {
		stats::writeKValuesToFile();
		stats::reBuildStats();
	}
	stats::mtx.unlock();
}

void stats::readAndBuildStats()
{
	stats::nonceIndex = 0;
	stats::readKValuesFromFile();
	stats::updateKCount();
	stats::transformKCount();
	stats::sortKCount();
}

void stats::reBuildStats() 
{
	stats::updateKCount();
	stats::transformKCount();
	stats::sortKCount();
}



