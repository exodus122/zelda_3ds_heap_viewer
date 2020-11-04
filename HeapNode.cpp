#include "HeapNode.h"
#include "actorList.h"

// Convert a memory address to a hex string. Can specify the number of significant figures
std::string hex(u64 address, int numChars = 0) {
	std::stringstream ss;
	ss << std::setw(numChars) << std::setfill('0') << std::hex << address;
	std::string address_str = ss.str();
	for (auto& c : address_str) c = toupper(c);

	return address_str;
}

// Converts float to string, with exactly some number of decimal places
template <typename T>
std::string to_string_with_precision(const T a_value, const int n = 6)
{
	std::ostringstream out;
	out.precision(n);
	out << std::setw(7) << std::fixed << a_value;
	return out.str();
}

// Determine if the value could be a float
bool probably_a_float(u32 val) {
	return (val == 0 or val == 0x80000000 or (val >= 0x38000000 and val <= 0x4c000000) or (val >= 0xb8000000 and val <= 0xcc000000));
}

// Constructor
HeapNode::HeapNode(u64 nodeAddr, HANDLE pHandle, int game) {
	this->game = game;
	this->headerAddr = nodeAddr;
	this->dataAddr = nodeAddr + ACTOR_HEADER_SIZE[game];
	this->isActor = false;
	this->isPlayer = false;
	checkIfActor(pHandle);

	if (this->game == GAME_OOT3D) {
		ReadProcessMemory(pHandle, (LPVOID)(nodeAddr + 0), &this->magic, sizeof(this->magic), 0);
		ReadProcessMemory(pHandle, (LPVOID)(nodeAddr + 2), &this->free, sizeof(this->free), 0);
		ReadProcessMemory(pHandle, (LPVOID)(nodeAddr + 4), &this->blockSize, sizeof(this->blockSize), 0);
		ReadProcessMemory(pHandle, (LPVOID)(nodeAddr + 8), &this->next, sizeof(this->next), 0);
		ReadProcessMemory(pHandle, (LPVOID)(nodeAddr + 12), &this->prev, sizeof(this->prev), 0);

		this->endAddr = this->dataAddr + this->blockSize;
	}
	else {
		//ReadProcessMemory(pHandle, (LPVOID)(nodeAddr + 0), &this->magic, sizeof(this->magic), 0);
		//ReadProcessMemory(pHandle, (LPVOID)(nodeAddr + 2), &this->free, sizeof(this->free), 0);
		
		ReadProcessMemory(pHandle, (LPVOID)(nodeAddr + 0x14), &this->next, sizeof(this->next), 0);
		ReadProcessMemory(pHandle, (LPVOID)(nodeAddr + 0x10), &this->prev, sizeof(this->prev), 0);
		ReadProcessMemory(pHandle, (LPVOID)(nodeAddr + 0x30), &this->blockSize, sizeof(this->blockSize), 0);
		ReadProcessMemory(pHandle, (LPVOID)(nodeAddr + 0x34), &this->nameAddr, sizeof(this->nameAddr), 0);
		//this->blockSize = (long)this->blockSize;

		if (this->blockSize < 0) {
			// negative size means not free
			this->blockSize = -1 * this->blockSize;
			this->blockSize -= ACTOR_HEADER_SIZE[game];
			this->free = 0;
		}
		else {
			this->blockSize -= ACTOR_HEADER_SIZE[game];
			this->free = 1;
		}

		this->endAddr = this->dataAddr + this->blockSize;
	}
}

// Destructor
HeapNode::~HeapNode() {

}


