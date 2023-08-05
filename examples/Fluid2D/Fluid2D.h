#pragma once
#include <LLGI.CommandList.h>
#include <LLGI.Buffer.h>
#include <LLGI.PipelineState.h>
#include <LLGI.Graphics.h>

class Fluid2D;

struct alignas(16) CalcExternalForceCB
{
    float Force[2];
    float Gravity[2];
    float Dt[2];
    int GridNum[2];
    int GridSize[2];
};

struct alignas(16) BuildGridCB
{
    int GridNum[2];
    int GridSize[2];
    int ParticlesCount;
};

struct alignas(16) BitonicSortCB
{
    int Inc;
    int Dir;
};

struct alignas(16) BuildGridIndicesCB
{
    int ParticlesCount;
};

struct alignas(16) CalcScalingFactorCB
{
    int GridNum[2];
    int GridSize[2];
    float EffectiveRadius;
    float Density;
    float Eps;
    float Dt;
    float Wpoly6;
    float GWspiky;
};

using CalcCorrectPositionCB = CalcScalingFactorCB;

struct alignas(16) IntegrateCB
{
    float Dt;
};

struct alignas(16) BuildVBIBCB
{
    float ParticleRadius;
    float Color[4];
    float FixedColor[4];
};

struct alignas(16) Vertex
{
    float Position[3];
    float Color[3];
    float UV1[2];
    float UV2[2];
};

struct alignas(16) Particle
{
    float Current[2];
    float Next[2];
    float Velocity[2];
    float Pscl;
    bool IsFix;
};

class Fluid2D
{
public:
    Fluid2D(LLGI::Graphics* graphics, LLGI::DeviceType deviceType);
    void Initialize(LLGI::Graphics* graphics, LLGI::CommandList* commandList);
    void Update(LLGI::Graphics* graphics, LLGI::CommandList* commandList);
    void Render(LLGI::Graphics* graphics, LLGI::CommandList* commandList, LLGI::RenderPass* renderPass);

private:
    std::shared_ptr<LLGI::Shader> vs_;
    std::shared_ptr<LLGI::Shader> ps_;

    std::shared_ptr<LLGI::Buffer> particleComputeIndex_;
    std::shared_ptr<LLGI::Buffer> particleComputeVertex_;
    std::shared_ptr<LLGI::Buffer> particleIndex_;
    std::shared_ptr<LLGI::Buffer> particleVertex_;
    std::shared_ptr<LLGI::Buffer> particles_;
    std::shared_ptr<LLGI::Buffer> gridTable_;
    std::shared_ptr<LLGI::Buffer> gridIndicesTable_;

    std::shared_ptr<LLGI::Shader> calcExternalShader_;
    std::shared_ptr<LLGI::Shader> buildGridShader_;
    std::shared_ptr<LLGI::Shader> bitonicSortShader_;
    std::shared_ptr<LLGI::Shader> clearGridIndicesShader_;
    std::shared_ptr<LLGI::Shader> buildGridIndicesShader_;
    std::shared_ptr<LLGI::Shader> calcScalingFactorShader_;
    std::shared_ptr<LLGI::Shader> calcCorrectPositionShader_;
    std::shared_ptr<LLGI::Shader> integrateShader_;
    std::shared_ptr<LLGI::Shader> buildVBIBShader_;

    std::shared_ptr<LLGI::Buffer> calcExternalConstant_;
    std::shared_ptr<LLGI::Buffer> buildGridConstant_;
    std::shared_ptr<LLGI::Buffer> bitonicSortConstant_;
    std::shared_ptr<LLGI::Buffer> buildGridIndicesConstant_;
    std::shared_ptr<LLGI::Buffer> calcScalingFactorConstant_;
    std::shared_ptr<LLGI::Buffer> calcCorrectPositionConstant_;
    std::shared_ptr<LLGI::Buffer> integrateConstant_;
    std::shared_ptr<LLGI::Buffer> buildVBIBConstant_;

    std::shared_ptr<LLGI::PipelineState> calcExternalPipeline_;
    std::shared_ptr<LLGI::PipelineState> buildGridPipeline_;
    std::shared_ptr<LLGI::PipelineState> bitonicSortPipeline_;
    std::shared_ptr<LLGI::PipelineState> clearGridIndicesPipeline_;
    std::shared_ptr<LLGI::PipelineState> buildGridIndicesPipeline_;
    std::shared_ptr<LLGI::PipelineState> calcScalingFactorPipeline_;
    std::shared_ptr<LLGI::PipelineState> calcCorrectPositionPipeline_;
    std::shared_ptr<LLGI::PipelineState> integratePipeline_;
    std::shared_ptr<LLGI::PipelineState> buildVBIBPipeline_;

    std::unordered_map<std::shared_ptr<LLGI::RenderPassPipelineState>, std::shared_ptr<LLGI::PipelineState>> pipelineCache_;

    LLGI::Vec2I gridNum_;
    LLGI::Vec2I gridSize_;
    float density_;
    float wpoly6_;
    float gwspiky_;

    static float CalcRestDensity(float h);

    static constexpr float Pi = 3.14159265f;
    static constexpr int IterationCount = 4;
    static constexpr float EffectiveRadius = 6.0f;
    static constexpr float ParticleRadius = 2.0f;
    static constexpr int ParticlesCount = 2048;
    static constexpr float Eps = 0.1f;
    static constexpr float Dt = 1.0f / 60.0f;
};