
#include "test.h"

#ifdef _WIN32
#pragma comment(lib, "d3dcompiler.lib")

#ifdef ENABLE_VULKAN
#pragma comment(lib, "vulkan-1.lib")
#endif

#endif

// Empty
void test_empty(LLGI::DeviceType deviceType = LLGI::DeviceType::Default);

// About clear
void test_clear(LLGI::DeviceType deviceType = LLGI::DeviceType::Default);
void test_clear_update(LLGI::DeviceType deviceType = LLGI::DeviceType::Default);

// Render
void test_simple_rectangle(LLGI::DeviceType deviceType = LLGI::DeviceType::Default);

void test_simple_constant_rectangle(LLGI::ConstantBufferType type, LLGI::DeviceType deviceType = LLGI::DeviceType::Default);

void test_simple_texture_rectangle(LLGI::DeviceType deviceType = LLGI::DeviceType::Default);

// Compile
void test_compile(LLGI::DeviceType deviceType = LLGI::DeviceType::Default);

// About renderPass
void test_renderPass(LLGI::DeviceType deviceType = LLGI::DeviceType::Default);

int main()
{
	auto device = LLGI::DeviceType::Default;

	// Empty
	test_empty(device);

	// About clear
	// test_clear(device);
	// test_clear_update(device);

	// About compile
	// test_compile(device);

	// Render
	// test_simple_rectangle(device);
	// test_simple_constant_rectangle(LLGI::ConstantBufferType::LongTime, device);
	// test_simple_texture_rectangle(device);

	// About renderPass
	// test_renderPass(device);

	return 0;
}
