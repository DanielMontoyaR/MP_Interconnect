#include <fstream>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <mutex>
#include <iomanip>  
#include <cmath>
#include <thread>
#include <chrono>
#include <unordered_map>
#include <vector>
#include <queue>
#include <condition_variable>
#include <atomic>
#include <functional>
#include <compare>




#include "../Include/MemorySave.hpp"

using namespace std;

std::array<std::array<uint8_t, 32>, 4096> shared_memory;

using PECache =  std::array<std::array<uint8_t, 128>, 128>;

int stepping = 0;

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

enum class InstructionType {
    WRITE,
    READ,
    BROADCAST_INVALIDATE
};

struct Instruction {
    InstructionType type;
    uint8_t src;
    uint32_t addr;
    uint16_t qos;

    // Para WRITE_MEM
    uint16_t num_of_cache_lines = 0;
    uint16_t start_cache_line = 0;

    // Para READ_MEM
    uint32_t size = 0;

    // Para BROADCAST_INVALIDATE
    uint16_t cache_line = 0;
};

std::queue<Instruction> instruction_queue;
std::mutex queue_mutex;
std::condition_variable queue_cv;

struct QoSComparator {
    bool operator()(const Instruction& a, const Instruction& b) {
        return a.qos < b.qos; // Mayor QoS = mayor prioridad
    }
};

std::priority_queue<Instruction, std::vector<Instruction>, QoSComparator> qos_queue;


enum class MemOpType { READ, WRITE, BROADCAST_INVALIDATE };

struct MemRequest {
    MemOpType op_type;
    uint8_t src;
    uint32_t addr;
    uint32_t size; // size or num_of_cache_lines depending on op_type
    uint16_t start_cache_line; // only for write
    uint16_t cache_line;
    uint16_t qos;
};





bool all_instructions_loaded = false;


class SchedulerWrapper {
private:
    queue<MemRequest> fifo_queue;
    priority_queue<MemRequest, vector<MemRequest>, function<bool(const MemRequest&, const MemRequest&)>> qos_queue;
    bool use_fifo;
    mutex queue_mutex;
    condition_variable cv;

public:
    SchedulerWrapper(bool use_fifo_policy)
        : use_fifo(use_fifo_policy),
          qos_queue([](const MemRequest& a, const MemRequest& b) {
              return a.qos < b.qos; // Higher QoS first
          }) {}

    void submit_request(const MemRequest& req) {
        unique_lock<mutex> lock(queue_mutex);
        cout << "[ENQUEUED] PE" << unsigned(req.src) 
         << " QoS=0x" << hex << setw(2) << setfill('0') << req.qos << endl;
        if (use_fifo)
            fifo_queue.push(req);
        else
            qos_queue.push(req);
        cv.notify_one();
    }

    bool has_request() {
        return !fifo_queue.empty() || !qos_queue.empty();
    }

    MemRequest get_next_request() {
        unique_lock<mutex> lock(queue_mutex);
        cv.wait(lock, [&] { return has_request(); });
        MemRequest req = use_fifo ? fifo_queue.front() : qos_queue.top();
        if (use_fifo)
            fifo_queue.pop();
        else
            qos_queue.pop();
        return req;
    }
};

SchedulerWrapper scheduler(/*use_fifo_policy=*/false); // Cambia a false si quieres usar QoS
mutex mem_mutex;





struct BandwidthStats {
    int bytes_transfered = 0;
    double time_spent = 0.0;
    double shared_memory_access_time = 0.0;
};

std::unordered_map<uint8_t, BandwidthStats> pe_bandwidth; //Bandwidth map for each PE





void save_bandwidth_stats() {
    for (const auto& [pe_id, stats] : pe_bandwidth) {
        std::string filename = "../PE_Bandwidth/PE" + std::to_string(pe_id) + "_bandwidth.txt";
        std::ofstream file(filename);

        if (file.is_open()) {
            double bandwidth = (stats.time_spent > 0) 
                ? stats.bytes_transfered / stats.time_spent 
                : 0.0;

            file << "PE " << unsigned(pe_id) << " Bandwidth Stats:\n";
            file << "Total Bytes Transferred: " << stats.bytes_transfered << "\n";
            file << "Total Time Spent (s): " << stats.time_spent << "\n";
            file << "Bandwidth (Bytes/s): " << bandwidth << "\n";
            file << "Shared Memory Access Time: " << stats.shared_memory_access_time << "\n";
        }
    }
}


