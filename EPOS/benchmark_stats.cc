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

/**
 * @brief Convert latency statistics to throughput (bytes/second).
 *
 * This helper transforms nanosecond-level latency measurements into
 * bytes-per-second throughput figures. All values (average, min, max,
 * median, standard deviation) are computed using the same formulas used
 * in calculate_stats(), but applied to the derived throughput samples.
 *
 * @param latency_stats Primitive latency statistics already calculated
 * @param bytes_per_iter Number of bytes processed per iteration
 * @return ThroughputStats Aggregated throughput statistics
 */
ThroughputStats calculate_throughput(const PrimitiveStats& latency_stats, size_t bytes_per_iter) {
    ThroughputStats tstats;
    tstats.name = latency_stats.name;
    tstats.bytes_per_iteration = bytes_per_iter;
    tstats.iterations = latency_stats.iterations;

    // Guard against division by zero / empty vector
    if (latency_stats.latencies.empty()) {
        tstats.avg_bps = tstats.min_bps = tstats.max_bps = 0;
        tstats.median_bps = tstats.stdev_bps = 0;
        return tstats;
    }

    // Convert each latency sample to throughput (bytes per second)
    tstats.bps.reserve(latency_stats.latencies.size());
    for (uint64_t ns : latency_stats.latencies) {
        // bytes_per_iter * 1e9 / nanoseconds -> bytes per second
        double bps = (static_cast<double>(bytes_per_iter) * 1e9) / static_cast<double>(ns);
        tstats.bps.push_back(bps);
    }

    // Aggregate statistics
    double total_bps = 0.0;
    for (double v : tstats.bps) total_bps += v;
    tstats.avg_bps = total_bps / tstats.bps.size();

    tstats.min_bps = *std::min_element(tstats.bps.begin(), tstats.bps.end());
    tstats.max_bps = *std::max_element(tstats.bps.begin(), tstats.bps.end());

    std::vector<double> sorted_bps = tstats.bps;
    std::sort(sorted_bps.begin(), sorted_bps.end());
    if (sorted_bps.size() % 2 == 0) {
        tstats.median_bps = (sorted_bps[sorted_bps.size()/2 - 1] + sorted_bps[sorted_bps.size()/2]) / 2.0;
    } else {
        tstats.median_bps = sorted_bps[sorted_bps.size()/2];
    }

    double variance = 0.0;
    for (double v : tstats.bps) variance += std::pow(v - tstats.avg_bps, 2);
    variance /= tstats.bps.size();
    tstats.stdev_bps = std::sqrt(variance);

    return tstats;
}

/**
 * @brief Print throughput statistics for a primitive.
 *
 * @param stats Pre-calculated throughput statistics
 */
void print_throughput(const ThroughputStats& stats) {
    std::cout << "\n=== " << stats.name << " Throughput ===\n";
    std::cout << "Bytes/iteration: " << stats.bytes_per_iteration << "\n";
    std::cout << "Iterations: " << stats.iterations << "\n";
    std::cout << "Average: " << stats.avg_bps << " B/s\n";
    std::cout << "Median:  " << stats.median_bps << " B/s\n";
    std::cout << "Min:     " << stats.min_bps << " B/s\n";
    std::cout << "Max:     " << stats.max_bps << " B/s\n";
    std::cout << "Std Dev: " << stats.stdev_bps << " B/s\n";
    std::cout << "=========================================\n";
}

__END_SYS 