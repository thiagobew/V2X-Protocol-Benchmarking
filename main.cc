#include <iostream>
#include <chrono>
#include <fstream>
#include <cryptopp/sha.h>
#include "EPOS/diffie_hellman.h"
#include "EPOS/poly1305.h"
#include "EPOS/cipher.h"
#include "EPOS/benchmark_stats.h"

#define ITERATIONS 10000
#define MAX_POLY1305_MESSAGE_SIZE 264 // Size of OTP for Forwarding Grant
#define MAX_AES_MESSAGE_SIZE 192 // Size of payload for Forwarding Grant

struct DH_Data {
    EPOS::S::Diffie_Hellman::Public_Key public_key;
    EPOS::S::Bignum<EPOS::S::Diffie_Hellman::SECRET_SIZE> private_key;
} typedef DH_Data;

struct Poly1305_Data {
    unsigned char key[EPOS::S::Diffie_Hellman::SECRET_SIZE];
    unsigned char nonce[EPOS::S::Diffie_Hellman::SECRET_SIZE];
    char message[MAX_POLY1305_MESSAGE_SIZE];
} typedef Poly1305_Data;

struct AES_Data {
    char key[EPOS::S::Diffie_Hellman::SECRET_SIZE];
    char message[MAX_AES_MESSAGE_SIZE];
} typedef AES_Data;

struct SHA256_Data {
    unsigned char data[EPOS::S::Diffie_Hellman::SECRET_SIZE];
} typedef SHA256_Data;

DH_Data dh_test_data[ITERATIONS];
Poly1305_Data poly1305_test_data[ITERATIONS];
AES_Data aes_test_data[ITERATIONS];
SHA256_Data sha256_test_data[ITERATIONS];

void fill_random(void* buffer, size_t size) {
    unsigned char* buf = static_cast<unsigned char*>(buffer);
    for (size_t i = 0; i < size; ++i) {
        buf[i] = static_cast<unsigned char>(rand() % 256);
    }
}