void HeapNode::checkIfActor(HANDLE pHandle) {
	if (this->game == GAME_OOT3D) {
		// Read some data to determine if it is an actor instance
		u32 first_u32;
		ReadProcessMemory(pHandle, (LPVOID)(this->dataAddr), &first_u32, sizeof(first_u32), 0);

		u16 first_u16;
		ReadProcessMemory(pHandle, (LPVOID)(this->dataAddr), &first_u16, sizeof(first_u16), 0);

		u32 maybe_xpos, maybe_ypos, maybe_zpos;
		ReadProcessMemory(pHandle, (LPVOID)(this->dataAddr + 0x28), &maybe_xpos, sizeof(maybe_xpos), 0);
		ReadProcessMemory(pHandle, (LPVOID)(this->dataAddr + 0x2C), &maybe_ypos, sizeof(maybe_ypos), 0);
		ReadProcessMemory(pHandle, (LPVOID)(this->dataAddr + 0x30), &maybe_zpos, sizeof(maybe_zpos), 0);

		if (probably_a_float(maybe_xpos) && probably_a_float(maybe_ypos) && probably_a_float(maybe_zpos) && first_u16 < 0x0200 && first_u32 > 0)
			this->isActor = true;
		else
			this->isActor = false;
	}
	else {
		// Read some data to determine if it is an actor instance
		/*u32 first_u32;
		ReadProcessMemory(pHandle, (LPVOID)(this->dataAddr), &first_u32, sizeof(first_u32), 0);*/

		u16 first_u16;
		ReadProcessMemory(pHandle, (LPVOID)(this->dataAddr), &first_u16, sizeof(first_u16), 0);

		/*u32 maybe_xpos, maybe_ypos, maybe_zpos;
		ReadProcessMemory(pHandle, (LPVOID)(this->dataAddr + 0x24), &maybe_xpos, sizeof(maybe_xpos), 0);
		ReadProcessMemory(pHandle, (LPVOID)(this->dataAddr + 0x28), &maybe_ypos, sizeof(maybe_ypos), 0);
		ReadProcessMemory(pHandle, (LPVOID)(this->dataAddr + 0x2C), &maybe_zpos, sizeof(maybe_zpos), 0);*/
		
		ReadProcessMemory(pHandle, (LPVOID)(this->headerAddr + 0x34), &this->nameAddr, sizeof(this->nameAddr), 0);

		if (this->nameAddr == MM3D_ACTOR_NAME) {
			this->isActor = true;
			if (first_u16 == 0x0000)
				this->isPlayer = true;
		}
		else
			this->isActor = false;
	}
	
}

// Main Functions
std::string HeapNode::describeHeader(bool addrCitra, bool showSize, u64 citra3dsOffset, bool other) {
	std::string sizeOrEnd;

	if (showSize)
		sizeOrEnd = hex(ACTOR_HEADER_SIZE[game], 6);
	else {
		if(addrCitra)
			sizeOrEnd = hex(getHeaderAddr() + ACTOR_HEADER_SIZE[game]);
		else
			sizeOrEnd = hex(getHeaderAddr() + ACTOR_HEADER_SIZE[game] - citra3dsOffset);
	}

	if (addrCitra)
		return hex(getHeaderAddr()) + ":" + sizeOrEnd + " LINK\t\t" + hex(getBlockSize()) + "\t" + std::to_string(isFree());
	else
		return hex(getHeaderAddr() - citra3dsOffset) + ":" + sizeOrEnd + "\tLINK\t\t" + hex(getBlockSize()) + "\t" + std::to_string(isFree());
}

std::string readString(HANDLE pHandle, u64 fcramAddr, u64 addr) {
	char name[200];

	ReadProcessMemory(pHandle, LPCVOID(fcramAddr + 0x076D5D18 + (addr - 0x0063DD18)), &name, 200, NULL);
	//std::cout << name << std::endl;
	//system("pause");
	return name;
}

