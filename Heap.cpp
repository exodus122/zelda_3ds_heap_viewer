#include "Heap.h"

// Convert a memory address to a hex string. Can specify the number of significant figures
std::string hex2(u64 address, int numChars = 0) {
    std::stringstream ss;
    ss << std::setw(numChars) << std::setfill('0') << std::hex << address;
    std::string address_str = ss.str();
    for (auto& c : address_str) c = toupper(c);

    return address_str;
}

// Search for two strings in memory, separated by specified amount
char* GetAddressOfData(DWORD pid, const char* data, size_t len, const char* data2, size_t len2, size_t distance)
{
    
    HANDLE process = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, pid);
    if (process)
    {
        SYSTEM_INFO si;
        GetSystemInfo(&si);

        MEMORY_BASIC_INFORMATION info;
        std::vector<char> chunk;
        char* p = (char*)distance;
        while (p < si.lpMaximumApplicationAddress)
        {
            if (VirtualQueryEx(process, p, &info, sizeof(info)) == sizeof(info))
            {
                p = (char*)info.BaseAddress;
                chunk.resize(info.RegionSize);
                SIZE_T bytesRead;
                if (ReadProcessMemory(process, p, &chunk[0], info.RegionSize, &bytesRead))
                {
                    for (size_t i = 16; i < (bytesRead - len); ++i)
                    {
                        if (memcmp(data, &chunk[i], len) == 0)
                        {
                            if (memcmp(data2, (&chunk[i]) - distance, len2) == 0)
                            {
                                return (char*)p + i - distance;
                            }
                        }
                    }
                }
                p += info.RegionSize;
            }
        }
    }
    return 0;
}

// Constructor. Initialize the empty heap.
Heap::Heap(std::string windowName, int game) {
    init(windowName, game);
}

// Destructor
Heap::~Heap() {
    CloseHandle(this->pHandle);
    heapNodes.clear();
}

