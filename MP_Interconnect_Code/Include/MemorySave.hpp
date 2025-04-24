#ifndef MEMORY_SAVE_HPP
#define MEMORY_SAVE_HPP

#include <array>
#include <cstdint>

// Declaraciones
void saveSharedMemoryToFile(const std::array<std::array<uint8_t, 32>, 4096>& shared_memory);
void savePECacheToFile(int pe_id, const std::array<std::array<uint8_t, 128>, 128>& pe_cache);

#endif
