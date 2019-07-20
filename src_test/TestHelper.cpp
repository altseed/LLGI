#include "TestHelper.h"

std::string TestHelper::root_;

std::vector<uint8_t> TestHelper::LoadData(const char* path)
{
	std::vector<uint8_t> ret;
	auto path_ = root_ + path;

#ifdef _WIN32
	FILE* fp = nullptr;
	fopen_s(&fp, path_.c_str(), "rb");

#else
	FILE* fp = fopen(path_.c_str(), "rb");
#endif

	if (fp == nullptr)
		return ret;

	fseek(fp, 0, SEEK_END);
	auto size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	ret.resize(size);
	fread(ret.data(), 1, size, fp);
	fclose(fp);

	return ret;
}

void TestHelper::SetRoot(const char* root) { root_ = root; }