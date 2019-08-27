
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
