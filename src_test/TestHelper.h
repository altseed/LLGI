
#pragma once
#include <stdint.h>
#include <string>
#include <vector>

class TestHelper
{
private:
	static std::string root_;

public:
	static std::vector<uint8_t> LoadData(const char* path);
	static void SetRoot(const char* root);
};