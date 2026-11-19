#pragma once

#include <stddef.h>
#include <stdint.h>

// AES-128 encryption (FIPS-197), the forward cipher only: enough for counter mode, which encrypts and
// decrypts alike.
class Aes128 {
public:
    explicit Aes128(const uint8_t key[16]);

    void encryptBlock(const uint8_t in[16], uint8_t out[16]) const;
    // Counter mode in place: the counter block starts at counter and grows as a 128-bit big-endian number.
    void ctr(const uint8_t counter[16], uint8_t* data, size_t length) const;

private:
    uint8_t roundKeys_[11][16];
};
