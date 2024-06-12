#pragma once
#include <vector>
#include <string>

#include <imgui.h>

#include "AssetStore/AssetPackage.h"

#include "SdlContainer.h"

#include "AnimationComponent.h"

#include "Debugging/Logger.h"

static std::string Label(const std::string& label, Guid guid) {
	return label + "##" + std::to_string(guid);
}

class Animator {

	std::vector<AssetPackage*> assetPackages;
	AssetFile* selectedAsset;
	AssetHandle animationHandle;
	AnimationComponent player;
	float animationDuration;
	float currentAnimationTime;

public:
	Animator() {
		selectedAsset = nullptr;
		animationHandle = AssetHandle();
		player = AnimationComponent("animationFile");
		animationDuration = 1;
		currentAnimationTime = 0;
	}
	~Animator() {
	}

	AssetPackage* GetPackage(const std::string& filePath);

	void Render();
	void PackageLoaderEditor();
	void AnimationAssetEditor(ImVec2 size);
	void AnimationPlayerEditor(ImVec2 size);
	void AnimationViewportEditor(ImVec2 size);
	bool IsAssetSelected();
	void RenderSelectedAsset();
	void RenderFrame(Frame* frame, int id);
	void RenderFrameImage(Frame* frame, int id,float fullWidth);
	int GetFrameCount();
};