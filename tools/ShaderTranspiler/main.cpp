
#include <ShaderTranspilerCore.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

enum class OutputType
{
	GLSL,
	VULKAN_GLSL,
	MSL,
	Max,
};

int main(int argc, char* argv[])
{

	std::vector<std::string> args;

	for (int i = 1; i < argc; i++)
	{
		args.emplace_back(argv[i]);
	}

	LLGI::ShaderStageType shaderStage = LLGI::ShaderStageType::Max;
	OutputType outputType = OutputType::Max;
	std::string code;
	std::string outputPath;

	for (size_t i = 0; i < args.size();)
	{
		if (args[i] == "--vert")
		{
			shaderStage = LLGI::ShaderStageType::Vertex;
			i += 1;
		}
		else if (args[i] == "--frag")
		{
			shaderStage = LLGI::ShaderStageType::Pixel;
			i += 1;
		}
		else if (args[i] == "-G")
		{
			outputType = OutputType::GLSL;
			i += 1;
		}
		else if (args[i] == "-M")
		{
			outputType = OutputType::MSL;
			i += 1;
		}
		else if (args[i] == "-V")
		{
			outputType = OutputType::VULKAN_GLSL;
			i += 1;
		}
		else if (args[i] == "--input")
		{
			if (i == args.size() - 1)
			{
				std::cout << "Invald input" << std::endl;
				return 0;
			}

			std::ifstream ifs(args[i + 1]);
			if (ifs.fail())
			{
				std::cout << "Invald input" << std::endl;
				return 0;
			}
			code = std::string((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

			i += 2;
		}
		else if (args[i] == "--output")
		{
			if (i == args.size() - 1)
			{
				std::cout << "Invald output" << std::endl;
				return 0;
			}

			outputPath = args[i + 1];

			i += 2;
		}
		else
		{
			i++;
		}
	}

	if (outputType == OutputType::Max)
	{
		std::cout << "Unknown type" << std::endl;
		return 0;
	}

	if (shaderStage == LLGI::ShaderStageType::Max)
	{
		std::cout << "Unknown ShaderStage" << std::endl;
		return 0;
	}

	if (outputPath == "")
	{
		std::cout << "Invalid output type" << std::endl;
		return 0;
	}

	auto generator = std::make_shared<LLGI::SPIRVGenerator>();

	auto spirv = generator->Generate(code.c_str(), shaderStage, outputType == OutputType::VULKAN_GLSL);

	if (spirv->GetData().size() == 0)
	{
		std::cout << spirv->GetError() << std::endl;
		return 0;
	}

	auto transpiler = std::shared_ptr<LLGI::SPIRVTranspiler>();

	if (outputType == OutputType::GLSL)
	{
		transpiler = std::make_shared<LLGI::SPIRVToGLSLTranspiler>(false);
	}
	else if (outputType == OutputType::VULKAN_GLSL)
	{
		transpiler = std::make_shared<LLGI::SPIRVToGLSLTranspiler>(true);
	}
	else if (outputType == OutputType::MSL)
	{
		transpiler = std::make_shared<LLGI::SPIRVToMSLTranspiler>();
	}

	if (!transpiler->Transpile(spirv))
	{
		std::cout << transpiler->GetErrorCode() << std::endl;
		return 0;
	}

	std::ofstream outputfile(outputPath);
	if (outputfile.bad())
	{
		std::cout << "Invald output" << std::endl;
		return 0;
	}

	outputfile << transpiler->GetCode();

	return 0;
}