void PE_logs(int num, const std::string& message) {
    std::ofstream log_file;

    // Crear nombre de archivo: PE{num}_logs.txt
    std::string filename = "../PE_logs/PE" + std::to_string(num) + "_logs.txt";


    if (message == "666") {
        // Truncar el archivo (borrar su contenido)
        std::ofstream log_file(filename, std::ios::trunc);
        if (log_file.is_open()) {
            log_file.close();
            //std::cout << "Archivo " << filename << " borrado (contenido eliminado)." << std::endl;
        } else {
            std::cerr << "No se pudo borrar el archivo: " << filename << std::endl;
        }
        return;
    }


    // Abrir en modo append
    log_file.open(filename, std::ios::app);
    
    if (log_file.is_open()) {
        // Escribir en el archivo con el formato solicitado
        log_file << "[PE" << num << "] → " << message << std::endl;
        log_file.close();
    } else {
        std::cerr << "Error al abrir el archivo de log: " << filename << std::endl;
    }
}






void stepping_wait(int id) {
    std::string nombre_archivo = "../Inputs/PE" + std::to_string(id) + "_Input";
    std::string linea;

    std::cout << "PE " << id << " Waiting '1' in " << nombre_archivo << "...\n";

    while (true) {
        std::ifstream archivo(nombre_archivo);
        if (archivo.is_open()) {
            std::getline(archivo, linea);
            archivo.close();

            if (linea == "1") {
                // Reseteamos a "0"
                std::ofstream archivo_out(nombre_archivo);
                archivo_out << "0\n";
                archivo_out.close();
                break;
            }
        }
        //std::this_thread::sleep_for(std::chrono::milliseconds(200)); // Espera para no sobrecargar el CPU
    }

    std::cout << "Hilo " << id << " reanudado\n";
}





void write_resp(uint8_t dest, uint8_t status, uint16_t qos){
    //cout << "[PE"<< dest*1 << "] → WRITE_RESP: Process Initiated" << endl;
    string log = "";
    if(status == 1){

        //cout << "[PE"<< dest*1 << "] → WRITE_RESP: Write Operation Successful.  System exit with STATUS = " << status*1 << endl;
        log = "WRITE_RESP: Write Operation Successful.  System exit with STATUS = " + to_string(status*1);

    }
    else{

        //cout << "[PE"<< dest*1 << "] → WRITE_RESP: Write Operation Unsuccessful. System exit with STATUS = " << status*1 << endl;
        log = "WRITE_RESP: Write Operation Unsuccessful.  System exit with STATUS = " + to_string(status*1);

    }

    PE_logs(dest, log);

    return;
}


