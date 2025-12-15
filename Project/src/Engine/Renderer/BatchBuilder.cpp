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
	// if alreadt batched, skip
	if (cmd.item.instanceData != nullptr) return;

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

		instanceData->guiData.emplace_back(GUIData{ cmd.item.uvRect, cmd.item.transform.GetGUIModelMatrix(cmd.item.relativePosition, cmd.item.relativeSize, cmd.item.anchorPoint, screenWidth, screenHeight) });
	}
}

void BatchBuilder::AppendInstanceData(RenderSubmission& batch, const Renderable& r)
{
	if(r.instanceData != nullptr)
	{
		// already batched, merge
		if(batch.item.layer != RenderLayer::GUI)
		{
			auto* batchData = dynamic_cast<InstanceData*>(batch.item.instanceData);
			auto* srcData = dynamic_cast<InstanceData*>(r.instanceData);

			if(batchData && srcData)
			{
				batchData->modelMatrices.reserve(batchData->modelMatrices.size() + srcData->modelMatrices.size());
				batchData->modelMatrices.insert(batchData->modelMatrices.end(), srcData->modelMatrices.begin(), srcData->modelMatrices.end());

				batch.item.instanceData->count += srcData->count;
			}
		}
		else
		{
			auto* batchData = dynamic_cast<InstanceDataGUI*>(batch.item.instanceData);
			auto* srcData = dynamic_cast<InstanceDataGUI*>(r.instanceData);

			if (batchData && srcData) {
				// Reserve and append both model matrices and uv offsets
				batchData->guiData.reserve(batchData->guiData.size() + srcData->guiData.size());
				batchData->guiData.insert(batchData->guiData.end(), srcData->guiData.begin(), srcData->guiData.end());

				batch.item.instanceData->count += srcData->count;
			}
		}
		return;
	}


	batch.item.instanceData->count += 1;
	if (batch.item.layer != RenderLayer::GUI) {
		dynamic_cast<InstanceData*>(batch.item.instanceData)->modelMatrices.emplace_back(r.transform.GetModelMatrix());
	}
	else {
		auto& win = AppAttorney::GetWindow(App::Get());
		float screenWidth = static_cast<float>(win.GetWidth());
		float screenHeight = static_cast<float>(win.GetHeight());
		auto* instanceData = dynamic_cast<InstanceDataGUI*>(batch.item.instanceData);

		instanceData->guiData.emplace_back(GUIData{r.uvRect,  r.transform.GetGUIModelMatrix(r.relativePosition, r.relativeSize, r.anchorPoint, screenWidth, screenHeight)});
	}
}
