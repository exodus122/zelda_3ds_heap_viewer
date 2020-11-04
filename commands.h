#ifndef COMMANDS_H
#define COMMANDS_H

#include <iostream>
#include <iomanip>
#include <Windows.h>
#include <string>
#include <sstream>
#include <regex>
#include <vector>

// Flags for commands
bool help = false; // print the help text, listing the commands
bool showLinks = false; // show the headers of the nodes
bool showSize = true; // show size of the node instead of the end address
bool other = true; // display things on heap that are not actors or free nodes
bool combine = true; // combine objects that are next to eachother
bool addrCitra = true; // display the citra address instead of the 3DS address
bool change_age = false; // change age on next load (OoT only)
bool coord = true; // display position coordinates for actors
bool name = true; // display internal name of actor if true, description if false
bool info = true;
bool unloaded = false; // display unloaded actors (not always accurate)
std::string next_age; // the age to switch to on next load

// Incorrect command syntax flags
bool xyzSyntax = false;
bool xSyntax = false;
bool ySyntax = false;
bool zSyntax = false;
bool entSyntax = false;

// Clears console and increases the buffer
void clearConsole() {
	COORD topLeft = { 0, 0 };
	HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleScreenBufferSize(console, { 120, 20000 });
	CONSOLE_SCREEN_BUFFER_INFO screen;
	DWORD written;

	GetConsoleScreenBufferInfo(console, &screen);
	FillConsoleOutputCharacterA(
		console, ' ', screen.dwSize.X * screen.dwSize.Y, topLeft, &written
	);
	FillConsoleOutputAttribute(
		console, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE,
		screen.dwSize.X * screen.dwSize.Y, topLeft, &written
	);
	SetConsoleCursorPosition(console, topLeft);
}

// Copy string to clipboard
void toClipboard(const std::string& s) {
	OpenClipboard(0);
	EmptyClipboard();
	HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, s.size());
	if (!hg) {
		CloseClipboard();
		return;
	}
	memcpy(GlobalLock(hg), s.c_str(), s.size());
	GlobalUnlock(hg);
	SetClipboardData(CF_TEXT, hg);
	CloseClipboard();
	GlobalFree(hg);
}

// Writes XYZ coordinates
void move_link(HANDLE pHandle, std::string input, u64 playerAddr, int game) {

	std::regex myRegex(R"(^[\s]*xyz[\s]+[(]*[\s]*((?=.)[+-]?[0-9]*(.[0-9]+)?)[\s]+((?=.)[+-]?[0-9]*(.[0-9]+)?)[\s]+((?=.)[+-]?[0-9]*(.[0-9]+)?)[\s]*[)]*[\s]*$)");
	std::smatch match;

	if (regex_search(input, match, myRegex)) {
		float newxpos = stof(match.str(1)), newypos = stof(match.str(3)), newzpos = stof(match.str(5));

		WriteProcessMemory(pHandle, (LPVOID)(playerAddr + 0x8), &newxpos, sizeof(newxpos), 0);
		WriteProcessMemory(pHandle, (LPVOID)(playerAddr + 0xC), &newypos, sizeof(newypos), 0);
		WriteProcessMemory(pHandle, (LPVOID)(playerAddr + 0x10), &newzpos, sizeof(newzpos), 0);

		if (game == GAME_OOT3D) {
			WriteProcessMemory(pHandle, (LPVOID)(playerAddr + 0x28), &newxpos, sizeof(newxpos), 0);
			WriteProcessMemory(pHandle, (LPVOID)(playerAddr + 0x2C), &newypos, sizeof(newypos), 0);
			WriteProcessMemory(pHandle, (LPVOID)(playerAddr + 0x30), &newzpos, sizeof(newzpos), 0);
		}
		else {
			WriteProcessMemory(pHandle, (LPVOID)(playerAddr + 0x24), &newxpos, sizeof(newxpos), 0);
			WriteProcessMemory(pHandle, (LPVOID)(playerAddr + 0x28), &newypos, sizeof(newypos), 0);
			WriteProcessMemory(pHandle, (LPVOID)(playerAddr + 0x2C), &newzpos, sizeof(newzpos), 0);
		}
	}
	else {
		xyzSyntax = true;
	}
}

// Writes X Coordinate
void set_X(HANDLE pHandle, std::string input, u64 playerAddr, int game) {

	std::regex myRegex(R"(^[\s]*x[\s]+((?=.)[+-]?[0-9]*(.[0-9]+)?)[\s]*$)");
	std::smatch match;

	if (regex_search(input, match, myRegex)) {
		float newxpos = stof(match.str(1));

		WriteProcessMemory(pHandle, (LPVOID)(playerAddr + 0x8), &newxpos, sizeof(newxpos), 0);
		if (game == GAME_OOT3D) {
			WriteProcessMemory(pHandle, (LPVOID)(playerAddr + 0x28), &newxpos, sizeof(newxpos), 0);
		}
		else {
			WriteProcessMemory(pHandle, (LPVOID)(playerAddr + 0x24), &newxpos, sizeof(newxpos), 0);
		}
	}
	else {
		xSyntax = true;
	}
}

