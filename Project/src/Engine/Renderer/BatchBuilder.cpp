#include "Engine/Renderer/BatchBuilder.h"

#include "Engine/App.h"
#include "Engine/Window.h"

std::vector<RenderSubmission> BatchBuilder::Build(std::vector<RenderSubmission>& sorted)
{
	std::vector<RenderSubmission> batched;
	if (sorted.empty()) return batched;

	size_t batchStart = 0;
	RenderSubmission& current = sorted[0];
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
	if(cmd.item.layer != RenderLayer::GUI) cmd.item.instanceData = new InstanceData();
	else cmd.item.instanceData = new InstanceDataGUI();
	cmd.item.instanceData->count = 1;
	if (cmd.item.layer != RenderLayer::GUI) {
		dynamic_cast<InstanceData*>(cmd.item.instanceData)->modelMatrices.emplace_back(cmd.item.transform.GetModelMatrix());
	}
	else {
		auto& win = AppAttorney::GetWindow(App::Get());
		float screenWidth = static_cast<float>(win.GetWidth());
		float screenHeight = static_cast<float>(win.GetHeight());
		auto* instanceData = dynamic_cast<InstanceDataGUI*>(cmd.item.instanceData);

		instanceData->modelMatrices.emplace_back(cmd.item.transform.GetGUIModelMatrix(cmd.item.relativePosition, cmd.item.relativeSize, cmd.item.anchorPoint, screenWidth, screenHeight));
		instanceData->uvOffsets.emplace_back(cmd.item.uvRect);
	}
}

void BatchBuilder::AppendInstanceData(RenderSubmission& batch, const Renderable& r)
{
	batch.item.instanceData->count += 1;
	if (batch.item.layer != RenderLayer::GUI) {
		dynamic_cast<InstanceData*>(batch.item.instanceData)->modelMatrices.emplace_back(r.transform.GetModelMatrix());
	}
	else {
		auto& win = AppAttorney::GetWindow(App::Get());
		float screenWidth = static_cast<float>(win.GetWidth());
		float screenHeight = static_cast<float>(win.GetHeight());
		auto* instanceData = dynamic_cast<InstanceDataGUI*>(batch.item.instanceData);

		instanceData->modelMatrices.emplace_back(r.transform.GetGUIModelMatrix(r.relativePosition, r.relativeSize, r.anchorPoint, screenWidth, screenHeight));
		instanceData->uvOffsets.emplace_back(r.uvRect);
	}
}
