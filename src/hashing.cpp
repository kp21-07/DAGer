#include "dagr.h"
#include <openssl/sha.h>
#include <stdio.h>

string sha1(binary_buffer& data) {
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1((const unsigned char*) data.data(), data.size(), hash);

    char hex[41];
    for (int i = 0; i < SHA_DIGEST_LENGTH; i++) {
        sprintf(&hex[i*2], "%02x", hash[i]);
    }
    return string(hex);
}
