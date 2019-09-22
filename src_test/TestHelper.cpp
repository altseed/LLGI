
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "TestHelper.h"
#include "thirdparty/stb/stb_image.h"
#include "thirdparty/stb/stb_image_write.h"

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
	{
		std::cout << "Error: " << path << std::endl;
		return ret;
	}

	fseek(fp, 0, SEEK_END);
	auto size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	ret.resize(size);
	fread(ret.data(), 1, size, fp);
	fclose(fp);

	return ret;
}

void TestHelper::SetRoot(const char* root) { root_ = root; }

void TestHelper::CreateRectangle(LLGI::Graphics* graphics,
								 const LLGI::Vec3F& ul,
								 const LLGI::Vec3F& lr,
								 const LLGI::Color8& cul,
								 const LLGI::Color8& clr,
								 std::shared_ptr<LLGI::VertexBuffer>& vb,
								 std::shared_ptr<LLGI::IndexBuffer>& ib)
{
	vb = LLGI::CreateSharedPtr(graphics->CreateVertexBuffer(sizeof(SimpleVertex) * 4));
	ib = LLGI::CreateSharedPtr(graphics->CreateIndexBuffer(2, 6));
	auto vb_buf = (SimpleVertex*)vb->Lock();
	vb_buf[0].Pos = LLGI::Vec3F(ul.X, ul.Y, ul.Z);
	vb_buf[1].Pos = LLGI::Vec3F(lr.X, ul.Y, ul.Z);
	vb_buf[2].Pos = LLGI::Vec3F(lr.X, lr.Y, lr.Z);
	vb_buf[3].Pos = LLGI::Vec3F(ul.X, lr.Y, lr.Z);

	vb_buf[0].UV = LLGI::Vec2F(0.0f, 0.0f);
	vb_buf[1].UV = LLGI::Vec2F(1.0f, 0.0f);
	vb_buf[2].UV = LLGI::Vec2F(1.0f, 1.0f);
	vb_buf[3].UV = LLGI::Vec2F(0.0f, 1.0f);

	vb_buf[0].Color = cul;
	vb_buf[1].Color = LLGI::Color8(cul.R, cul.G, clr.B, cul.A);
	vb_buf[2].Color = clr;
	vb_buf[3].Color = LLGI::Color8(clr.R, clr.G, cul.B, clr.A);

	vb->Unlock();

	auto ib_buf = (uint16_t*)ib->Lock();
	ib_buf[0] = 0;
	ib_buf[1] = 1;
	ib_buf[2] = 2;
	ib_buf[3] = 0;
	ib_buf[4] = 2;
	ib_buf[5] = 3;
	ib->Unlock();
}

void TestHelper::CreateShader(LLGI::Graphics* graphics,
							  LLGI::DeviceType deviceType,
							  const char* vsBinaryPath,
							  const char* psBinaryPath,
							  std::shared_ptr<LLGI::Shader>& vs,
							  std::shared_ptr<LLGI::Shader>& ps)
{
	auto compiler = LLGI::CreateSharedPtr(LLGI::CreateCompiler(deviceType));

	std::vector<LLGI::DataStructure> data_vs;
	std::vector<LLGI::DataStructure> data_ps;

	if (compiler == nullptr)
	{
		auto vsBinaryPath_ = std::string(vsBinaryPath);
		auto psBinaryPath_ = std::string(psBinaryPath);

		if (deviceType == LLGI::DeviceType::Vulkan)
		{
			vsBinaryPath_ += ".spv";
			psBinaryPath_ += ".spv";
		}

		auto binary_vs = TestHelper::LoadData(vsBinaryPath_.c_str());
		auto binary_ps = TestHelper::LoadData(psBinaryPath_.c_str());

		LLGI::DataStructure d_vs;
		LLGI::DataStructure d_ps;

		d_vs.Data = binary_vs.data();
		d_vs.Size = binary_vs.size();
		d_ps.Data = binary_ps.data();
		d_ps.Size = binary_ps.size();

		data_vs.push_back(d_vs);
		data_ps.push_back(d_ps);

		vs = LLGI::CreateSharedPtr(graphics->CreateShader(data_vs.data(), data_vs.size()));
		ps = LLGI::CreateSharedPtr(graphics->CreateShader(data_ps.data(), data_ps.size()));
	}
	else
	{
		LLGI::CompilerResult result_vs;
		LLGI::CompilerResult result_ps;

		auto vsBinaryPath_ = std::string(vsBinaryPath);
		auto psBinaryPath_ = std::string(psBinaryPath);

		auto code_vs = TestHelper::LoadData(vsBinaryPath_.c_str());
		auto code_ps = TestHelper::LoadData(psBinaryPath_.c_str());
		code_vs.push_back(0);
		code_ps.push_back(0);

		compiler->Compile(result_vs, (const char*)code_vs.data(), LLGI::ShaderStageType::Vertex);
		compiler->Compile(result_ps, (const char*)code_ps.data(), LLGI::ShaderStageType::Pixel);

		std::cout << result_vs.Message.c_str() << std::endl;
		std::cout << result_ps.Message.c_str() << std::endl;

		for (auto& b : result_vs.Binary)
		{
			LLGI::DataStructure d;
			d.Data = b.data();
			d.Size = b.size();
			data_vs.push_back(d);
		}

		for (auto& b : result_ps.Binary)
		{
			LLGI::DataStructure d;
			d.Data = b.data();
			d.Size = b.size();
			data_ps.push_back(d);
		}

		vs = LLGI::CreateSharedPtr(graphics->CreateShader(data_vs.data(), data_vs.size()));
		ps = LLGI::CreateSharedPtr(graphics->CreateShader(data_ps.data(), data_ps.size()));
	}
}