std::string HeapNode::describeData(HANDLE pHandle, u64 nodeAddr, bool addrCitra, bool showSize, u64 citra3dsOffset,
	bool other, bool combine, bool coords, u64 fcramAddr, bool names, bool unloaded, bool info) {

	std::string sizeOrEnd;

	// If node is free
	if (this->isFree()) {

		std::string description = "";
		std::string actorNameOrDescription;

		if (game == GAME_OOT3D && this->isActor && unloaded) {
			u16 first_u16;
			ReadProcessMemory(pHandle, (LPVOID)(this->dataAddr), &first_u16, sizeof(first_u16), 0);

			if (names || game == GAME_MM3D) {
				actorNameOrDescription = actorList[game][first_u16];
			}
			else {
				actorNameOrDescription = actorList[2][first_u16];
			}

			// Get actor coordinates and string
			float xpos, ypos, zpos;
			if (game == GAME_OOT3D) {
				ReadProcessMemory(pHandle, (LPVOID)(this->dataAddr + 0x28), &xpos, sizeof(xpos), 0);
				ReadProcessMemory(pHandle, (LPVOID)(this->dataAddr + 0x2C), &ypos, sizeof(ypos), 0);
				ReadProcessMemory(pHandle, (LPVOID)(this->dataAddr + 0x30), &zpos, sizeof(zpos), 0);
			}
			else {
				ReadProcessMemory(pHandle, (LPVOID)(this->dataAddr + 0x24), &xpos, sizeof(xpos), 0);
				ReadProcessMemory(pHandle, (LPVOID)(this->dataAddr + 0x28), &ypos, sizeof(ypos), 0);
				ReadProcessMemory(pHandle, (LPVOID)(this->dataAddr + 0x2C), &zpos, sizeof(zpos), 0);
			}
			std::string coordString = "";
			if (coords) {
				coordString = "\t( " + to_string_with_precision(xpos, 1) + " " + to_string_with_precision(ypos, 1)
					+ " " + to_string_with_precision(zpos, 1) + " )";

				// Format Actor Name
				if (names || game == GAME_MM3D) {
					std::stringstream ss3;
					ss3 << std::left << std::setw(16) << std::setfill(' ') << actorNameOrDescription;
					actorNameOrDescription = ss3.str();
				}
				else {
					std::stringstream ss3;
					ss3 << std::left << std::setw(40) << std::setfill(' ') << "[" + actorNameOrDescription + "]";
					actorNameOrDescription = ss3.str();
				}
			}

			std::string tabs = "\t\t";
			if (addrCitra && !showSize)
				tabs = "\t";

			description = " [AI " + hex(first_u16, 4) + "]" + tabs + actorNameOrDescription + coordString;
		}

		if (showSize)
			sizeOrEnd = hex(getBlockSize(), 6);
		else {
			if (addrCitra)
				sizeOrEnd = hex(this->dataAddr + getBlockSize());
			else
				sizeOrEnd = hex(this->dataAddr + getBlockSize() - citra3dsOffset);
		}

		if (addrCitra)
			return hex(getDataAddr()) + ":" + sizeOrEnd + " free" + description;
		else
			if (showSize)
				return hex(getDataAddr() - citra3dsOffset) + ":" + sizeOrEnd + " free" + description;
			else
				return hex(getDataAddr() - citra3dsOffset) + ":" + sizeOrEnd + "\tfree" + description;
	}

	// Check if it's an actor instance, and if so, describe it
	else if (this->isActor) {
		u16 first_u16;
		ReadProcessMemory(pHandle, (LPVOID)(this->dataAddr), &first_u16, sizeof(first_u16), 0);

		std::string infoString = "";
		if (info) {

			u16 category; // aka type
			ReadProcessMemory(pHandle, (LPVOID)(this->dataAddr + 0x2), &category, sizeof(category), 0);
			/*
			enum ActorType_enum {
				Switch = 0,
				Background = 1,
				Player = 2,
				Bomb = 3,
				Npc = 4,
				Enemy = 5,
				Prop = 6,
				Item = 7,
				Misc = 8,
				Boss = 9,
				Door = 10,
				Chest = 11,
			};
			*/

			u16 drawn;
			if (game == GAME_OOT3D)
				ReadProcessMemory(pHandle, (LPVOID)(this->dataAddr + 0x120), &drawn, sizeof(drawn), 0);
			else
				ReadProcessMemory(pHandle, (LPVOID)(this->dataAddr + 0x124), &drawn, sizeof(drawn), 0);

			u16 variable; // not sure if correct for MM3D
			ReadProcessMemory(pHandle, (LPVOID)(this->dataAddr + 0x1C), &variable, sizeof(variable), 0);

			// Get the actor category and room number as hex strings
			std::string temp = hex(category, 4);
			std::string room_string = temp.substr(0, 2);
			std::string category_str = temp.substr(3, 4);

			// Get the actor drawn flag as hex string
			std::string drawn_str = hex(drawn, 4).substr(1, 1);

			// Get the actor variable as hex string
			std::string variable_str = hex(variable, 4);

			infoString = category_str + " " + room_string + " " + drawn_str + " " + variable_str + " \t";
		}

		// Get the actor ID as a hex string
		std::string id_str = hex(first_u16, 4);

		// Format Actor Name
		
		
		std::string actorNameOrDescription;
		if (names || game == GAME_MM3D) {
			actorNameOrDescription = actorList[this->game][first_u16];
		}
		else {
			actorNameOrDescription = actorList[2][first_u16];
		}
		
		// Get actor coordinates and string
		std::string coordString = "";
		if (coords) {
			float xpos, ypos, zpos;
			if (game == GAME_OOT3D) {
				ReadProcessMemory(pHandle, (LPVOID)(this->dataAddr + 0x28), &xpos, sizeof(xpos), 0);
				ReadProcessMemory(pHandle, (LPVOID)(this->dataAddr + 0x2C), &ypos, sizeof(ypos), 0);
				ReadProcessMemory(pHandle, (LPVOID)(this->dataAddr + 0x30), &zpos, sizeof(zpos), 0);
			}
			else {
				ReadProcessMemory(pHandle, (LPVOID)(this->dataAddr + 0x24), &xpos, sizeof(xpos), 0);
				ReadProcessMemory(pHandle, (LPVOID)(this->dataAddr + 0x28), &ypos, sizeof(ypos), 0);
				ReadProcessMemory(pHandle, (LPVOID)(this->dataAddr + 0x2C), &zpos, sizeof(zpos), 0);
			}

			coordString = "\t( " + to_string_with_precision(xpos, 1) + " " + to_string_with_precision(ypos, 1)
				+ " " + to_string_with_precision(zpos, 1) + " )";

			// Resize actor name/description
			std::stringstream ss2;
			if (names || game == GAME_MM3D) {
				ss2 << std::left << std::setw(16) << std::setfill(' ') << actorNameOrDescription;
			}
			else {
				ss2 << std::left << std::setw(40) << std::setfill(' ') << actorNameOrDescription;
			}
			actorNameOrDescription = ss2.str();
		}

		// Get Start address and End/Size strings
		std::string startString = "", endOrSizeString = "";
		if (showSize) {
			if (addrCitra) {
				startString = hex(getDataAddr());
				endOrSizeString = hex(getBlockSize(), 6);
			}
			else {
				startString = hex(getDataAddr() - citra3dsOffset);
				endOrSizeString = hex(getBlockSize(), 6);
			}
		}
		else {
			if (addrCitra) {
				startString = hex(getDataAddr());
				endOrSizeString = hex(getEndAddr());
			}
			else {
				startString = hex(getDataAddr() - citra3dsOffset);
				endOrSizeString = hex(getEndAddr() - citra3dsOffset);
			}
		}

		return startString + ":" + endOrSizeString + " AI " + id_str + ": " + infoString + actorNameOrDescription
			+ coordString;
	}

	else if (game == GAME_OOT3D || (game == GAME_MM3D && other && !combine)) {
		std::string otherString = "Other";

		if (game == GAME_MM3D && getBlockSize() == MM3D_GLOBAL_CONTEXT_SIZE)
			otherString = "Global Context";

		if (game == GAME_OOT3D && getBlockSize() == OOT3D_GET_ITEM_MODEL_SIZE)
			otherString = "Get Item Model";

		// If it wasn't an actor, call it Other
		if (showSize)
			sizeOrEnd = hex(getBlockSize(), 6);
		else {
			if (addrCitra)
				sizeOrEnd = hex(getEndAddr());
			else
				sizeOrEnd = hex(getEndAddr() - citra3dsOffset);
		}
		std::string nameString = "";

		if (game == GAME_MM3D) {
			nameString = hex(this->nameAddr);
			if (this->nameAddr > 0x00600000 && this->nameAddr < 0x00700000)
				nameString = nameString + " " + readString(pHandle, fcramAddr, this->nameAddr);
		}

		if (addrCitra)
			return hex(getDataAddr()) + ":" + sizeOrEnd + " " + otherString + " " + nameString;
		else
			return hex(getDataAddr() - citra3dsOffset) + ":" + sizeOrEnd + " " + otherString + " " + nameString;
	}
	else if (other && game == GAME_MM3D) {
		if (addrCitra) {
			if (showSize)
				return hex(getDataAddr()) + ":\tOther";
			else
				return hex(getDataAddr()) + ":\t  Other";
		}
		else
			if (showSize)
				return hex(getDataAddr() - citra3dsOffset) + ":       Other";
			else
				return hex(getDataAddr() - citra3dsOffset) + ":\tOther";
	}

	return "";
}

bool HeapNode::isFree() {
	return this->free;
}
bool HeapNode::isValid() {
	return (this->magic == 0x7373);
}

// Accessors
u64 HeapNode::getHeaderAddr() {
	return this->headerAddr;
}
u64 HeapNode::getDataAddr() {
	return this->dataAddr;
}
u64 HeapNode::getEndAddr() {
	return this->endAddr;
}

s32 HeapNode::getBlockSize() {
	return this->blockSize;
}
void HeapNode::setBlockSize(s32 blockSize) {
	this->blockSize = blockSize;
}

u64 HeapNode::getNext() {
	return this->next;
}
u64 HeapNode::getPrev() {
	return this->prev;
}
bool HeapNode::isAnActor() {
	return this->isActor;
}
bool HeapNode::isThePlayer() {
	return this->isPlayer;
}