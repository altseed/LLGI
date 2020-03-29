
#include "TestHelper.h"
#include "test.h"
#include <string>

#ifdef _WIN32
#pragma comment(lib, "d3dcompiler.lib")

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <stdlib.h>

#endif

// Empty
void test_empty(LLGI::DeviceType deviceType = LLGI::DeviceType::Default);

// About clear
void test_clear(LLGI::DeviceType deviceType = LLGI::DeviceType::Default);
void test_clear_update(LLGI::DeviceType deviceType = LLGI::DeviceType::Default);

// Render
void test_simple_rectangle(LLGI::DeviceType deviceType = LLGI::DeviceType::Default);
void test_index_offset(LLGI::DeviceType deviceType = LLGI::DeviceType::Default);

void test_simple_constant_rectangle(LLGI::ConstantBufferType type, LLGI::DeviceType deviceType = LLGI::DeviceType::Default);

void test_simple_texture_rectangle(LLGI::DeviceType deviceType = LLGI::DeviceType::Default);

// Compile
void test_compile(LLGI::DeviceType deviceType = LLGI::DeviceType::Default);

// About renderPass
void test_renderPass(LLGI::DeviceType deviceType = LLGI::DeviceType::Default, RenderPassTestMode mode = RenderPassTestMode::None);
void test_multiRenderPass(LLGI::DeviceType deviceType = LLGI::DeviceType::Default);
void test_capture(LLGI::DeviceType deviceType = LLGI::DeviceType::Default);

// About depth
void test_depth(LLGI::DeviceType deviceType = LLGI::DeviceType::Default);

void test_stencil(LLGI::DeviceType deviceType = LLGI::DeviceType::Default);

void call_test(LLGI::DeviceType device)
{
	LLGI::SetLogger([](LLGI::LogType logType, const char* message) { printf("%s\n", message); });

	// Empty
	// test_empty(device);

	// About clear
	// test_clear(device);
	// test_clear_update(device);

	// About compile
	// test_compile(device);

	// Render
	// test_simple_rectangle(device);
	// test_index_offset(device);
	// test_simple_constant_rectangle(LLGI::ConstantBufferType::LongTime, device);
	test_simple_texture_rectangle(device);

	// About renderPass
	// test_renderPass(device, RenderPassTestMode::CopyTexture);
	// test_multiRenderPass(device);

	// test_capture(device);

	// About depth
	// test_depth(device);
	// test_stencil(device);

	LLGI::SetLogger(nullptr);
}

#if defined(__linux__) || defined(__APPLE__) || defined(WIN32)

int main(int argc, char* argv[])
{

#if _WIN32
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	
#if defined(WIN32) && 1
	auto device = LLGI::DeviceType::DirectX12;
#elif defined(__APPLE__)
	auto device = LLGI::DeviceType::Metal;
#else
	auto device = LLGI::DeviceType::Vulkan;
#endif
	
	// make shaders folder path from __FILE__
	{
		
		auto path = std::string(__FILE__);
#if defined(WIN32)
		auto pos = path.find_last_of("\\");
#else
		auto pos = path.find_last_of("/");
#endif
		
		path = path.substr(0, pos);
		
		if (device == LLGI::DeviceType::DirectX12)
		{
			TestHelper::SetRoot((path + "/Shaders/HLSL_DX12/").c_str());
		}
		else if (device == LLGI::DeviceType::Metal)
		{
			TestHelper::SetRoot((path + "/Shaders/Metal/").c_str());
		}
		else if (device == LLGI::DeviceType::Vulkan)
		{
			TestHelper::SetRoot((path + "/Shaders/SPIRV/").c_str());
		}	
	}
	
	if (argc > 1)
	{
		TestHelper::SetIsCaptureRequired(true);
		::testing::InitGoogleTest(&argc, argv);
		return RUN_ALL_TESTS();
	}
	else
	{
		call_test(device);
	}

	TestHelper::AvoidLeak();

#if _WIN32
	//_CrtDumpMemoryLeaks();
#endif

	return 0;
}
#endif
