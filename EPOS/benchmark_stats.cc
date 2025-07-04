#include "benchmark_stats.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>

__BEGIN_SYS

/**
 * @brief Read latencies from CSV file and group by primitive
 * 
 * @param filename CSV filename to read
 * @return std::map<std::string, std::vector<uint64_t>> Map of primitive name to latency values
 */
std::map<std::string, std::vector<uint64_t>> read_latencies_csv(const std::string& filename) {
    std::map<std::string, std::vector<uint64_t>> primitive_latencies;
    std::ifstream file(filename);
    std::string line;
    
    if (!file.is_open()) {
        std::cerr << "Error: Could not open " << filename << std::endl;
        return primitive_latencies;
    }
    
    // Skip header line
    std::getline(file, line);
    
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string primitive, iteration_str, latency_str;
        
        // Parse CSV line: primitive,iteration,ns
        if (std::getline(ss, primitive, ',') &&
            std::getline(ss, iteration_str, ',') &&
            std::getline(ss, latency_str)) {
            
            uint64_t latency = std::stoull(latency_str);
            primitive_latencies[primitive].push_back(latency);
        }
    }
    
    file.close();
    return primitive_latencies;
}

/**
 * @brief Calculate statistics for a set of latency measurements
 * 
 * @param primitive_name Name of the primitive
 * @param latencies Vector of latency measurements in nanoseconds
 * @return PrimitiveStats Calculated statistics
 */
PrimitiveStats calculate_stats(const std::string& primitive_name, const std::vector<uint64_t>& latencies) {
    PrimitiveStats stats;
    stats.name = primitive_name;
    stats.latencies = latencies;
    stats.iterations = latencies.size();
    
    if (latencies.empty()) {
        stats.total_ns = 0;
        stats.avg_ns = 0;
        stats.min_ns = 0;
        stats.max_ns = 0;
        stats.median_ns = 0;
        stats.stdev_ns = 0;
        return stats;
    }
    
    // Calculate total and average
    stats.total_ns = 0;
    for (uint64_t latency : latencies) {
        stats.total_ns += latency;
    }
    stats.avg_ns = static_cast<double>(stats.total_ns) / stats.iterations;
    
    // Calculate min and max
    stats.min_ns = *std::min_element(latencies.begin(), latencies.end());
    stats.max_ns = *std::max_element(latencies.begin(), latencies.end());
    
    // Calculate median
    std::vector<uint64_t> sorted_latencies = latencies;
    std::sort(sorted_latencies.begin(), sorted_latencies.end());
    if (sorted_latencies.size() % 2 == 0) {
        stats.median_ns = (sorted_latencies[sorted_latencies.size() / 2 - 1] + 
                          sorted_latencies[sorted_latencies.size() / 2]) / 2.0;
    } else {
        stats.median_ns = sorted_latencies[sorted_latencies.size() / 2];
    }
    
    // Calculate standard deviation
    double variance = 0;
    for (uint64_t latency : latencies) {
        variance += std::pow(latency - stats.avg_ns, 2);
    }
    variance /= stats.iterations;
    stats.stdev_ns = std::sqrt(variance);
    
    return stats;
}

/**
 * @brief Print statistics for a primitive in a formatted way
 * 
 * @param stats Statistics to print
 */
void print_stats(const PrimitiveStats& stats) {
    std::cout << "\n=== " << stats.name << " Statistics ===\n";
    std::cout << "Iterations: " << stats.iterations << "\n";
    std::cout << "Total time: " << stats.total_ns << " ns\n";
    std::cout << "Average: " << stats.avg_ns << " ns\n";
    std::cout << "Median: " << stats.median_ns << " ns\n";
    std::cout << "Min: " << stats.min_ns << " ns\n";
    std::cout << "Max: " << stats.max_ns << " ns\n";
    std::cout << "Std Dev: " << stats.stdev_ns << " ns\n";
    std::cout << "=========================================\n";
}

__END_SYS 