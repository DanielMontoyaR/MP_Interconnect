#include <fstream>
#include <array>
#include <cstdint>
#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <mutex>
#include <cmath>


#include "../Include/MemorySave.hpp"

using namespace std;

std::array<std::array<uint8_t, 32>, 4096> shared_memory;

using PECache =  std::array<std::array<uint8_t, 128>, 128>;

PECache pe0_cache;
PECache pe1_cache;
PECache pe2_cache;
PECache pe3_cache;
PECache pe4_cache;
PECache pe5_cache;
PECache pe6_cache;
PECache pe7_cache;

PECache* pe_caches[] = {&pe0_cache, &pe1_cache, &pe2_cache, &pe3_cache, &pe4_cache,&pe5_cache, &pe6_cache, &pe7_cache};

std::mutex shared_memory_mtx;

std::mutex pe0_cache_mtx;
std::mutex pe1_cache_mtx;
std::mutex pe2_cache_mtx;
std::mutex pe3_cache_mtx;
std::mutex pe4_cache_mtx;
std::mutex pe5_cache_mtx;
std::mutex pe6_cache_mtx;
std::mutex pe7_cache_mtx;

std::mutex* pe_cache_mutexes[] = {&pe0_cache_mtx, &pe1_cache_mtx, &pe2_cache_mtx, &pe3_cache_mtx, &pe4_cache_mtx, &pe5_cache_mtx, &pe6_cache_mtx, &pe7_cache_mtx};


void write_mem(uint8_t src, uint32_t addr, uint16_t num_of_cache_lines, uint16_t start_cache_line, uint16_t qos){
    //cout << std::hex << "Entrada al write_mem con los parámetros \n src " << src*1 << "\n addr " << addr << "\n num_of_cache_lines " << num_of_cache_lines 
    //<< "\n start_cache_line " << start_cache_line << "\n qos " << qos << "\n\n\n\n" << endl;
    //int memory_adress = int(addr);
    //cout << "La dirección es: " << memory_adress << endl;

    int shared_row = addr / 32;
    int shared_col = addr % 32;

    cout << "El valor de fila y columna con la dirección " << std::hex << addr << " son: \nfila: "<< std::dec << shared_row << "\ncolumna: " << shared_col << endl << endl;
    
    //std::lock(shared_memory_mtx,pe0_cache_mtx,pe1_cache_mtx);

    shared_memory_mtx.lock();

    //int cache_col = 1;

    for(int cache_row = 0; cache_row < num_of_cache_lines; cache_row++){ //Loop for traverse the number of cache lines

        if(pe_caches[src]->at(start_cache_line).at(0) == 1){
            cout << "Error, line " << start_cache_line << " of PE "<< src*1 <<" is invalid" << endl; //We evaluate the invalidation bit
            start_cache_line++;
        }
        else{
            
            for(int cache_col=1; cache_col<(*pe_caches[src])[0].size(); cache_col++){
                //cout << "Wrote on line blah blah this " << pe_caches[src]->at(start_cache_line).at(cache_col)*1 << endl;
                //shared_memory[shared_row][shared_col] = pe_caches[src]->at(start_cache_line).at(cache_col);
                shared_memory[shared_row][shared_col] = (*pe_caches[src])[start_cache_line][cache_col];//This one does the same as the first one
                shared_col++;

                if(shared_col==shared_memory[0].size()){
                    
                    shared_col=0;
                    shared_row++;
                }
            }
            start_cache_line++;
        }
    }

    shared_memory_mtx.unlock();


    return;
}








void read_mem(uint8_t src, uint32_t addr, uint32_t size, uint16_t qos){
    cout << "Entrada al read_mem" << "\n\n\n\n" << endl;
    //src = src*1;
    
    int shared_row = addr / 32;
    int shared_col = addr % 32;

    int cache_row = addr / 128;
    //int cache_row = std::floor(addr/128);
    int cache_col = addr % 128;

    cout << "Cache row value is: " << cache_row << endl;

    cout << "Shared row value is: " << shared_row << endl; 

    int byte_to_bit = int(size)*8;



    cout << "The size in bytes is: " << std::hex << size << ". Its equivalent in decimal is: " << std::dec << byte_to_bit << endl;

    std::lock(shared_memory_mtx,(*pe_cache_mutexes[src]));

    if(cache_col == 0){ //Cache col 0 is always used for the invalidation bit
        cache_col = 1; 
    }

    for(int bits = 0; bits < byte_to_bit; bits++){


        (*pe_caches[src])[cache_row][cache_col] = shared_memory[shared_row][shared_col];

        shared_col++;
        cache_col++;

        if(shared_col = shared_memory[0].size()){//End shared memory columns

            shared_row++;
            shared_col=0;

        }
        else if (cache_col == (*pe_caches[src])[0].size()){// End cache memory columns

            cache_row++;
            cache_col=1;

        }
        
        

    }


    shared_memory_mtx.unlock();
    (*pe_cache_mutexes[src]).unlock();

    return;
}

void broadcast_invalidate(uint8_t src, uint16_t cache_line, uint16_t qos){
    //cout << "Entrada al broadcast_invalidate" << "\n\n\n\n" << endl;

    std::lock(pe0_cache_mtx, pe1_cache_mtx, pe2_cache_mtx, pe3_cache_mtx, pe4_cache_mtx, pe5_cache_mtx, pe6_cache_mtx, pe7_cache_mtx);

    for(int PEs = 0; PEs < 8; PEs++){

        pe_caches[PEs]->at(cache_line-1).at(0) = 1;

    }

    pe0_cache_mtx.unlock();
    pe1_cache_mtx.unlock();
    pe2_cache_mtx.unlock();
    pe3_cache_mtx.unlock();
    pe4_cache_mtx.unlock();
    pe5_cache_mtx.unlock();
    pe6_cache_mtx.unlock();
    pe7_cache_mtx.unlock();

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

    //First bit of each pe_cache is the invalidation bit
    shared_memory[0][0] = 1;
    shared_memory[0][1] = 0;
    shared_memory[0][2] = 1;
    shared_memory[0][3] = 1;
    pe0_cache[0][0] = 0; //Here we disable the first line.
    pe0_cache[1][1] = 0; //Here we don't disable the line

    for(int row=0; row<pe0_cache.size();row++){
        for(int col=1; col<pe0_cache[0].size();col++){
            pe0_cache[row][col] = 5;
        }
    }

    pe0_cache[0][0]= 1;
    pe0_cache[1][0]= 0;




    //Here we use threads (of for each PE)
    instructionReader(uint8_t(0));

    //cout << "El tamaño del cache es: " << pe0_cache[0].size() <<endl;
    //pe0_cache[0][pe0_cache[0].size()] = 7;

    // Guardar en archivos
    saveSharedMemoryToFile(shared_memory);

    // Suponiendo que tenemos 8 PEs (PE0 a PE7)
    for (int i = 0; i < 8; ++i) {
        savePECacheToFile(i,*pe_caches[i]);
    }

    return 0;
}