void write_mem(uint8_t src, uint32_t addr, uint16_t num_of_cache_lines, uint16_t start_cache_line, uint16_t qos){

    //For bandwidth calculations
    auto start_time = std::chrono::high_resolution_clock::now();

    

    string log = "";

    int shared_row = addr / 32;
    int shared_col = addr % 32;

    //cout << "El valor de fila y columna con la dirección " << std::hex << addr << " son: \nfila: "<< std::dec << shared_row << "\ncolumna: " << shared_col << endl << endl;
    
    //std::lock(shared_memory_mtx,pe0_cache_mtx,pe1_cache_mtx);

    //shared_memory_mtx.lock();

    std::lock(shared_memory_mtx,(*pe_cache_mutexes[src]));

    //cout << "[PE"<< src*1 << "] → WRITE_MEM: Process Initiated" << endl;

    log = "WRITE_MEM: Process Initiated";
    PE_logs(src,log);

    //int cache_col = 1;

    uint8_t status = 0x1;

    int cache_row = start_cache_line;

    for(int cache_lines = 0; cache_lines < num_of_cache_lines; cache_lines++){

        //cout << "row " << cache_row << " col " << "0 " << "Value " << (*pe_caches[src])[cache_row][0]*1 << endl;

        if((*pe_caches[src])[cache_row][0] == 1){

            //cout << "[PE" << src*1 << "] → WRITE_MEM: Error, cache line " << cache_row << " is invalid" << endl; //We evaluate the invalidation bit
            status = 0x0;   
            
            log = "WRITE_MEM: Error, cache line " + to_string(cache_row) + " is invalid";
            PE_logs(src,log);

        }
        cache_row++;
    }


    if(status == 0x1){
        for(int cache_row = 0; cache_row < num_of_cache_lines; cache_row++){ //Loop for traverse the number of cache lines
 
            for(int cache_col=1; cache_col<(*pe_caches[src])[0].size(); cache_col++){
    
                shared_memory[shared_row][shared_col] = (*pe_caches[src])[start_cache_line][cache_col];//Operation to write on memory
                shared_col++;
    
                if(shared_col==shared_memory[0].size()){
                    
                    shared_col=0;
                    shared_row++;
                }
            }
            start_cache_line++;
        }
    }


    write_resp(src,status,qos);

    shared_memory_mtx.unlock();
    (*pe_cache_mutexes[src]).unlock();

    uint32_t total_bytes = (num_of_cache_lines * 16)*status;

    auto end_time = std::chrono::high_resolution_clock::now();

    double elapsed = std::chrono::duration<double>(end_time-start_time).count();

    pe_bandwidth[src].bytes_transfered += total_bytes;
    pe_bandwidth[src].time_spent += elapsed;
    pe_bandwidth[src].shared_memory_access_time += elapsed;


    // Comportamiento stepping (después de encolar)
    if (stepping == 1) {
        cout << "[PE" << src << "] " << "WRITE_MEM" << " Operation Enqueued: Waiting Input" << endl;
        savePECacheToFile(src, *pe_caches[src]);
        saveSharedMemoryToFile(shared_memory);
        stepping_wait(src);
    }

    return;
}







void read_resp(uint8_t dest, std::vector<uint8_t> data, uint16_t qos){
    
    string log = "";


    

    //cout << "[PE"<< dest*1 << "] → READ_RESP: Process Initiated" << endl;
    log = "READ_RESP: Process Initiated";
    PE_logs(dest,log);
    
    //cout <<"[PE" << dest*1 << "] → READ_RESP: Data wrote to cache memory:" << endl;
    log = "READ_RESP: Data wrote to cache memory:";
    PE_logs(dest,log);

    log = "";

    for(int data_index = 0; data_index < data.size(); data_index++){
        //cout << data[data_index]*1; 

        log += to_string(data[data_index]*1);
    }

    PE_logs(dest,log);
    //cout << endl;

}


void read_mem(uint8_t src, uint32_t addr, uint32_t size, uint16_t qos){

    //For bandwidth calculations
    auto start_time = std::chrono::high_resolution_clock::now();

    string log = "";

    //src = src*1;
    
    int shared_row = addr / 32;
    int shared_col = addr % 32;

    int cache_row = addr / 128;
    //int cache_row = std::floor(addr/128);
    int cache_col = addr % 128;

    //cout << "Cache row value is: " << cache_row << endl;

    //cout << "Shared row value is: " << shared_row << endl;

    int byte_to_bit = int(size)*8;

    std::vector<uint8_t> data(byte_to_bit);

    //cout << "The size in bytes is: " << std::hex << size << ". Its equivalent in decimal is: " << std::dec << byte_to_bit << endl;

    std::lock(shared_memory_mtx,(*pe_cache_mutexes[src]));

    //cout << "[PE"<< src*1 << "] → READ_MEM: Process Initiated" << endl;

    log = "READ_MEM: Process Initiated";
    PE_logs(src,log);

    if(cache_col == 0){ //Cache col 0 is always used for the invalidation bit
        cache_col = 1; 
    }

    for(int bits = 0; bits < byte_to_bit; bits++){

        if((*pe_caches[src])[cache_row][0] == 1){
            (*pe_caches[src])[cache_row][0] = 0; //We validate the bit here because we brought an updated block of memory
        }

        (*pe_caches[src])[cache_row][cache_col] = shared_memory[shared_row][shared_col];

        data[bits] =(*pe_caches[src])[cache_row][cache_col];
        //cout << "Wrote value:" << shared_memory[shared_row][shared_col]*1 << "\nrow: " << shared_row << "\ncol:"<< shared_col <<endl;

        shared_col++;
        cache_col++;

        if(shared_col == shared_memory[0].size()){//End shared memory columns

            shared_row++;
            shared_col=0;

        }
        
        if (cache_col == (*pe_caches[src])[0].size()){// End cache memory columns



            cache_row++;
            cache_col=1;

        }
        

    }

    read_resp(src, data, qos);

    shared_memory_mtx.unlock();
    (*pe_cache_mutexes[src]).unlock();

    uint32_t total_bytes = size;

    auto end_time = std::chrono::high_resolution_clock::now();

    double elapsed = std::chrono::duration<double>(end_time-start_time).count();

    pe_bandwidth[src].bytes_transfered += total_bytes;
    pe_bandwidth[src].time_spent += elapsed;
    pe_bandwidth[src].shared_memory_access_time += elapsed;


    // Comportamiento stepping (después de encolar)
    if (stepping == 1) {
        cout << "[PE" << src << "] " << "READ_MEM" << " Operation Enqueued: Waiting Input" << endl;
        savePECacheToFile(src, *pe_caches[src]);
        saveSharedMemoryToFile(shared_memory);
        stepping_wait(src);
    }





    return;
}




