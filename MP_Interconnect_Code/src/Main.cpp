#include <fstream>
#include <array>
#include <cstdint>
#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>

#include "../Include/MemorySave.hpp"

using namespace std;

std::array<std::array<uint8_t, 32>, 4096> shared_memory;
std::array<std::array<uint8_t, 128>, 128> pe_cache;  // Example for a single PE


/*
void saveSharedMemoryToFile() {
    std::ofstream file("MemoryAndCache/shared_memory.txt");
    if (!file.is_open()) {
        std::cerr << "Error al abrir shared_memory.txt\n";
        return;
    }

    for (size_t block = 0; block < shared_memory.size(); ++block) {
        //file << "Dirección " << (block*4) << ": ";
        for (size_t byte = 0; byte < 4; ++byte) {
            file << std::hex << static_cast<int>(shared_memory[block][byte]) << " ";
        }
        file << "\n";
    }
    file.close();
    file.close();
}

void savePECacheToFile(int pe_id) {
    std::ostringstream filename;
    filename << "MemoryAndCache/PE" << pe_id << "_cache.txt";
    std::ofstream file(filename.str());
    
    if (!file.is_open()) {
        std::cerr << "Error al abrir " << filename.str() << "\n";
        return;
    }

    for (size_t block = 0; block < pe_cache.size(); ++block) {
        //file << "Bloque " << block << ": ";
        for (size_t byte = 0; byte < 16; ++byte) {
            file << std::hex << static_cast<int>(pe_cache[block][byte]) << " ";
        }
        file << "\n";
    }
    file.close();
}

*/

void write_mem(uint8_t src, uint32_t addr, uint16_t num_of_cache_lines, uint16_t start_cache_line, uint16_t qos){
    cout << std::hex << "Entrada al write_mem con los parámetros \n src " << src*1 << "\n addr " << addr << "\n num_of_cache_lines " << num_of_cache_lines 
    << "\n start_cache_line " << start_cache_line << "\n qos " << qos << "\n\n\n\n" << endl;


    
    return;
}

void read_mem(uint8_t src, uint32_t addr, uint32_t size, uint16_t qos){
    cout << "Entrada al read_mem" << "\n\n\n\n" << endl;
    return;
}

void broadcast_invalidate(uint8_t src, uint16_t cache_line, uint16_t qos){
    cout << "Entrada al broadcast_invalidate" << "\n\n\n\n" << endl;
    return;
}

void inv_ack(uint8_t src, uint16_t qos){
    cout << "Entrada al inv_ack" << "\n\n\n\n" << endl;
}

void inv_complete(uint8_t dest, uint16_t qos){
    cout << "Entrada al inv_complete" << "\n\n\n\n" << endl;
}

void read_resp(uint8_t dest, std::array<uint32_t, 4096> data, uint16_t qos){
    cout << "Entrada al read_resp" << "\n\n\n\n" << endl;
}

void write_resp(uint8_t dest, uint8_t status, uint16_t qos){
    cout << "Entrada al write_resp" << "\n\n\n\n" << endl;
}



void instructionReader(uint8_t src){

    uint16_t qos = 0; // For testing until we implement qos.

    string filename = "../Workloads/PE" + to_string(src) + "_Instructions.txt";

    ifstream file(filename);

    if(!file.is_open()){
        cerr <<  "Error opening: " << filename << endl;
        return;
    }

    // Quitar comas si existen
    auto remove_commas = [](std::string &s) {
        s.erase(std::remove(s.begin(), s.end(), ','), s.end());
    };

    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;

        istringstream iss(line);
        string instruction;
        iss >> instruction;

        if (instruction == "WRITE_MEM") {
            string addr_str, n_lines_str, start_line_str;
            iss >> addr_str >> n_lines_str >> start_line_str;

            remove_commas(addr_str);
            remove_commas(n_lines_str);
            remove_commas(start_line_str);

            uint32_t addr = stoul(addr_str, nullptr, 0);
            uint16_t num_of_cache_lines = static_cast<uint16_t>(stoul(n_lines_str, nullptr, 0));
            uint16_t start_cache_line = static_cast<uint16_t>(stoul(start_line_str, nullptr, 0));
            
            write_mem(src, addr, num_of_cache_lines, start_cache_line, qos);
        } 
        else if (instruction == "READ_MEM") {
            string addr_str, size_str;
            iss >> addr_str >> size_str;


            remove_commas(addr_str);
            remove_commas(size_str);

            uint32_t addr = stoul(addr_str, nullptr, 0);
            uint32_t size = stoul(size_str, nullptr, 0);
            
            

            read_mem(src ,addr, size, qos);
        } 
        else if (instruction == "BROADCAST_INVALIDATE") {
            string line_str;
            iss >> line_str;

            remove_commas(line_str);

            uint16_t cache_line = static_cast<uint16_t>(stoul(line_str, nullptr, 0));
            broadcast_invalidate(src, cache_line, qos);
        } 
        else {
            cerr << "Unrecognized instruction: " << instruction << endl;
        }
    }

    file.close();
}


int main() {
    // Inicializar memoria y cachés (ejemplo)
    shared_memory[0][0] = 1;
    shared_memory[0][1] = 0;
    shared_memory[0][2] = 1;
    shared_memory[0][3] = 1;
    pe_cache[0][0] = 1;

    // Guardar en archivos
    saveSharedMemoryToFile(shared_memory);
    
    // Suponiendo que tenemos 8 PEs (PE0 a PE7)
    for (int i = 0; i < 8; ++i) {
        savePECacheToFile(i,pe_cache);
    }


    instructionReader(0);


    return 0;
}


