#include "Heap.h"
#include "commands.h"
#include <regex>

void handleInput(HANDLE pHandle, u64 fcramAddr, int game, Heap heap, std::string processName, std::string & output);

int main() {
	std::string processName = "";
	//std::string processName = "Citra Nightly 1629 | Ocarina of Time 3D";

	while (true) {

		// Get Citra process name from user
		if (processName == "") {
			std::string input = "";
			std::cout << "Please enter the name of the Citra window. Examples:\nCitra Nightly 1629 | Ocarina of Time 3D\nCitra Nightly 1629 | Majora's Mask 3D\n\n";
			std::getline(std::cin, input);
			std::regex myRegex(R"(^(.*Citra.*)$)");
			std::smatch match;
			if (regex_search(input, match, myRegex)) {
				processName = match.str(1);
			}
			else {
				clearConsole();
				std::cout << "Could not find the Citra window. Try again\n";
				//system("pause");
				continue;
			}
		}

		// decide which game the heap viewer is attaching to, OoT3D or MM3D.
		int game; 
		if (processName.find("Ocarina of Time") != std::string::npos)
			game = GAME_OOT3D;
		else if (processName.find("Majora's Mask") != std::string::npos)
			game = GAME_MM3D;
		else {
			clearConsole();
			std::cout << "Could not find OoT3D or MM3D window. Try again\n";
			system("pause");
			continue;
		}

		// Create the heap
		Heap heap(processName, game);
		
		// bug: infinite loop if game is open but heap not initialized (such as before game loads, or in loading zone). need a better check here
		if (!heap.getStartAddress()) {
			clearConsole();
			std::cout << "Failed to attach to Citra Emulator. Try again\n";
			system("pause");
			continue;
		}

		heap.populate();

		// Print the heap
		clearConsole();
		std::cout << "Zelda 3DS Heap Viewer by Exodus\n\n";
		std::string heapString = heap.print(showLinks, addrCitra, showSize, other, combine, coord, name, unloaded, info);

		// Take user input
		u64 fcramPtr = NULL;
		if (game == GAME_OOT3D) {
			fcramPtr = heap.getFcramAddress();
		}
		else {
			fcramPtr = heap.getFcramAddress();
		}
		handleInput(heap.getHandle(), fcramPtr, game, heap, processName, heapString);
	}

	return 0;
}

void handleInput(HANDLE pHandle, u64 fcramPtr, int game, Heap heap, std::string processName, std::string & heapString) {
	if (help) {
		/*if (!showSize)
			std::cout << "start:end	id: category room drawn params	   name		coords\n";
		else
			std::cout << "start:size	id: category room drawn params	   name		coords\n";*/
		std::cout << "\nList of commands:\n";
		std::cout << "-\"c\"          copy heap output to clipboard\n";
		std::cout << "-\"size\"       toggle displaying the actor sizes or end addresses\n";
		std::cout << "-\"addr\"       toggle displaying the 3DS RAM address or Citra RAM address\n";
		std::cout << "-\"name\"       toggle displaying actor names or descriptions (OoT3D only)\n";
		std::cout << "-\"info\"       toggle displaying actor type, room number, drawn, and variable\n";
		std::cout << "-\"coord\"      toggle displaying actor coordinates\n";
		std::cout << "-\"unloaded\"   toggle displaying some unloaded actors (not always accurate, OoT3D only)\n";
		std::cout << "-\"links\"      toggle displaying the links between Actors\n";
		std::cout << "-\"other\"      toggle displaying \"other\" (non-actor and non-free) entries (MM3D only)\n";
		std::cout << "-\"combine\"    toggle combining \"other\" entries (MM3D only)\n";
		std::cout << "-\"xyz 0 0 0\"  move Link to the specified coordinates\n";
		std::cout << "-\"x 0\"        set Link's x coordinate\n";
		std::cout << "-\"y 0\"        set Link's y coordinate\n";
		std::cout << "-\"z 0\"        set Link's z coordinate\n";
		std::cout << "-\"ent 0000\"   load any entrance. Enter a hexadecimal number\n";
		std::cout << "-\"breakfree\"  break free of the current cutscene and gain control\n";
		std::cout << "-\"age\"        swap to the other age on next scene load (OoT3D only)\n";
		help = false;
	}
	else if (xyzSyntax) {
		std::cout << "\nInvalid xyz command. Example usage: \"xyz 0.0 0.0 0.0\"";
		xyzSyntax = false;
	}
	else if (xSyntax) {
		std::cout << "\nInvalid x command. Example usage: \"x 0.0\"";
		xSyntax = false;
	}
	else if (ySyntax) {
		std::cout << "\nInvalid y command. Example usage: \"y 0.0\"";
		ySyntax = false;
	}
	else if (zSyntax) {
		std::cout << "\nInvalid z command. Example usage: \"z 0.0\"";
		zSyntax = false;
	}
	else if (entSyntax) {
		std::cout << "\nInvalid ent command. Example usage: \"ent 0000\", where 0000 is a hexadecimal number";
		entSyntax = false;
	}
	else if (change_age) {
		std::cout << "\nSwapping age to " << next_age << " on next scene load";
		change_age = false;
	}

	std::string input = "";
	std::cout << "\nPress Enter to refresh or type a command (type 'help' for a list of commands):\n";
	std::getline(std::cin, input);

	while (input == "c") {
		toClipboard(heapString); // put the heap on the clipboard
		std::getline(std::cin, input);
	}

	// Refresh heap. This is necessary if a savestate was loaded or the game was restarted
	heap.init(processName, game);
	heap.populate();
	u64 playerAddr = heap.getPlayerAddress();
	u64 globalContextAddr = NULL; 
	
	if(game == GAME_MM3D)
		globalContextAddr = heap.getGlobalContextAddress();

	// Determine what to do with the user input
	if (input == "links") {
		showLinks = !showLinks;
	}
	else if (input == "other") {
		other = !other;
	}
	else if (input == "combine") {
		combine = !combine;
	}
	else if (input == "help") {
		help = true;
	}
	else if (input == "size") {
		showSize = !showSize;
	}
	else if (input == "addr") {
		addrCitra = !addrCitra;
	}
	else if (input == "info") {
		info = !info;
	}
	else if (input == "coords" || input == "coord") {
		coord = !coord;
	}
	else if (input == "names" || input == "name") {
		name = !name;
	}
	else if (input == "unloaded") {
		unloaded = !unloaded;
	}
	else if (input == "age") {
		next_age = swap_age(pHandle, fcramPtr, game);
	}
	else if (input == "breakfree") {
		gain_control_cutscene(pHandle, fcramPtr, game, globalContextAddr);
	}
	else if (input.find("ent") != std::string::npos) { // If ent is in input string
		load_entrance(pHandle, input, fcramPtr, game);
	}
	else if (input.find("xyz") != std::string::npos) { // If xyz is in input string
		move_link(pHandle, input, playerAddr, game);
	}
	else if (input.find("x") != std::string::npos) { // If x is in input string
		set_X(pHandle, input, playerAddr, game);
	}
	else if (input.find("y") != std::string::npos) { // If y is in input string
		set_Y(pHandle, input, playerAddr, game);
	}
	else if (input.find("z") != std::string::npos) { // If z is in input string
		set_Z(pHandle, input, playerAddr, game);
	}
}
