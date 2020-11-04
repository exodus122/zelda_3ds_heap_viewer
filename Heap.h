#ifndef HEAP_H
#define HEAP_H

#include "HeapNode.h"

const u64 FCRAM_HEAP_OFFSET[] = { 0x06FF4000, 0x05B22000 };
const u64 HEAP_3DS_ADDRESS[] = { 0x098F4000, 0x08010000 };

class Heap
{
public:
	Heap(std::string windowName, int game);
	~Heap();

	void init(std::string windowName, int game);
	u64 getCitra3dsOffset();
	void populate();
	std::string print(bool showLinks, bool addrCitra, bool showSize, bool other, bool combine, bool coords,
		bool names, bool unloaded, bool info);

	HANDLE getHandle();
	u64 getStartAddress();
	u64 getCurrAddress();
	u64 getPlayerAddress();
	u64 getGlobalContextAddress();
	u64 getFcramAddress();
	
private:
	std::vector<HeapNode> heapNodes; // vector of the heap nodes
	HANDLE pHandle; // handle for windows process
	u64 fcramAddr;
	u64 startAddress; // beginning of the heap
	u64 currAddress; // current address while going through the heap
	u64 playerAddress; // pointer to player actor instance
	u64 globalContextAddress; 
	u64 citraOffset; // the difference between the Citra Address and the 3DS address
	int game;
};

#endif
