
#pragma once

#include <G3/LLGI.G3.CommandList.h>
#include <G3/LLGI.G3.Compiler.h>
#include <G3/LLGI.G3.ConstantBuffer.h>
#include <G3/LLGI.G3.Graphics.h>
#include <G3/LLGI.G3.IndexBuffer.h>
#include <G3/LLGI.G3.PipelineState.h>
#include <G3/LLGI.G3.Platform.h>
#include <G3/LLGI.G3.Shader.h>
#include <G3/LLGI.G3.Texture.h>
#include <G3/LLGI.G3.VertexBuffer.h>

#include <iostream>

struct SimpleVertex
{
	LLGI::Vec3F Pos;
	LLGI::Vec2F UV;
	LLGI::Color8 Color;
};
