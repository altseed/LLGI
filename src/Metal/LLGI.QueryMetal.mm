#include "LLGI.QueryMetal.h"
#include "LLGI.CommandListMetal.h"
#include "LLGI.GraphicsMetal.h"
#include "LLGI.Metal_Impl.h"
#include "LLGI.PipelineStateMetal.h"
#include "LLGI.RenderPassMetal.h"
#include "LLGI.ShaderMetal.h"
#include "LLGI.SingleFrameMemoryPoolMetal.h"
#include "LLGI.TextureMetal.h"

#import <MetalKit/MetalKit.h>

namespace LLGI
{

QueryMetal::QueryMetal()
{
}

QueryMetal::~QueryMetal()
{
}

bool QueryMetal::Initialize(Graphics* graphics, QueryType queryType, uint32_t queryCount)
{
	queryCount_ = queryCount;

	auto device = static_cast<GraphicsMetal*>(graphics)->GetDevice();
	
	if (queryType == QueryType::Timestamp)
	{
		MTLCounterSampleBufferDescriptor *descriptor = [[MTLCounterSampleBufferDescriptor alloc] init];
		descriptor.label = @"Timestamps";
		descriptor.storageMode = MTLStorageModeShared;
		descriptor.sampleCount = queryCount;
		for (id<MTLCounterSet> counterSet in device.counterSets)
		{
			if ([counterSet.name isEqualToString:MTLCommonCounterSetTimestamp])
			{
				descriptor.counterSet = counterSet;
			}
		}
		if (descriptor.counterSet != nil)
		{
			timestampBuffer_ = [device newCounterSampleBufferWithDescriptor:descriptor error:nil];
			return (timestampBuffer_ != nil);
		}
	}
	return false;
}

uint64_t QueryMetal::GetQueryResult(uint32_t queryIndex)
{
	uint64_t result = 0;
	NSRange range = { queryIndex, 1 };
	NSData * counterData = [timestampBuffer_ resolveCounterRange:range];
	if (counterData)
	{
		memcpy(&result, [counterData bytes], sizeof(uint64_t));
	}
	return result;
}

}