int main() {
    // Seed random number generator
    srand(static_cast<unsigned int>(time(nullptr)));

    // Populate DH_Data
    for (int i = 0; i < ITERATIONS; ++i) {
        fill_random(&dh_test_data[i].public_key, sizeof(dh_test_data[i].public_key));
        fill_random(&dh_test_data[i].private_key, sizeof(dh_test_data[i].private_key));
    }

    // Populate Poly1305_Data
    for (int i = 0; i < ITERATIONS; ++i) {
        fill_random(&poly1305_test_data[i].key, sizeof(poly1305_test_data[i].key));
        fill_random(&poly1305_test_data[i].nonce, sizeof(poly1305_test_data[i].nonce));
        fill_random(poly1305_test_data[i].message, sizeof(poly1305_test_data[i].message));
    }

    // Populate AES_Data
    for (int i = 0; i < ITERATIONS; ++i) {
        fill_random(&aes_test_data[i].key, sizeof(aes_test_data[i].key));
        fill_random(aes_test_data[i].message, sizeof(aes_test_data[i].message));
    }

    // Populate SHA256_Data
    for (int i = 0; i < ITERATIONS; ++i) {
        fill_random(&sha256_test_data[i].data, sizeof(sha256_test_data[i].data));
    }

    std::cout << "Structures populated with random data." << std::endl;
    std::cout << "Starting benchmarks..." << std::endl;

    // Open CSV file for results
    std::ofstream csv_file("latencies.csv");
    csv_file << "primitive,iteration,ns\n";

    // Declare timing variables once
    unsigned char sha256_digest[CryptoPP::SHA256::DIGESTSIZE];
    CryptoPP::SHA256 hash;
    unsigned char mac[16];
    EPOS::S::Cipher cipher;
    unsigned char ciphertext[EPOS::S::Diffie_Hellman::SECRET_SIZE];

    // SHA-256 benchmark
    std::cout << "Running SHA-256 benchmark..." << std::endl;
    // Warmup
    for (int i = 0; i < 100; ++i) {
        hash.CalculateDigest(sha256_digest, sha256_test_data[i % ITERATIONS].data, sizeof(sha256_test_data[i % ITERATIONS].data));
    }
    // Measured iterations
    for (int i = 0; i < ITERATIONS; ++i) {
        auto start = std::chrono::steady_clock::now();
        hash.CalculateDigest(sha256_digest, sha256_test_data[i].data, sizeof(sha256_test_data[i].data));
        auto end = std::chrono::steady_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
        csv_file << "sha256," << i << "," << duration.count() << "\n";
    }

    // AES encryption benchmark
    std::cout << "Running AES encryption benchmark..." << std::endl;
    // Warmup
    for (int i = 0; i < 100; ++i) {
        cipher.encrypt(reinterpret_cast<const unsigned char*>(aes_test_data[i % ITERATIONS].message), 
                      reinterpret_cast<const unsigned char*>(aes_test_data[i % ITERATIONS].key), 
                      ciphertext);
    }
    // Measured iterations
    for (int i = 0; i < ITERATIONS; ++i) {
        auto start = std::chrono::steady_clock::now();
        cipher.encrypt(reinterpret_cast<const unsigned char*>(aes_test_data[i].message), 
                      reinterpret_cast<const unsigned char*>(aes_test_data[i].key), 
                      ciphertext);
        auto end = std::chrono::steady_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
        csv_file << "aes128," << i << "," << duration.count() << "\n";
    }

    // Poly1305 MAC benchmark
    std::cout << "Running Poly1305 MAC benchmark..." << std::endl;
    // Warmup
    for (int i = 0; i < 100; ++i) {
        EPOS::S::Poly1305 poly1305_warmup(poly1305_test_data[i % ITERATIONS].key, poly1305_test_data[i % ITERATIONS].nonce);
        poly1305_warmup.stamp(mac, poly1305_test_data[i % ITERATIONS].nonce, 
                             reinterpret_cast<const unsigned char*>(poly1305_test_data[i % ITERATIONS].message), 
                             sizeof(poly1305_test_data[i % ITERATIONS].message));
    }
    // Measured iterations
    for (int i = 0; i < ITERATIONS; ++i) {
        auto start = std::chrono::steady_clock::now();
        EPOS::S::Poly1305 poly1305(poly1305_test_data[i].key, poly1305_test_data[i].nonce);
        poly1305.stamp(mac, poly1305_test_data[i].nonce, 
                      reinterpret_cast<const unsigned char*>(poly1305_test_data[i].message), 
                      sizeof(poly1305_test_data[i].message));
        auto end = std::chrono::steady_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
        csv_file << "poly1305," << i << "," << duration.count() << "\n";
    }

    // ECDH shared secret benchmark
    std::cout << "Running ECDH shared secret benchmark..." << std::endl;
    // Warmup
    for (int i = 0; i < 100; ++i) {
        EPOS::S::Diffie_Hellman::shared_key(dh_test_data[i % ITERATIONS].public_key, dh_test_data[i % ITERATIONS].private_key);
    }
    // Measured iterations
    for (int i = 0; i < ITERATIONS; ++i) {
        auto start = std::chrono::steady_clock::now();
        EPOS::S::Diffie_Hellman::shared_key(dh_test_data[i].public_key, dh_test_data[i].private_key);
        auto end = std::chrono::steady_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
        csv_file << "ecdh_shared," << i << "," << duration.count() << "\n";
    }

    csv_file.close();
    std::cout << "Benchmarks completed. Results saved to latencies.csv" << std::endl;

    // Read back the CSV file and calculate statistics
    std::cout << "\nCalculating statistics from latencies.csv..." << std::endl;
    auto primitive_latencies = EPOS::S::read_latencies_csv("latencies.csv");
    
    // Calculate and print statistics for each primitive
    for (const auto& pair : primitive_latencies) {
        const std::string& primitive_name = pair.first;
        const std::vector<uint64_t>& latencies = pair.second;
        
        EPOS::S::PrimitiveStats stats = EPOS::S::calculate_stats(primitive_name, latencies);
        EPOS::S::print_stats(stats);
    }

    // Calculate and print throughput statistics for primitives that process data
    std::cout << "\nCalculating throughput statistics..." << std::endl;
    
    // SHA-256 throughput (processes SECRET_SIZE bytes per iteration)
    if (primitive_latencies.find("sha256") != primitive_latencies.end()) {
        EPOS::S::PrimitiveStats sha256_stats = EPOS::S::calculate_stats("sha256", primitive_latencies.at("sha256"));
        EPOS::S::ThroughputStats sha256_throughput = EPOS::S::calculate_throughput(sha256_stats, EPOS::S::Diffie_Hellman::SECRET_SIZE);
        EPOS::S::print_throughput(sha256_throughput);
    }
    
    // Poly1305 throughput (processes MAX_POLY1305_MESSAGE_SIZE bytes per iteration)
    if (primitive_latencies.find("poly1305") != primitive_latencies.end()) {
        EPOS::S::PrimitiveStats poly1305_stats = EPOS::S::calculate_stats("poly1305", primitive_latencies.at("poly1305"));
        EPOS::S::ThroughputStats poly1305_throughput = EPOS::S::calculate_throughput(poly1305_stats, MAX_POLY1305_MESSAGE_SIZE);
        EPOS::S::print_throughput(poly1305_throughput);
    }

    return 0;
}