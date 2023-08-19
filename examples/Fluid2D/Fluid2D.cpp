#include <iostream>

#include <LLGI.Base.h>
#include <LLGI.Buffer.h>
#include <LLGI.CommandList.h>
#include <LLGI.Compiler.h>
#include <LLGI.Shader.h>
#include <LLGI.PipelineState.h>
#include <LLGI.Graphics.h>

#include "Fluid2D.h"

std::vector<uint8_t> LoadData(const char* path)
{
    std::vector<uint8_t> ret;

#ifdef _WIN32
    FILE* fp = nullptr;
    fopen_s(&fp, path, "rb");

#else
    FILE* fp = fopen(path, "rb");
#endif

    if (fp == nullptr)
    {
        std::cout << "Not found : " << path << std::endl;
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

std::shared_ptr<LLGI::Shader> CreateShader(
    LLGI::Graphics* graphics,
    LLGI::DeviceType deviceType,
    const char* csFilename,
    LLGI::ShaderStageType stageType)
{
    const auto compiler = LLGI::CreateSharedPtr(LLGI::CreateCompiler(deviceType));

    std::vector<LLGI::DataStructure> data;

    auto csBinaryPath = std::string(EXAMPLE_ASSET_DIR);

    if (compiler == nullptr)
    {
        csBinaryPath = csBinaryPath + "/Shaders/SPIRV/" + csFilename + ".spv";
        const auto binary_cs = LoadData(csBinaryPath.c_str());

        LLGI::DataStructure d;

        d.Data = binary_cs.data();
        d.Size = static_cast<int32_t>(binary_cs.size());

        data.push_back(d);

        return LLGI::CreateSharedPtr(graphics->CreateShader(data.data(), static_cast<int32_t>(data.size())));
    }
    else
    {
        csBinaryPath = csBinaryPath + "/Shaders/HLSL_DX12/" + csFilename;
        LLGI::CompilerResult result;

        auto code = LoadData(csBinaryPath.c_str());
        code.push_back(0);

        compiler->Compile(result, (const char*)code.data(), stageType);

        std::cout << result.Message.c_str() << std::endl;

        for (auto& b : result.Binary)
        {
            LLGI::DataStructure d;
            d.Data = b.data();
            d.Size = static_cast<int32_t>(b.size());
            data.push_back(d);
        }

        return LLGI::CreateSharedPtr(graphics->CreateShader(data.data(), static_cast<int32_t>(data.size())));
    }
}

float Fluid2D::CalcRestDensity(const float h)
{
    const auto a = 4.0f / (Pi * std::powf(h, 8.0f));
    const auto l = 2 * ParticleRadius;
    const auto n = (int)std::ceilf(h / l) + 1;

    auto r0 = 0.0f;

    for (int x = -n; x <= n; ++x)
    {
        for (int y = -n; y <= n; ++y)
        {
            const auto rij = LLGI::Vec2F(x * l, y * l);
            const auto r = std::sqrtf(rij.X * rij.X + rij.Y * rij.Y);
            if (r >= 0.0 && r <= h)
            {
                const auto q = h * h - r * r;
                r0 += a * q * q * q;
            }
        }
    }
    return r0;
}

void copyVec2F(const LLGI::Vec2F& src, float* dst)
{
    dst[0] = src.X;
    dst[1] = src.Y;
}

void copyVec2I(const LLGI::Vec2I& src, int* dst)
{
    dst[0] = src.X;
    dst[1] = src.Y;
}

Fluid2D::Fluid2D(LLGI::Graphics* graphics, LLGI::DeviceType deviceType)
    : gridNum_(LLGI::Vec2I(60, 60))
    , gridSize_(LLGI::Vec2I(5, 5))
    , density_(CalcRestDensity(EffectiveRadius))
    , wpoly6_(4.0f / (Pi * std::powf(EffectiveRadius, 8.0f)))
    , gwspiky_(-30.0f / (Pi * powf(EffectiveRadius, 5.0f)))
{
    std::cout << "Creating compute buffers." << std::endl;

    particleComputeIndex_ = LLGI::CreateSharedPtr(
        graphics->CreateBuffer(
            LLGI::BufferUsageType::Compute | LLGI::BufferUsageType::CopySrc,
            sizeof(int) * ParticlesCount * 6));

    particleComputeVertex_ = LLGI::CreateSharedPtr(
        graphics->CreateBuffer(
            LLGI::BufferUsageType::Compute | LLGI::BufferUsageType::CopySrc,
            sizeof(Vertex) * ParticlesCount * 4));

    particleIndex_ = LLGI::CreateSharedPtr(
        graphics->CreateBuffer(
            LLGI::BufferUsageType::Index,
            sizeof(int) * ParticlesCount * 6));

    particleVertex_ = LLGI::CreateSharedPtr(
        graphics->CreateBuffer(
            LLGI::BufferUsageType::Vertex,
            sizeof(Vertex) * ParticlesCount * 4));

    particles_ = LLGI::CreateSharedPtr(
        graphics->CreateBuffer(
            LLGI::BufferUsageType::Compute | LLGI::BufferUsageType::CopyDst,
            sizeof(Particle) * ParticlesCount));

    gridTable_ = LLGI::CreateSharedPtr(
        graphics->CreateBuffer(
            LLGI::BufferUsageType::Compute,
            sizeof(int[2]) * ParticlesCount));

    gridIndicesTable_ = LLGI::CreateSharedPtr(
        graphics->CreateBuffer(
            LLGI::BufferUsageType::Compute,
            sizeof(int[2]) * gridNum_.X * gridNum_.Y));

    // Create Shader
    std::cout << "Creating shaders." << std::endl;

    vs_ = CreateShader(graphics, deviceType, "render.vert", LLGI::ShaderStageType::Vertex);
    ps_ = CreateShader(graphics, deviceType, "render.frag", LLGI::ShaderStageType::Pixel);
    calcExternalShader_ = CreateShader(graphics, deviceType, "calcExternalForce.comp", LLGI::ShaderStageType::Compute);
    buildGridShader_ = CreateShader(graphics, deviceType, "buildGrid.comp", LLGI::ShaderStageType::Compute);
    bitonicSortShader_ = CreateShader(graphics, deviceType, "bitonicSort.comp", LLGI::ShaderStageType::Compute);
    clearGridIndicesShader_ = CreateShader(graphics, deviceType, "clearGridIndices.comp", LLGI::ShaderStageType::Compute);
    buildGridIndicesShader_ = CreateShader(graphics, deviceType, "buildGridIndices.comp", LLGI::ShaderStageType::Compute);
    calcScalingFactorShader_ = CreateShader(graphics, deviceType, "calcScalingFactor.comp", LLGI::ShaderStageType::Compute);
    calcCorrectPositionShader_ = CreateShader(graphics, deviceType, "calcCorrectPosition.comp", LLGI::ShaderStageType::Compute);
    integrateShader_ = CreateShader(graphics, deviceType, "integrate.comp", LLGI::ShaderStageType::Compute);
    buildVBIBShader_ = CreateShader(graphics, deviceType, "buildVBIB.comp", LLGI::ShaderStageType::Compute);

    // calcExternal
    std::cout << "Creating calcExternalPipeline_" << std::endl;
    calcExternalPipeline_ = LLGI::CreateSharedPtr(graphics->CreatePiplineState());
    calcExternalConstant_ = LLGI::CreateSharedPtr(
        graphics->CreateBuffer(LLGI::BufferUsageType::Constant | LLGI::BufferUsageType::MapWrite, sizeof(CalcExternalForceCB))
    );
    calcExternalPipeline_->SetShader(LLGI::ShaderStageType::Compute, calcExternalShader_.get());
    if (!calcExternalPipeline_->Compile())
    {
        std::cerr << "failed to compile calcExternalPipline" << std::endl;
        abort();
    }

    {
        const auto cb = (CalcExternalForceCB*)calcExternalConstant_->Lock();

        cb->Force[0] = 0;
        cb->Force[1] = 0;
        cb->Gravity[0] = 0;
        cb->Gravity[1] = 100;
        cb->Dt[0] = Dt;
        cb->Dt[1] = 0;
        copyVec2I(gridNum_, cb->GridNum);
        copyVec2I(gridSize_, cb->GridSize);

        calcExternalConstant_->Unlock();
    }

    // buildGrid
    std::cout << "Creating buildGridPipeline_" << std::endl;
    buildGridPipeline_ = LLGI::CreateSharedPtr(graphics->CreatePiplineState());
    buildGridConstant_ = LLGI::CreateSharedPtr(
        graphics->CreateBuffer(LLGI::BufferUsageType::Constant | LLGI::BufferUsageType::MapWrite, sizeof(BuildGridCB))
    );

    buildGridPipeline_->SetShader(LLGI::ShaderStageType::Compute, buildGridShader_.get());
    if (!buildGridPipeline_->Compile())
    {
        std::cerr << "failed to compile buildGridPipeline" << std::endl;
        abort();
    }

    {
        const auto cb = (BuildGridCB*)buildGridConstant_->Lock();

        copyVec2I(gridNum_, cb->GridNum);
        copyVec2I(gridSize_, cb->GridSize);
        cb->ParticlesCount = ParticlesCount;

        buildGridConstant_->Unlock();
    }

    // bitonicSort
    std::cout << "Creating bitonicSortPipeline_" << std::endl;
    bitonicSortPipeline_ = LLGI::CreateSharedPtr(graphics->CreatePiplineState());
    bitonicSortConstant_ = LLGI::CreateSharedPtr(
        graphics->CreateBuffer(LLGI::BufferUsageType::Constant | LLGI::BufferUsageType::MapWrite, sizeof(BuildGridIndicesCB))
    );

    bitonicSortPipeline_->SetShader(LLGI::ShaderStageType::Compute, bitonicSortShader_.get());
    if (!bitonicSortPipeline_->Compile())
    {
        std::cerr << "failed to compile bitonicSortPipeline" << std::endl;
        abort();
    }

    // clearGridIndices
    std::cout << "Creating clearGridIndicesPipeline_" << std::endl;
    clearGridIndicesPipeline_ = LLGI::CreateSharedPtr(graphics->CreatePiplineState());
    clearGridIndicesPipeline_->SetShader(LLGI::ShaderStageType::Compute, clearGridIndicesShader_.get());
    if (!clearGridIndicesPipeline_->Compile())
    {
        std::cerr << "failed to compile clearGridIndicesPipeline" << std::endl;
        abort();
    }

    // buildGridIndices
    std::cout << "Creating buildGridIndicesPipeline_" << std::endl;
    buildGridIndicesPipeline_ = LLGI::CreateSharedPtr(graphics->CreatePiplineState());
    buildGridIndicesConstant_ = LLGI::CreateSharedPtr(
        graphics->CreateBuffer(LLGI::BufferUsageType::Constant | LLGI::BufferUsageType::MapWrite, sizeof(BuildGridIndicesCB))
    );
    buildGridIndicesPipeline_->SetShader(LLGI::ShaderStageType::Compute, buildGridIndicesShader_.get());
    if (!buildGridIndicesPipeline_->Compile())
    {
        std::cerr << "failed to compile buildGridIndicesPipeline" << std::endl;
        abort();
    }

    {
        const auto cb = (BuildGridIndicesCB*)buildGridIndicesConstant_->Lock();

        cb->ParticlesCount = ParticlesCount;

        buildGridIndicesConstant_->Unlock();
    }

    // calcScalingfactor
    std::cout << "Creating calcScalingFactorPipeline_" << std::endl;
    calcScalingFactorPipeline_ = LLGI::CreateSharedPtr(graphics->CreatePiplineState());
    calcScalingFactorConstant_ = LLGI::CreateSharedPtr(
        graphics->CreateBuffer(LLGI::BufferUsageType::Constant | LLGI::BufferUsageType::MapWrite, sizeof(CalcScalingFactorCB))
    );
    calcScalingFactorPipeline_->SetShader(LLGI::ShaderStageType::Compute, calcScalingFactorShader_.get());
    if (!calcScalingFactorPipeline_->Compile())
    {
        std::cerr << "failed to compile calcScalingFactorPipeline" << std::endl;
        abort();
    }

    {
        const auto cb = (CalcScalingFactorCB*)calcScalingFactorConstant_->Lock();

        copyVec2I(gridNum_, cb->GridNum);
        copyVec2I(gridSize_, cb->GridSize);
        cb->EffectiveRadius = EffectiveRadius;
        cb->Density = density_;
        cb->Eps = Eps;
        cb->Dt = Dt;
        cb->Wpoly6 = wpoly6_;
        cb->GWspiky = gwspiky_;

        calcScalingFactorConstant_->Unlock();
    }

    // calcCorrectPosition
    std::cout << "Creating calcCorrectPositionPipeline_" << std::endl;
    calcCorrectPositionPipeline_ = LLGI::CreateSharedPtr(graphics->CreatePiplineState());
    calcCorrectPositionConstant_ = LLGI::CreateSharedPtr(
        graphics->CreateBuffer(LLGI::BufferUsageType::Constant | LLGI::BufferUsageType::MapWrite, sizeof(CalcCorrectPositionCB))
    );

    calcCorrectPositionPipeline_->SetShader(LLGI::ShaderStageType::Compute, calcCorrectPositionShader_.get());
    if (!calcCorrectPositionPipeline_->Compile())
    {
        std::cerr << "failed to compile calcCorrectPositionPipeline" << std::endl;
        abort();
    }

    {
        const auto cb = (CalcCorrectPositionCB*)calcCorrectPositionConstant_->Lock();

        copyVec2I(gridNum_, cb->GridNum);
        copyVec2I(gridSize_, cb->GridSize);
        cb->EffectiveRadius = EffectiveRadius;
        cb->Density = density_;
        cb->Eps = Eps;
        cb->Dt = Dt;
        cb->Wpoly6 = wpoly6_;
        cb->GWspiky = gwspiky_;

        calcCorrectPositionConstant_->Unlock();
    }

    // integrate
    std::cout << "Creating integratePipeline_" << std::endl;
    integratePipeline_ = LLGI::CreateSharedPtr(graphics->CreatePiplineState());
    integrateConstant_ = LLGI::CreateSharedPtr(
        graphics->CreateBuffer(LLGI::BufferUsageType::Constant | LLGI::BufferUsageType::MapWrite, sizeof(IntegrateCB))
    );

    integratePipeline_->SetShader(LLGI::ShaderStageType::Compute, integrateShader_.get());
    if (!integratePipeline_->Compile())
    {
        std::cerr << "failed to compile integratePipeline" << std::endl;
        abort();
    }

    {
        const auto cb = (IntegrateCB*)integrateConstant_->Lock();

        cb->Dt = Dt;

        integrateConstant_->Unlock();
    }

    // buildVBIB
    std::cout << "Creating buildVBIBPipeline_" << std::endl;
    buildVBIBPipeline_ = LLGI::CreateSharedPtr(graphics->CreatePiplineState());
    buildVBIBConstant_ = LLGI::CreateSharedPtr(
        graphics->CreateBuffer(LLGI::BufferUsageType::Constant | LLGI::BufferUsageType::MapWrite, sizeof(BuildVBIBCB))
    );

    buildVBIBPipeline_->SetShader(LLGI::ShaderStageType::Compute, buildVBIBShader_.get());
    if (!buildVBIBPipeline_->Compile())
    {
        std::cerr << "failed to compile buildVBIBPipeline" << std::endl;
        abort();
    }

    {
        const auto cb = (BuildVBIBCB*)buildVBIBConstant_->Lock();

        cb->ParticleRadius = ParticleRadius;
        cb->Color[0] = 0.2f;
        cb->Color[1] = 0.2f;
        cb->Color[2] = 1.0f;
        cb->Color[3] = 1.0f;

        cb->FixedColor[0] = 0.0f;
        cb->FixedColor[1] = 0.0f;
        cb->FixedColor[2] = 0.0f;
        cb->FixedColor[3] = 1.0f;

        buildVBIBConstant_->Unlock();
    }
}

void Fluid2D::Initialize(LLGI::Graphics* graphics, LLGI::CommandList* commandList)
{
    std::cout << "Initializing" << std::endl;

    const auto particlesInput = LLGI::CreateUniqueReference(
        graphics->CreateBuffer(
            LLGI::BufferUsageType::MapWrite | LLGI::BufferUsageType::CopySrc,
            sizeof(Particle) * ParticlesCount
        )
    );

    {
        const auto data = (Particle*)particlesInput->Lock();

        int ind = 0;
        for (int y = 0; y < 300 / (ParticleRadius * 2); y++)
        {
            for (int x = 0; x < 300 / (ParticleRadius * 2); x++)
            {
                if (x * (ParticleRadius * 2) + ParticleRadius < 8 ||
                    x * (ParticleRadius * 2) + ParticleRadius > 300 - 8 ||
                    y * (ParticleRadius * 2) + ParticleRadius < 8 ||
                    y * (ParticleRadius * 2) + ParticleRadius > 300 - 8)
                {
                    const auto posX = x * (ParticleRadius * 2) + ParticleRadius;
                    const auto posY = y * (ParticleRadius * 2) + ParticleRadius;
                    data[ind].Current[0] = posX;
                    data[ind].Current[1] = posY;
                    data[ind].Next[0] = posX;
                    data[ind].Next[1] = posY;
                    data[ind].Velocity[0] = 0;
                    data[ind].Velocity[1] = 0;
                    data[ind].Pscl = 0;
                    data[ind].IsFix = true;
                    ind++;
                }
            }
        }

        for (int i = ind; i < ParticlesCount; i++)
        {
            data[i].Current[0] = static_cast<float>(i % 50 * 4 + 10);
            data[i].Current[1] = static_cast<float>(i / 50 * 4 + 20);
            data[i].Velocity[0] = 0;
            data[i].Velocity[1] = 0;
            data[i].Pscl = 0;
        }

        particlesInput->Unlock();
    }

    commandList->Begin();
    commandList->CopyBuffer(particlesInput.get(), particles_.get());
    commandList->End();
    graphics->Execute(commandList);
    graphics->WaitFinish();
}

void Fluid2D::Update(LLGI::Graphics* graphics, LLGI::CommandList* commandList)
{
    commandList->Begin();

    commandList->BeginComputePass();

    commandList->SetPipelineState(calcExternalPipeline_.get());
    commandList->SetConstantBuffer(calcExternalConstant_.get(), 0);
    commandList->SetComputeBuffer(particles_.get(), sizeof(Particle), 0);
    commandList->Dispatch(ParticlesCount, 1, 1, 1, 1, 1);
    commandList->ResetComputeBuffer();

    for (int l = 0; l < IterationCount; l++)
    {
        commandList->SetPipelineState(buildGridPipeline_.get());
        commandList->SetConstantBuffer(buildGridConstant_.get(), 0);
        commandList->SetComputeBuffer(particles_.get(), sizeof(Particle), 0);
        commandList->SetComputeBuffer(gridTable_.get(), sizeof(int[2]), 1);
        commandList->Dispatch(ParticlesCount, 1, 1, 1, 1, 1);
        commandList->ResetComputeBuffer();

        const int nlog = static_cast<int>(std::log2f(ParticlesCount));
        int inc;

        commandList->SetPipelineState(bitonicSortPipeline_.get());
        commandList->SetComputeBuffer(gridTable_.get(), sizeof(int[2]), 0);
        for (int i = 0; i < nlog; i++)
        {
            inc = 1 << i;

            const auto dir = 2 << i;

            for (int j = 0; j < i + 1; j++)
            {
                {
                    const auto cb = (BitonicSortCB*)bitonicSortConstant_->Lock();
                    cb->Inc = inc;
                    cb->Dir = dir;
                    bitonicSortConstant_->Unlock();
                }

                commandList->SetConstantBuffer(bitonicSortConstant_.get(), 0);
                commandList->Dispatch(ParticlesCount / 2, 1, 1, 1, 1, 1);

                inc /= 2;
            }
        }
        commandList->ResetComputeBuffer();

        commandList->SetPipelineState(clearGridIndicesPipeline_.get());
        commandList->SetComputeBuffer(gridIndicesTable_.get(), sizeof(int[2]), 0);
        commandList->Dispatch(gridNum_.X * gridNum_.Y, 1, 1, 1, 1, 1);
        commandList->ResetComputeBuffer();

        commandList->SetPipelineState(buildGridIndicesPipeline_.get());
        commandList->SetConstantBuffer(buildGridIndicesConstant_.get(), 0);
        commandList->SetComputeBuffer(gridTable_.get(), sizeof(int[2]), 0);
        commandList->SetComputeBuffer(gridIndicesTable_.get(), sizeof(int[2]), 1);
        commandList->Dispatch(ParticlesCount, 1, 1, 1, 1, 1);
        commandList->ResetComputeBuffer();

        commandList->SetComputeBuffer(particles_.get(), sizeof(Particle), 0);
        commandList->SetComputeBuffer(gridTable_.get(), sizeof(int[2]), 1);
        commandList->SetComputeBuffer(gridIndicesTable_.get(), sizeof(int[2]), 2);
        {
            commandList->SetPipelineState(calcScalingFactorPipeline_.get());
            commandList->SetConstantBuffer(calcScalingFactorConstant_.get(), 0);
            commandList->Dispatch(ParticlesCount, 1, 1, 1, 1, 1);

            commandList->SetPipelineState(calcCorrectPositionPipeline_.get());
            commandList->SetConstantBuffer(calcCorrectPositionConstant_.get(), 0);
            commandList->Dispatch(ParticlesCount, 1, 1, 1, 1, 1);
        }
        commandList->ResetComputeBuffer();
    }

    commandList->SetPipelineState(integratePipeline_.get());
    commandList->SetConstantBuffer(integrateConstant_.get(), 0);
    commandList->SetComputeBuffer(particles_.get(), sizeof(Particle), 0);
    commandList->Dispatch(ParticlesCount, 1, 1, 1, 1, 1);
    commandList->ResetComputeBuffer();

    commandList->SetPipelineState(buildVBIBPipeline_.get());
    commandList->SetConstantBuffer(buildVBIBConstant_.get(), 0);
    commandList->SetComputeBuffer(particles_.get(), sizeof(Particle), 0);
    commandList->SetComputeBuffer(particleComputeVertex_.get(), sizeof(Vertex), 1);
    commandList->SetComputeBuffer(particleComputeIndex_.get(), sizeof(int), 2);
    commandList->Dispatch(ParticlesCount, 1, 1, 1, 1, 1);
    commandList->ResetComputeBuffer();

    commandList->EndComputePass();

    commandList->CopyBuffer(particleComputeVertex_.get(), particleVertex_.get());
    commandList->CopyBuffer(particleComputeIndex_.get(), particleIndex_.get());

    commandList->End();
    graphics->Execute(commandList);
    graphics->WaitFinish();
}

void Fluid2D::Render(LLGI::Graphics* graphics, LLGI::CommandList* commandList, LLGI::RenderPass* renderPass)
{
    const auto renderPassState = LLGI::CreateSharedPtr(graphics->CreateRenderPassPipelineState(renderPass));

    std::shared_ptr<LLGI::PipelineState> pipeline;
    auto itr = pipelineCache_.find(renderPassState);
    if (itr == pipelineCache_.end())
    {
        const auto pip = graphics->CreatePiplineState();
        pip->VertexLayouts[0] = LLGI::VertexLayoutFormat::R32G32B32_FLOAT;
        pip->VertexLayouts[1] = LLGI::VertexLayoutFormat::R8G8B8A8_UNORM;
        pip->VertexLayouts[3] = LLGI::VertexLayoutFormat::R32G32_FLOAT;
        pip->VertexLayouts[4] = LLGI::VertexLayoutFormat::R32G32_FLOAT;
        pip->VertexLayoutNames[0] = "POSITION";
        pip->VertexLayoutNames[1] = "COLOR";
        pip->VertexLayoutNames[2] = "UV1";
        pip->VertexLayoutNames[3] = "UV2";
        pip->VertexLayoutCount = 4;
        pip->Topology = LLGI::TopologyType::Triangle;

        pip->IsBlendEnabled = true;
        pip->BlendSrcFunc = LLGI::BlendFuncType::SrcAlpha;
        pip->BlendDstFunc = LLGI::BlendFuncType::One;
        pip->BlendSrcFuncAlpha = LLGI::BlendFuncType::One;
        pip->BlendDstFuncAlpha = LLGI::BlendFuncType::One;
        pip->BlendEquationRGB = LLGI::BlendEquationType::Max;
        pip->BlendEquationAlpha = LLGI::BlendEquationType::Max;

        pip->SetShader(LLGI::ShaderStageType::Vertex, vs_.get());
        pip->SetShader(LLGI::ShaderStageType::Pixel, ps_.get());
        pip->SetRenderPassPipelineState(renderPassState.get());
        pip->Compile();
        pipeline = LLGI::CreateSharedPtr(pip);
        pipelineCache_[renderPassState] = pipeline;
    }
    else
    {
        pipeline = itr->second;
    }

    commandList->BeginRenderPass(renderPass);

    commandList->SetVertexBuffer(particleVertex_.get(), sizeof(Vertex), 0);
    commandList->SetIndexBuffer(particleIndex_.get(), sizeof(int));
    commandList->SetPipelineState(pipeline.get());
    commandList->Draw(ParticlesCount * 2);

    commandList->EndRenderPass();
}