void Heap::init(std::string windowName, int game) {
    this->pHandle = NULL;
    this->startAddress = NULL;
    this->currAddress = NULL;
    this->playerAddress = NULL;
    this->citraOffset = NULL;
    this->game = game;

    try
    {
        // Get processHandle and baseAddress
        HWND hWnd = FindWindowA(0, windowName.c_str()); // "Citra | Ocarina of Time 3D"
        if (!hWnd) {
            return;
            throw("Emulator window not found");
        }

        DWORD pid;
        GetWindowThreadProcessId(hWnd, &pid);

        this->pHandle = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, pid);
        if (this->pHandle == NULL)
            throw("Could not create process handle");

        // Determine the start of the FCRAM region
        char pattern1[8] = {}; // game id
        char pattern2[8] = {}; // unknown

        if (this->game == GAME_OOT3D) {
            pattern1[0] = '\x00';
            pattern1[1] = '\x35';
            pattern1[2] = '\x03';
            pattern1[3] = '\x00';
            pattern1[4] = '\x00';
            pattern1[5] = '\x00';
            pattern1[6] = '\x04';
            pattern1[7] = '\x00'; // game id of OoT 3D
            pattern2[0] = '\x61';
            pattern2[1] = '\x00';
            pattern2[2] = '\x00';
            pattern2[3] = '\x00';
            pattern2[4] = '\x00';
            pattern2[5] = '\x00';
            pattern2[6] = '\x00';
            pattern2[7] = '\x00'; // unknown
        }
        else if (this->game == GAME_MM3D) {
            pattern1[0] = '\x00';
            pattern1[1] = '\x55';
            pattern1[2] = '\x12';
            pattern1[3] = '\x00';
            pattern1[4] = '\x00';
            pattern1[5] = '\x00';
            pattern1[6] = '\x04';
            pattern1[7] = '\x00'; // game id of MM 3D
            pattern2[0] = '\x61';
            pattern2[1] = '\x00';
            pattern2[2] = '\x00';
            pattern2[3] = '\x00';
            pattern2[4] = '\x00';
            pattern2[5] = '\x00';
            pattern2[6] = '\x00';
            pattern2[7] = '\x00'; // unknown
        }
        else {
            throw("Invalid game");
        }

        u64 cheat_engine_ptr = this->fcramAddr = (u64)GetAddressOfData(pid, pattern1, sizeof(pattern1), pattern2, sizeof(pattern2), 0x10) - 0x58;
        /*std::cout << "Address of Data: " << hex2(this->fcramAddr, 8) << std::endl;*/

        ReadProcessMemory(pHandle, (LPVOID)(this->fcramAddr), &this->fcramAddr, sizeof(this->fcramAddr), 0); // Get Ptr to Ptr to FCRAM start
        /*std::cout << "Ptr to Ptr to FCRAM start: " << hex2(this->fcramAddr, 8) << std::endl;*/
        
        ReadProcessMemory(pHandle, (LPVOID)(this->fcramAddr), &this->fcramAddr, sizeof(this->fcramAddr), 0); // Get Ptr to FCRAM start
        /*std::cout << "Ptr to FCRAM start: " << hex2(this->fcramAddr, 8) << std::endl;*/
        
        ReadProcessMemory(pHandle, (LPVOID)(this->fcramAddr), &this->fcramAddr, sizeof(this->fcramAddr), 0); // Get FCRAM start
        if (!this->fcramAddr)
            throw("Could not find FCRAM start address");

        /*std::cout << "FCRAM start: " << hex2(this->fcramAddr, 8) << std::endl;
        system("pause");*/

        // Determine the start of the actor heap
        this->currAddress = this->startAddress = this->fcramAddr + FCRAM_HEAP_OFFSET[this->game];
        if (!this->startAddress)
            throw("Could not find start address of heap");

        //std::cout << "Heap start address: " << hex2(this->startAddress, 8) << '\n';
        //system("pause");

        this->citraOffset = this->startAddress - HEAP_3DS_ADDRESS[this->game];
        if (!this->citraOffset)
            throw("Could not find Citra Offset Address");

        // Create Cheat Engine file
        if (game == GAME_OOT3D) {

            std::ifstream cheatTemplate("./cheat_engine_file_template_oot.txt");
            std::string line;
            std::ofstream output("./OoT3D - new.CT");

            if (cheatTemplate.is_open())
            {

                while (std::getline(cheatTemplate, line)) {
                    size_t index = 0;
                    while (true) {
                        std::string orig = "[[\"citra-qt.exe\"+1534DC0]]";

                        /* Locate the substring to replace. */
                        index = line.find(orig, index);
                        if (index == std::string::npos)
                            break;

                        /* Make the replacement. */
                        std::string temp_string = "[[" + hex2(cheat_engine_ptr, 8) + "]]";
                        line.replace(index, orig.length(), temp_string);

                        /* Advance index forward so the next iteration doesn't pick it up as well. */
                        index += temp_string.length();
                    }

                    output << line << std::endl;
                    //std::cout << line << std::endl;
                }
                cheatTemplate.close();
            }

            else
                std::cout << "Unable to open file";

        }
        else if (game == GAME_MM3D) {

            std::ifstream cheatTemplate("./cheat_engine_file_template_mm.txt");
            std::string line;
            std::ofstream output("./MM3D - new.CT");

            if (cheatTemplate.is_open())
            {

                while (std::getline(cheatTemplate, line)) {
                    size_t index = 0;
                    while (true) {
                        std::string orig = "[[\"citra-qt.exe\"+1534DC0]]";

                        // Locate the substring to replace. 
                        index = line.find(orig, index);
                        if (index == std::string::npos)
                            break;

                        // Make the replacement. 
                        std::string temp_string = "[[" + hex2(cheat_engine_ptr, 8) + "]]";
                        line.replace(index, orig.length(), temp_string);

                        // Advance index forward so the next iteration doesn't pick it up as well. 
                        index += temp_string.length();
                    }

                    output << line << std::endl;
                    //std::cout << line << std::endl;
                }
                cheatTemplate.close();
            }

            else
                std::cout << "Unable to open file";

        }
    }
    catch (std::string exception)
    {
        std::cout << "EXCEPTION: Failed to build heap - " << exception << '\n';
    }
}

