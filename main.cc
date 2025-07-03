#include <iostream>
#include <cryptopp/sha.h>
#include "EPOS/diffie_hellman.h"
#include "EPOS/poly1305.h"
#include "EPOS/cipher.h"

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

    // Examples of how to call each function
    // Diffie-Hellman shared key calculation
    EPOS::S::Diffie_Hellman::shared_key(dh_test_data[0].public_key, dh_test_data[0].private_key);

    // Poly1305 message authentication
    unsigned char mac[16];
    EPOS::S::Poly1305 poly1305 = EPOS::S::Poly1305(poly1305_test_data[0].key, poly1305_test_data[0].nonce);
    poly1305.stamp(mac, poly1305_test_data[0].nonce, reinterpret_cast<const unsigned char*>(poly1305_test_data[0].message), sizeof(poly1305_test_data[0].message));

    // AES encryption
    EPOS::S::Cipher cipher;
    unsigned char ciphertext[EPOS::S::Diffie_Hellman::SECRET_SIZE];
    cipher.encrypt(reinterpret_cast<const unsigned char*>(aes_test_data[0].message), reinterpret_cast<const unsigned char*>(aes_test_data[0].key), ciphertext);
    // AES decryption
    unsigned char decrypted_text[EPOS::S::Diffie_Hellman::SECRET_SIZE];
    cipher.decrypt(ciphertext, reinterpret_cast<const unsigned char*>(aes_test_data[0].key), decrypted_text);

    // SHA256 hashing
    unsigned char sha256_digest[CryptoPP::SHA256::DIGESTSIZE];
    CryptoPP::SHA256 hash;
    hash.CalculateDigest(sha256_digest, sha256_test_data[0].data, sizeof(sha256_test_data[0].data));


    return 0;
}