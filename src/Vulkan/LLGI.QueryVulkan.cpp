#include "LLGI.QueryVulkan.h"
#include "LLGI.SingleFrameMemoryPoolVulkan.h"

namespace LLGI
{

QueryVulkan::QueryVulkan() {}

QueryVulkan::~QueryVulkan()
{
	if (queryPool_)
	{
		graphics_->GetDevice().destroyQueryPool(queryPool_);
		queryPool_ = nullptr;
	}
}

bool QueryVulkan::Initialize(GraphicsVulkan* graphics, QueryType queryType, uint32_t queryCount)
{
	graphics_ = CreateSharedPtr(graphics);
	queryType_ = queryType;
	queryCount_ = queryCount;

	vk::QueryPoolCreateInfo queryInfo{};
	queryInfo.sType = vk::StructureType::eQueryPoolCreateInfo;
	queryInfo.queryCount = static_cast<uint32_t>(queryCount);

	switch (queryType)
	{
	case QueryType::Timestamp:
		queryInfo.queryType = vk::QueryType::eTimestamp;
		break;
	case QueryType::Occulusion:
		queryInfo.queryType = vk::QueryType::eOcclusion;
		break;
	default:
		return false;
	}

	queryPool_ = graphics->GetDevice().createQueryPool(queryInfo);

	return true;
}

uint64_t QueryVulkan::GetQueryResult(uint32_t queryIndex)
{
	auto resultValue = graphics_->GetDevice().getQueryPoolResult<uint64_t>(
		queryPool_, queryIndex, 1, sizeof(uint64_t),
		vk::QueryResultFlagBits::e64 | vk::QueryResultFlagBits::eWait);

	if (resultValue.result == vk::Result::eSuccess)
	{
		return resultValue.value;
	}

	return 0;
}

} // namespace LLGI
