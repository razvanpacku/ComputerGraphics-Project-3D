#include "Engine/Renderer/BatchBuilder.h"

std::vector<RenderSubmission> BatchBuilder::Build(const std::vector<RenderSubmission>& sorted)
{
	std::vector<RenderSubmission> batched;
	batched.reserve(sorted.size());
	if (sorted.empty()) return batched;

	size_t batchStart = 0;
	RenderSubmission current = sorted[0];
	InitInstanceData(current);

	if(sorted.size() == 1)
	{
		batched.push_back(std::move(current));
		return batched;
	}

	uint64_t currentKey = current.sortKey;

	for(size_t i = 1; i < sorted.size(); ++i)
	{
		uint64_t nextKey = sorted[i].sortKey;
		const RenderSubmission& next = sorted[i];
		if (currentKey == nextKey)
		{
			AppendInstanceData(current, next.item);
		}
		else
		{
			batched.push_back(std::move(current));

			current = next;
			InitInstanceData(current);
			currentKey = nextKey;
			batchStart = i;
		}
	}
	// push the last batch
	batched.push_back(std::move(current));

	return batched;
}

void BatchBuilder::InitInstanceData(RenderSubmission& cmd)
{
	cmd.item.instanceData = new InstanceData();
	cmd.item.instanceData->count = 1;

	cmd.item.instanceData->modelMatrices.push_back(cmd.item.transform.GetModelMatrix());
}

void BatchBuilder::AppendInstanceData(RenderSubmission& batch, const Renderable& r)
{
	batch.item.instanceData->count += 1;
	batch.item.instanceData->modelMatrices.push_back(r.transform.GetModelMatrix());
}
