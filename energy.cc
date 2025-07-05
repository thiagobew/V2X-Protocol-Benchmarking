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

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " [dh|poly1305|aes-encrypt|aes-decrypt|sha256]" << std::endl;
        return 1;
    }

    std::string test_name = argv[1];

    size_t i = 0;
    enum TestType { SHA256, AES_ENCRYPT, AES_DECRYPT, POLY1305, DH, UNKNOWN };
    TestType test_type = UNKNOWN;

    if (test_name == "sha256") test_type = SHA256;
    else if (test_name == "aes-encrypt") test_type = AES_ENCRYPT;
    else if (test_name == "aes-decrypt") test_type = AES_DECRYPT;
    else if (test_name == "poly1305") test_type = POLY1305;
    else if (test_name == "dh") test_type = DH;

    switch (test_type) {
        case SHA256: {
            fill_random(&sha256_test_data[i], sizeof(SHA256_Data));
            unsigned char sha256_digest[CryptoPP::SHA256::DIGESTSIZE];
            CryptoPP::SHA256 hash;
            while (true) {
                auto idx = i % ITERATIONS;
                hash.CalculateDigest(sha256_digest, sha256_test_data[idx].data, sizeof(sha256_test_data[idx].data));
                ++i;
            }
            break;
        }
        case AES_ENCRYPT: {
            EPOS::S::Cipher cipher;
            unsigned char ciphertext[EPOS::S::Diffie_Hellman::SECRET_SIZE];
            fill_random(&aes_test_data[i], sizeof(AES_Data));
            while (true) {
                auto idx = i % ITERATIONS;
                cipher.encrypt(reinterpret_cast<const unsigned char*>(aes_test_data[idx].message), 
                               reinterpret_cast<const unsigned char*>(aes_test_data[idx].key), 
                               ciphertext);
                ++i;
            }
            break;
        }
        case AES_DECRYPT: {
            EPOS::S::Cipher cipher;
            unsigned char ciphertext[EPOS::S::Diffie_Hellman::SECRET_SIZE];
            fill_random(&aes_test_data[i], sizeof(AES_Data));

            // Pre-encrypt all messages for decryption test
            while (true) {
                auto idx = i % ITERATIONS;
                cipher.decrypt(reinterpret_cast<const unsigned char*>(aes_test_data[idx].message),
                               reinterpret_cast<const unsigned char*>(aes_test_data[idx].key),
                               reinterpret_cast<unsigned char*>(aes_test_data[idx].message)); // Overwrite with plaintext
                ++i;
            }
            break;
        }
        case POLY1305:
            fill_random(&poly1305_test_data[i], sizeof(Poly1305_Data));
            unsigned char mac[16];

            while (true) {
                auto idx = i % ITERATIONS;
                EPOS::S::Poly1305 poly1305(poly1305_test_data[idx].key, poly1305_test_data[idx].nonce);
                poly1305.stamp(mac, poly1305_test_data[idx].nonce, 
                               reinterpret_cast<const unsigned char*>(poly1305_test_data[idx].message), 
                               sizeof(poly1305_test_data[idx].message));
                ++i;
            }
            break;
        case DH:
            fill_random(&dh_test_data[i], sizeof(DH_Data));
            while (true) {
                auto idx = i % ITERATIONS;
                EPOS::S::Diffie_Hellman::shared_key(dh_test_data[idx].public_key, dh_test_data[idx].private_key);
                ++i;
            }
            break;
        default:
            std::cerr << "Unknown test name: " << test_name << std::endl;
            return 1;
    }

    return 0;
}