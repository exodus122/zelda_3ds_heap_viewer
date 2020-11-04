#ifndef HEAPNODE_H
#define HEAPNODE_H

#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <vector>
#include <Windows.h>
#include <Psapi.h>
#include <locale>
#include <codecvt>

typedef unsigned short u16;
typedef unsigned long u32;
typedef long s32;
typedef unsigned long long u64;

const int ACTOR_HEADER_SIZE[] = { 0x10, 0x40 };
const int MM3D_GLOBAL_CONTEXT_SIZE = 0x11030;
const int OOT3D_GET_ITEM_MODEL_SIZE = 0x10000;
const int MM3D_ACTOR_NAME = 0x00643A90;
enum gameEnum { GAME_OOT3D, GAME_MM3D };

class HeapNode
{
public:
	HeapNode(u64 nodeAddr, HANDLE pHandle, int game);
	~HeapNode();

	void init(u64 nodeAddr, HANDLE pHandle, int game);
	void checkIfActor(HANDLE pHandle);
	std::string describeHeader(bool addrCitra, bool showSize, u64 citra3dsOffset, bool other);
	std::string describeData(HANDLE pHandle, u64 nodeAddr, bool addrCitra, bool showSize, u64 citra3dsOffset, 
		bool other, bool combine, bool coords, u64 fcramAddr, bool names, bool unloaded, bool info);

	u64 getHeaderAddr();
	u64 getDataAddr();
	u64 getEndAddr();

	bool isValid();
	bool isFree();
	s32 getBlockSize();
	void setBlockSize(s32 blockSize);
	u64 getNext();
	u64 getPrev();
	bool isAnActor();
	bool isThePlayer();

private:
	u64 headerAddr;
	u64 dataAddr;
	u64 endAddr;

	u16 magic; // the magic number at the start of a heap node, 0x7373
	u16 free; // 1 = free?, 0 = not free?
	s32 blockSize;
	u32 next;
	u32 prev;
	u32 nameAddr; // filename pointer. mm3d only
	int game;
	bool isActor;
	bool isPlayer;
};

#endif
