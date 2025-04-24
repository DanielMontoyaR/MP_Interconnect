#include <fstream>
#include <array>
#include <cstdint>
#include <iostream>
#include <string>
#include <sstream>

#include "../Include/MemorySave.hpp"




void saveSharedMemoryToFile(const std::array<std::array<uint8_t, 32>, 4096>& shared_memory) {
    std::ofstream file("../MemoryAndCache/shared_memory.txt");
    //std::ofstream file("shared_memory.txt");
    if (!file.is_open()) {
        std::cerr << "Error al abrir shared_memory.txt\n";
        return;
    }
    int bits = 0;
    for (size_t block = 0; block < shared_memory.size(); ++block) {
        //file << "Dirección " << (block*4) << ": ";
        for (size_t byte = 0; byte < shared_memory[0].size(); ++byte) {
            file << std::hex << static_cast<int>(shared_memory[block][byte]);
            ++bits;
            if(bits == 8){
                file << " ";
                bits = 0;
            }
        }
        file << "\n";
    }
    file.close();
    file.close();
}

void savePECacheToFile(int pe_id, const std::array<std::array<uint8_t, 128>, 128>& pe_cache) {
    std::ostringstream filename;
    filename << "../MemoryAndCache/PE" << pe_id << "_cache.txt";
    std::ofstream file(filename.str());
    
    if (!file.is_open()) {
        std::cerr << "Error al abrir " << filename.str() << "\n";
        return;
    }
    int bits = 0;
    for (size_t block = 0; block < pe_cache.size(); ++block) {
        //file << "Bloque " << block << ": ";
        for (size_t byte = 0; byte < pe_cache[0].size(); ++byte) {
            file << std::hex << static_cast<int>(pe_cache[block][byte]);
            ++bits;
            if(bits == 8){
                file << " ";
                bits = 0;
            }
        }
        file << "\n";
    }
    file.close();
}