// Writes Y Coordinate
void set_Y(HANDLE pHandle, std::string input, u64 playerAddr, int game) {

	std::regex myRegex(R"(^[\s]*y[\s]+((?=.)[+-]?[0-9]*(.[0-9]+)?)[\s]*$)");
	std::smatch match;

	if (regex_search(input, match, myRegex)) {
		float newypos = stof(match.str(1));

		WriteProcessMemory(pHandle, (LPVOID)(playerAddr + 0xC), &newypos, sizeof(newypos), 0);
		if (game == GAME_OOT3D) {
			WriteProcessMemory(pHandle, (LPVOID)(playerAddr + 0x2C), &newypos, sizeof(newypos), 0);
		}
		else {
			WriteProcessMemory(pHandle, (LPVOID)(playerAddr + 0x28), &newypos, sizeof(newypos), 0);
		}
	}
	else {
		ySyntax = true;
	}
}

// Writes Z Coordinates
void set_Z(HANDLE pHandle, std::string input, u64 playerAddr, int game) {

	std::regex myRegex(R"(^[\s]*z[\s]+((?=.)[+-]?[0-9]*(.[0-9]+)?)[\s]*$)");
	std::smatch match;

	if (regex_search(input, match, myRegex)) {
		float newzpos = stof(match.str(1));

		WriteProcessMemory(pHandle, (LPVOID)(playerAddr + 0x10), &newzpos, sizeof(newzpos), 0);
		if (game == GAME_OOT3D) {
			WriteProcessMemory(pHandle, (LPVOID)(playerAddr + 0x30), &newzpos, sizeof(newzpos), 0);
		}
		else {
			WriteProcessMemory(pHandle, (LPVOID)(playerAddr + 0x2C), &newzpos, sizeof(newzpos), 0);
		}
	}
	else {
		zSyntax = true;
	}
}

// Writes Entrance and Warp Activator
void load_entrance(HANDLE pHandle, std::string input, u64 fcramPtr, int game) {

	std::regex myRegex(R"(^[\s]*ent[\s]+([0-9a-fA-F]{1,4})[\s]*$)");
	std::smatch match;

	if (regex_search(input, match, myRegex)) {
		u16 newent;
		std::stringstream ss;
		ss << std::hex << match.str(1);
		ss >> newent;
		char warp_activator = 1;

		if (game == GAME_OOT3D) {
			WriteProcessMemory(pHandle, (LPVOID)(fcramPtr + 0x05E24472), &newent, sizeof(newent), 0);
			WriteProcessMemory(pHandle, (LPVOID)(fcramPtr + 0x05E2446D), &warp_activator, sizeof(warp_activator), 0);
		}
		else {
			WriteProcessMemory(pHandle, (LPVOID)(fcramPtr + 0x05B6004E), &newent, sizeof(newent), 0);
			WriteProcessMemory(pHandle, (LPVOID)(fcramPtr + 0x05B60049), &warp_activator, sizeof(warp_activator), 0);
		}
	}
	else {
		entSyntax = true;
	}
}

// Swap ages on next scene load
std::string swap_age(HANDLE pHandle, u64 fcramPtr, int game) {
	if (game == GAME_MM3D) {
		return ""; // not implemented
	}

	change_age = true;
	DWORD curr_age;
	ReadProcessMemory(pHandle, (LPVOID)(fcramPtr + 0x077C695C), &curr_age, sizeof(curr_age), 0);

	char new_age = !curr_age;

	WriteProcessMemory(pHandle, (LPVOID)(fcramPtr + 0x05E24440), &new_age, sizeof(new_age), 0);

	if (new_age == 0)
		return "Adult";
	else
		return "Child";
}

// Gain control in the cutscene
void gain_control_cutscene(HANDLE pHandle, u64 fcramPtr, int game, u64 globalContextAddr) {
	char gainControl = 3;

	if (game == GAME_OOT3D) {
		WriteProcessMemory(pHandle, (LPVOID)(fcramPtr + 0x05E20AE0), &gainControl, sizeof(gainControl), 0);
	}
	else {
		WriteProcessMemory(pHandle, (LPVOID)(globalContextAddr + 0x268C), &gainControl, sizeof(gainControl), 0);
	}
}

#endif 