void inv_complete(uint8_t dest, uint16_t qos){
    //cout << "Entrada al inv_complete" << "\n\n\n\n" << endl;
    
    //cout << "[PE" << dest*1 << "] → INV_COMPLETE: Invalidation Process Completed"<<endl;
    PE_logs(dest, "INV_COMPLETE: Invalidation Process Completed");



}


void inv_ack(uint8_t src, uint16_t cache_line, uint16_t qos){

    //cout << "[PE"<< src*1 << "] → INV_ACK: Process Initiated" << endl;

    PE_logs(src, "INV_ACK: Process Initiated");

    for(int PEs = 0; PEs < 8; PEs++){

        if(pe_caches[PEs]->at(cache_line).at(0) == 1){

            //cout << "[PE"<< src*1 << "] → [PE" << unsigned(PEs) << "] INV_ACK: Invalidation Bit Confirmed at line " << cache_line <<" (QoS=" << qos << ")" << endl;
            
            PE_logs(src, "[PE" + to_string(unsigned(PEs)) + "] INV_ACK: Invalidation Bit Confirmed at line "
            + to_string(cache_line) + " (QoS=" + to_string(qos) + ")");

        }
        else{

            //cout << "[PE"<< src*1 << "] → [PE" << unsigned(PEs) << "] INV_ACK: Error, the bit was not invalidated at line " << cache_line <<" (QoS=" << qos << ")"<< endl;
            
            PE_logs(src, "[PE" + to_string(unsigned(PEs)) + "] INV_ACK: Error, the bit was not invalidated at line " 
            + to_string(cache_line) + " (QoS=" + to_string(qos) + ")");
        }
    }


    
    return;

}


void broadcast_invalidate(uint8_t src, uint16_t cache_line, uint16_t qos){
    

    //For bandwidth calculations
    auto start_time = std::chrono::high_resolution_clock::now();

    std::lock(pe0_cache_mtx, pe1_cache_mtx, pe2_cache_mtx, pe3_cache_mtx, pe4_cache_mtx, pe5_cache_mtx, pe6_cache_mtx, pe7_cache_mtx);
    
    //cout << "[PE"<< src*1 << "] → BROADCAST_INVALIDATE: Process Initiated" << endl;
    
    PE_logs(src, "BROADCAST_INVALIDATE: Process Initiated");

    /*
    string log = "";
    PE_logs(dest, log);*/

    for(int PEs = 0; PEs < 8; PEs++){

        pe_caches[PEs]->at(cache_line).at(0) = 1;
        

    }
    inv_ack(src, cache_line, qos);
    inv_complete(src, qos);

    pe0_cache_mtx.unlock();
    pe1_cache_mtx.unlock();
    pe2_cache_mtx.unlock();
    pe3_cache_mtx.unlock();
    pe4_cache_mtx.unlock();
    pe5_cache_mtx.unlock();
    pe6_cache_mtx.unlock();
    pe7_cache_mtx.unlock();



    uint32_t total_bytes = 8;

    auto end_time = std::chrono::high_resolution_clock::now();

    double elapsed = std::chrono::duration<double>(end_time-start_time).count();

    pe_bandwidth[src].bytes_transfered += total_bytes;
    pe_bandwidth[src].time_spent += elapsed;


    // Comportamiento stepping (después de encolar)
    if (stepping == 1) {
        cout << "[PE" << src << "] " << "BROADCAST_INVALIDATE" << " Operation Enqueued: Waiting Input" << endl;
        savePECacheToFile(src, *pe_caches[src]);
        saveSharedMemoryToFile(shared_memory);
        stepping_wait(src);
    }


    return;
}

