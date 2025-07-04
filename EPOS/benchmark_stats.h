#ifndef __benchmark_stats_h
#define __benchmark_stats_h

#include <vector>
#include <map>
#include <string>
#include <cstdint>
#include "epos_common.h"

__BEGIN_SYS

struct PrimitiveStats {
    std::string name;
    std::vector<uint64_t> latencies;
    uint64_t total_ns;
    double avg_ns;
    uint64_t min_ns;
    uint64_t max_ns;
    double median_ns;
    double stdev_ns;
    size_t iterations;
};

/**
 * @brief Read latencies from CSV file and group by primitive
 * 
 * @param filename CSV filename to read
 * @return std::map<std::string, std::vector<uint64_t>> Map of primitive name to latency values
 */
std::map<std::string, std::vector<uint64_t>> read_latencies_csv(const std::string& filename);

/**
 * @brief Calculate statistics for a set of latency measurements
 * 
 * @param primitive_name Name of the primitive
 * @param latencies Vector of latency measurements in nanoseconds
 * @return PrimitiveStats Calculated statistics
 */
PrimitiveStats calculate_stats(const std::string& primitive_name, const std::vector<uint64_t>& latencies);

/**
 * @brief Print statistics for a primitive in a formatted way
 * 
 * @param stats Statistics to print
 */
void print_stats(const PrimitiveStats& stats);

__END_SYS

#endif 