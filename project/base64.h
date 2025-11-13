#ifndef BASE64_H
#define BASE64_H

#include <string>
#include <vector>

std::string base64_encode(const unsigned char *data, size_t len);
std::vector<unsigned char> base64_decode(const std::string &input);

#endif