void schedulerExecutor() {
    while (true) {
        MemRequest req = scheduler.get_next_request();
        cout << "[EXECUTING] PE" << unsigned(req.src) 
        << " QoS=0x" << hex << setw(2) << setfill('0') << req.qos << endl;        lock_guard<mutex> lock(mem_mutex);
        if (req.op_type == MemOpType::WRITE) {
            //cout << "Write_EXEC" << endl;
            write_mem(req.src, req.addr, req.size, req.start_cache_line, req.qos);
        } else if (req.op_type == MemOpType::READ) {
            //cout << "Read_EXEC" << endl;
            read_mem(req.src, req.addr, req.size, req.qos);
        } else if (req.op_type == MemOpType::BROADCAST_INVALIDATE){
            //cout << "BCI_EXEC" << endl;
            broadcast_invalidate(req.src, req.cache_line, req.qos);
        }
        
        if(!scheduler.has_request()){
            break;
        }

    }
}




void instructionReader(uint8_t src, uint16_t qos) {
    PE_logs(src, "666"); // Clear log history
    PE_logs(src, "Initiated Operations (QoS=" + to_string(qos) + ")");

    string filename = "../Workloads/PE" + to_string(src) + "_Instructions.txt";
    ifstream file(filename);

    if (!file.is_open()) {
        cerr << "Error opening: " << filename << endl;
        return;
    }

    auto remove_commas = [](std::string &s) {
        s.erase(std::remove(s.begin(), s.end(), ','), s.end());
    };

    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;

        istringstream iss(line);
        string instruction;
        iss >> instruction;

        MemRequest req;
        req.src = src;
        req.qos = qos;

        if (instruction == "WRITE_MEM") {
            string addr_str, n_lines_str, start_line_str;
            iss >> addr_str >> n_lines_str >> start_line_str;
            remove_commas(addr_str);
            remove_commas(n_lines_str);
            remove_commas(start_line_str);

            req.op_type = MemOpType::WRITE;
            req.addr = stoul(addr_str, nullptr, 0);
            req.size = static_cast<uint16_t>(stoul(n_lines_str, nullptr, 0));
            req.start_cache_line = static_cast<uint16_t>(stoul(start_line_str, nullptr, 0));
            cout << "Write_Mem OP" << endl;

        } else if (instruction == "READ_MEM") {
            string addr_str, size_str;
            iss >> addr_str >> size_str;
            remove_commas(addr_str);
            remove_commas(size_str);

            req.op_type = MemOpType::READ;
            req.addr = stoul(addr_str, nullptr, 0);
            req.size = static_cast<uint16_t>(stoul(size_str, nullptr, 0));
            cout << "Read_Mem OP" << endl;


        } else if (instruction == "BROADCAST_INVALIDATE") {
            string line_str;
            iss >> line_str;

            remove_commas(line_str);

            uint16_t cache_line = static_cast<uint16_t>(stoul(line_str, nullptr, 0));
            //broadcast_invalidate(src, cache_line, qos);
            req.op_type = MemOpType::BROADCAST_INVALIDATE;
            req.cache_line = stoul(line_str, nullptr, 0);
            cout << "Broadcast_Invalidate OP Param" << req.cache_line << endl;
            //continue;
        } else {
            cerr << "Unrecognized instruction: " << instruction << endl;
            continue;
        }

        

        

        // Enviar solicitud al scheduler
        scheduler.submit_request(req);
    }

    file.close();
    PE_logs(src, "Finished Operations (QoS=" + to_string(qos) + ")");
}