Bitmap2D::Bitmap2D(const std::vector<uint8_t>& data, int width, int height, bool bgraFormat) : data_(data), width_(width), height_(height)
{
	if (bgraFormat)
	{
		for (int i = 0; i < width_ * height_; ++i)
		{
			auto* b = &data_[i * 4];
			std::swap(b[0], b[2]);
		}
	}
}

Bitmap2D::Bitmap2D(const char* filePath) : data_(), width_(0), height_(0)
{
	auto data = TestHelper::LoadData(filePath);

	int width;
	int height;
	int bpp;
	unsigned char* pixels = stbi_load_from_memory(data.data(), data.size(), &width, &height, &bpp, 4);

	data_ = std::vector<uint8_t>(pixels, pixels + (width * height * 4));
	width_ = width;
	height_ = height;

	stbi_image_free(pixels);
}

Bitmap2D::Color Bitmap2D::GetPixel(int x, int y) const
{
	auto* b = &data_[(x + y * width_) * 4];
	return Bitmap2D::Color{b[0], b[1], b[2], b[3]};
}

void Bitmap2D::Save(const char* filePath) { stbi_write_png(filePath, width_, height_, 4, data_.data(), width_ * 4); }

int Bitmap2D::CompareBitmap(const Bitmap2D& bmp1, const Bitmap2D& bmp2, int colorThreshold)
{
	assert(bmp1.width_ == bmp2.width_ && bmp1.height_ == bmp2.height_);

	int pass = 0;
	for (int y = 0; y < bmp1.height_; ++y)
	{
		for (int x = 0; x < bmp1.width_; ++x)
		{
			auto c1 = bmp1.mixPixels(x, y);
			auto c2 = bmp2.mixPixels(x, y);
			if (abs(c1.r - c2.r) <= colorThreshold && abs(c1.g - c2.g) <= colorThreshold && abs(c1.b - c2.b) <= colorThreshold &&
				abs(c1.a - c2.a) <= colorThreshold)
			{
				++pass;
			}
		}
	}

	return 100 * pass / (bmp1.width_ * bmp1.height_);
}

Bitmap2D::Color Bitmap2D::mixPixels(int x, int y) const
{
	auto c = GetPixel(x, y);
	int r = c.r;
	int g = c.g;
	int b = c.b;
	int a = c.a;
	int count = 1;

	if (y > 0)
	{
		if (x > 0)
		{
			auto c = GetPixel(x - 1, y - 1);
			r += c.r;
			g += c.g;
			b += c.b;
			a += c.a;
			++count;
		}
		{
			auto c = GetPixel(x, y - 1);
			r += c.r;
			g += c.g;
			b += c.b;
			a += c.a;
			++count;
		}
		if (x < width_ - 1)
		{
			auto c = GetPixel(x + 1, y - 1);
			r += c.r;
			g += c.g;
			b += c.b;
			a += c.a;
			++count;
		}
	}
	{
		if (x > 0)
		{
			auto c = GetPixel(x - 1, y);
			r += c.r;
			g += c.g;
			b += c.b;
			a += c.a;
			++count;
		}
		if (x < width_ - 1)
		{
			auto c = GetPixel(x + 1, y);
			r += c.r;
			g += c.g;
			b += c.b;
			a += c.a;
			++count;
		}
	}
	if (y < height_ - 1)
	{
		if (x > 0)
		{
			auto c = GetPixel(x - 1, y + 1);
			r += c.r;
			g += c.g;
			b += c.b;
			a += c.a;
			++count;
		}
		{
			auto c = GetPixel(x, y + 1);
			r += c.r;
			g += c.g;
			b += c.b;
			a += c.a;
			++count;
		}
		if (x < width_ - 1)
		{
			auto c = GetPixel(x + 1, y + 1);
			r += c.r;
			g += c.g;
			b += c.b;
			a += c.a;
			++count;
		}
	}

	return Color{
		static_cast<uint8_t>(r / count), static_cast<uint8_t>(g / count), static_cast<uint8_t>(b / count), static_cast<uint8_t>(a / count)};
}
