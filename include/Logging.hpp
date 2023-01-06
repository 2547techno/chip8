#pragma once

#include <string>

using std::string;

void log(string text);
void log(unsigned char byte, bool padded = true);
void log(unsigned short twoByte, bool padded = true);