int main() {


    //For stepping
    if(stepping == 1){
        // Asegura que todos los archivos inician en 0
        for (int i = 0; i < 8; ++i) {
            std::ofstream out("../Inputs/PE" + std::to_string(i) + "_Input");
            out << "0\n";
        }
    }


    // Inicializar memoria y cachés (ejemplo)

    int calendarización = 1;


    //First bit of each pe_cache is the invalidation bit
    shared_memory[0][0] = 1;
    shared_memory[0][1] = 2;
    shared_memory[0][2] = 3;
    shared_memory[0][3] = 4;
    shared_memory[0][4] = 5;
    shared_memory[0][5] = 6;
    shared_memory[0][6] = 7;
    shared_memory[0][7] = 8;


    for(int row=0; row<pe0_cache.size();row++){
        for(int col=1; col<pe0_cache[0].size();col++){
            pe0_cache[row][col] = 0;
            pe1_cache[row][col] = 1;
            pe2_cache[row][col] = 2;
            pe3_cache[row][col] = 3;
            pe4_cache[row][col] = 4;
            pe5_cache[row][col] = 5;
            pe6_cache[row][col] = 6;
            pe7_cache[row][col] = 7;
        }
    }

    for(int row=0; row<shared_memory.size();row++){
        for(int col=0; col<shared_memory[0].size();col++){
            shared_memory[row][col] = 9;
        }
    }

    
    pe0_cache[0][0] = 1; //Here we disable the first line.
    pe0_cache[1][1] = 0; //Here we don't disable the line
    pe0_cache[0][0]= 1;
    pe0_cache[1][0]= 0;


    uint8_t src = 0;

    uint16_t qos = 0x0;
 
    // Orden descendente 
    const std::array<uint16_t, 8> pe_qos_values = {
        0x00, // PE0 - mínima prioridad
        0x24, // PE1
        0x48, // PE2
        0x6C, // PE3
        0x90, // PE4
        0xB4, // PE5
        0xD8, // PE6
        0xFC  // PE7 - máxima prioridad
    };
    
    std::thread PE0(instructionReader,0,pe_qos_values[0]);
    std::thread PE1(instructionReader,1,pe_qos_values[1]);
    std::thread PE2(instructionReader,2,pe_qos_values[2]);
    std::thread PE3(instructionReader,3,pe_qos_values[3]);
    std::thread PE4(instructionReader,4,pe_qos_values[4]);
    std::thread PE5(instructionReader,5,pe_qos_values[5]);
    std::thread PE6(instructionReader,6,pe_qos_values[6]);
    std::thread PE7(instructionReader,7,pe_qos_values[7]);
    


    PE0.join();
    cout << "PE 0 Finished"<< endl;
    PE1.join();
    cout << "PE 1 Finished"<< endl;
    PE2.join();
    cout << "PE 2 Finished"<< endl;
    PE3.join();
    cout << "PE 3 Finished"<< endl;
    PE4.join();
    cout << "PE 4 Finished"<< endl;
    PE5.join();
    cout << "PE 5 Finished"<< endl;
    PE6.join();
    cout << "PE 6 Finished"<< endl;
    PE7.join();
    cout << "PE 7 Finished"<< endl;


    cout << "Starting scheduler thread" << endl;

    std::thread scheduler_thread(schedulerExecutor);
    

    //cout << "El tamaño del cache es: " << pe0_cache[0].size() <<endl;
    //pe0_cache[0][pe0_cache[0].size()] = 7;

    scheduler_thread.join(); // o usa un mecanismo de apagado si quieres terminarlo


    // Guardar en archivos
    saveSharedMemoryToFile(shared_memory);

    // Suponiendo que tenemos 8 PEs (PE0 a PE7)
    for (int i = 0; i < 8; ++i) {
        savePECacheToFile(i,*pe_caches[i]);
    }


    cout << "All operations completed, program finished." << endl;

    save_bandwidth_stats();

    system("python3 ../Graphics/BandwidthObserver.py");
    system("python3 ../Graphics/BytesAndTime.py");
    system("python3 ../Graphics/SharedMemoryTimeAccess.py");


    //cout << "TEST" << endl;
    //broadcast_invalidate(0x0, 0xF, 0x0);

    //(uint8_t src, uint16_t cache_line, uint16_t qos)

    return 0;
}
