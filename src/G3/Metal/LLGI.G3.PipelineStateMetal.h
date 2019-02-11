#pragma once

#include "../LLGI.G3.PipelineState.h"

namespace LLGI
{
namespace G3
{

class PipelineStateMetal
    : public PipelineState
    {
    private:
    public:
        PipelineStateMetal() = default;
        virtual ~PipelineStateMetal() = default;
        
        void SetShader(ShaderStageType stage, Shader* shader) override;
        void Compile() override;
    };
    

    
}
}