// Get the difference between the Citra address and the 3DS address
u64 Heap::getCitra3dsOffset() {
    return this->citraOffset;
}

// Fill the heap with nodes
void Heap::populate() {
    this->currAddress = this->startAddress;
    heapNodes.clear();

    HeapNode node(this->currAddress, this->pHandle, this->game);
    heapNodes.push_back(node);
    //std::cout << hex2(node.getHeaderAddr() - this->citraOffset) << ":" << hex2(node.getEndAddr() - this->citraOffset) << "\n";
    //system("pause");
    //std::cout << hex2(this->currAddress) << '\n';

    if (this->game == GAME_OOT3D) {
        this->playerAddress = this->startAddress + ACTOR_HEADER_SIZE[this->game];
    }

    while (true) {
        this->currAddress = node.getNext() + this->citraOffset;

        node = HeapNode(this->currAddress, this->pHandle, this->game);
        
        if (this->game == GAME_OOT3D) {
            heapNodes.push_back(node);
            //std::cout << hex2(node.getHeaderAddr()/*- this->citraOffset*/) << ":" << hex2(node.getEndAddr()/*- this->citraOffset*/) << "\n";
            //system("pause");
            if (node.getNext() == NULL)
                break;
        }
        if (this->game == GAME_MM3D) {
            if (this->currAddress == this->startAddress) // stop when reach the beginning of the circular heap a second time
                break;

            heapNodes.push_back(node);

            if (node.isThePlayer()) {
                this->playerAddress = node.getDataAddr();
                //std::cout << "found player: " << hex2(this->playerAddress) << "\n";
                //system("pause");
            }

            if (node.getBlockSize() == MM3D_GLOBAL_CONTEXT_SIZE) {
                this->globalContextAddress = node.getDataAddr();
            }
        }
        //std::cout << hex2(this->currAddress) << " = " << hex2(this->startAddress) << "?\n";
    }

    //system("pause");
}

// Print
std::string Heap::print(bool showLinks, bool addrCitra, bool showSize, bool other, bool combine, bool coords, 
    bool names, bool unloaded, bool info) {

    this->currAddress = this->startAddress;
    std::string output = "";

    for (size_t i = 0; i < heapNodes.size(); i++) {
        if (combine && game == GAME_MM3D && i != 0 && !heapNodes[i].isFree() && !heapNodes[i].isAnActor() 
            && (!heapNodes[i - 1].isAnActor() && !heapNodes[i - 1].isFree()))
            continue;

        if (showLinks) {
            std::cout << heapNodes[i].describeHeader(addrCitra, showSize, this->citraOffset, other) << std::endl;
        }
        std::string dataDescription = heapNodes[i].describeData(getHandle(), this->currAddress, addrCitra, 
            showSize, this->citraOffset, other, combine, coords, this->fcramAddr, names, unloaded, info);
        if (dataDescription != "") {
            std::cout << dataDescription << std::endl;
            output = output + dataDescription + "\n";
        }
        if (game == GAME_OOT3D && heapNodes[i].getNext() == 0)
            break;
    }

    return output;
}

// Accessors
HANDLE Heap::getHandle() {
    return this->pHandle;
}
u64 Heap::getStartAddress() {
    return this->startAddress;
}
u64 Heap::getCurrAddress() {
    return this->currAddress;
}
u64 Heap::getPlayerAddress() {
    return this->playerAddress;
}
u64 Heap::getGlobalContextAddress() {
    return this->globalContextAddress;
}
u64 Heap::getFcramAddress() {
    return this->fcramAddr;
}

