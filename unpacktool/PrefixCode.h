#pragma once

#include "CSInputBuffer.h"

#include <vector>

typedef struct XADCodeTreeNode XADCodeTreeNode;
typedef struct XADCodeTableEntry XADCodeTableEntry;

class XADPrefixCode final
{
public:
	XADPrefixCode();
	~XADPrefixCode();

	void startBuildingTree();
	void startZeroBranch();
	void startOneBranch();
	void finishBranches();
	void makeLeafWithValue(int value);


	std::vector<XADCodeTreeNode> tree;
	int numentries, minlength, maxlength;
	bool isstatic;

	int currnode;
	std::vector<int> stack;

	int tablesize;
	XADCodeTableEntry *table1, *table2;

	static XADPrefixCode *prefixCode();
	static XADPrefixCode *prefixCodeWithLengths(const int *lengths, int numsymbols, int maxlength, bool zeros);

	void init();
	bool initWithLengths(const int *lengths, int numSymbols, int maxlength, bool zeros);

	bool addValueHighBitFirst(int value, uint32_t code, int length);
	bool addValueHighBitFirst(int value, uint32_t code, int length, int repeatpos);
	bool addValueLowBitFirst(int value, uint32_t code, int length);
	bool addValueLowBitFirst(int value, uint32_t code, int length, int repeatpos);

	void _pushNode();
	void _popNode();

	void _makeTable();
	void _makeTableLE();
};

bool CSInputNextSymbolUsingCode(CSInputBuffer *buf, XADPrefixCode *code, int &outSymbol);
bool CSInputNextSymbolUsingCodeLE(CSInputBuffer *buf, XADPrefixCode *code, int &outSymbol);
