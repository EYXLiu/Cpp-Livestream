#include "base64.h"
#include <stdexcept>

static const char b64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static inline bool is_base64(unsigned char c) {
    return (isalnum(c) || c == '+' || c == '/');
}

std::string base64_encode(const unsigned char* data, size_t len) {
    std::string result;
    result.reserve((len + 2) / 3 * 4);

    for (size_t i = 0; i < len; i += 3) {
        unsigned int val = (data[i] << 16);
        if (i + 1 < len) val |= (data[i + 1] << 8);
        if (i + 2 < len) val |= data[i + 2];

        result.push_back(b64_table[(val >> 18) & 0x3F]);
        result.push_back(b64_table[(val >> 12) & 0x3F]);
        result.push_back((i + 1 < len) ? b64_table[(val >> 6) & 0x3F] : '=');
        result.push_back((i + 2 < len) ? b64_table[val & 0x3F] : '=');
    }

    return result;
}

std::vector<unsigned char> base64_decode(const std::string& input) {
    static const int decode_table[256] = {
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 62,-1,-1,-1, 63,
        52,53,54,55,56,57,58,59,60,61,-1,-1,-1,  0,-1,-1,
        -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
        15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,
        -1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
        41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1
    };

    int len = input.size();
    if (len % 4 != 0)
        throw std::runtime_error("Invalid base64 length");

    std::vector<unsigned char> out;
    out.reserve((len / 4) * 3);

    unsigned int val = 0;
    int valb = -8;
    for (unsigned char c : input) {
        if (decode_table[c] == -1) {
            if (c == '=') break;
            continue;
        }
        val = (val << 6) + decode_table[c];
        valb += 6;
        if (valb >= 0) {
            out.push_back((val >> valb) & 0xFF);
            valb -= 8;
        }
    }

    return out;
}
