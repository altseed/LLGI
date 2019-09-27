
#pragma once
#include "test.h"
#include <stdint.h>
#include <string>
#include <vector>

class TestHelper
{
private:
	static std::string root_;

public:
	static std::vector<uint8_t> LoadData(const char* path);

	static std::vector<uint8_t> LoadDataWithoutRoot(const char* path);

	static void SetRoot(const char* root);

	/**
		@brief create a rectangle
	*/
	static void CreateRectangle(LLGI::Graphics* graphics,
								const LLGI::Vec3F& ul,
								const LLGI::Vec3F& lr,
								const LLGI::Color8& cul,
								const LLGI::Color8& clr,
								std::shared_ptr<LLGI::VertexBuffer>& vb,
								std::shared_ptr<LLGI::IndexBuffer>& ib);

	static void CreateShader(LLGI::Graphics* graphics,
							 LLGI::DeviceType deviceType,
							 const char* vsBinaryPath,
							 const char* psBinaryPath,
							 std::shared_ptr<LLGI::Shader>& vs,
							 std::shared_ptr<LLGI::Shader>& ps);
};

class Bitmap2D
{
public:
	struct Color
	{
		uint8_t r, g, b, a;
	};

	Bitmap2D(const std::vector<uint8_t>& data, int width, int height, bool bgraFormat);
	Bitmap2D(const char* filePath);

	Color GetPixel(int x, int y) const;
	void Save(const char* filePath);

	/**
	 * @param[in]   colorThreshold : Tolerance threshold for each color element. (Absolute value)
	 * @return      Match rate (%)
	 */
	static int CompareBitmap(const Bitmap2D& bmp1, const Bitmap2D& bmp2, int colorThreshold);

private:
	Color mixPixels(int x, int y) const;

	std::vector<uint8_t> data_; // RGBA
	int width_;
	int height_;
};
