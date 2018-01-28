#pragma once
#include <string>

void DumpHex(const uint8_t* byteArray, int lenght, bool oneLine = true);
std::string ToHexString(const unsigned char *byteArray, int length);
std::string HashToHexString(const uint64_t* hash);
