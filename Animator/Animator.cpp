#include "Animator.h"

#include "AssetStore/AssetStore.h"

AssetPackage* Animator::GetPackage(const std::string& filePath) {
	for (auto pkg : assetPackages) {
		if (pkg->filePath == filePath) {
			return pkg;
		}
	}
	return nullptr;
}

void Animator::Render()
{
	int w, h;
	auto window = GETSYSTEM(SdlContainer).GetWindow();
	SDL_GetWindowSize(window, &w, &h);
	ImGui::Begin("Package Loader", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
	ImGui::SetWindowSize(ImVec2(w / 4, h));
	ImGui::SetWindowPos(ImVec2(0, 0));
	PackageLoaderEditor();
	ImGui::End();

	ImGui::Begin("Animation Asset", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
	ImGui::SetWindowSize(ImVec2(w / 4, h));
	ImGui::SetWindowPos(ImVec2(3 * w / 4, 0));
	AnimationAssetEditor(ImVec2(w / 4, h));
	ImGui::End();

	ImGui::Begin("Animation Player", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
	ImGui::SetWindowSize(ImVec2(w / 2, h / 3));
	ImGui::SetWindowPos(ImVec2(w / 4, h - h / 3));
	AnimationPlayerEditor(ImVec2(w / 2, h / 3));
	ImGui::End();

	ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
	ImGui::SetWindowSize(ImVec2(w / 2, 2 * h / 3));
	ImGui::SetWindowPos(ImVec2(w / 4, 0));
	AnimationViewportEditor(ImVec2(w / 2, 2 * h / 3));
	ImGui::End();
}

void Animator::PackageLoaderEditor()
{
	static char fileName[31] = "assets/Packages/a.pkg";
	ImGui::InputText("##OpenAssetPackageFileName", fileName, 31);
	if (ImGui::Button("Load Asset Package")) {
		auto pkg = GetPackage(fileName);
		if (pkg != nullptr) {
			Logger::Log("Reloading Asset Package");
			assetPackages.erase(std::find(assetPackages.begin(), assetPackages.end(), pkg));
			delete pkg;
		}
		pkg = new AssetPackage();
		if (pkg->Load(fileName)) {
			assetPackages.push_back(pkg);
			GETSYSTEM(AssetStore).LoadPackage(fileName);
			Logger::Log("Asset package Opened");
		}
		else {
			delete pkg;
			Logger::Log("Failed to open asset package " + std::string(fileName));
		}
	}

	//ImGui::SameLine();

	for (auto package : assetPackages) {
		if (ImGui::CollapsingHeader(Label("Package", package->guid).c_str())) {
			ImGui::Indent();
			if (ImGui::Button(Label("Close", package->guid).c_str())) {
				if (selectedAsset != nullptr && package->ContainsAsset(selectedAsset)) {
					selectedAsset = nullptr;
				}
				assetPackages.erase(std::find(assetPackages.begin(), assetPackages.end(), package));
				delete package;
				continue;
			}
			if (ImGui::BeginListBox(Label("Textures", package->guid).c_str())) {
				for (auto assetFile : package->assets) {
					if (assetFile->assetType != AssetType::Texture) {
						continue;
					}
					bool selected = selectedAsset == assetFile;
					if (ImGui::Selectable((assetFile->metaData->name + " :" + std::to_string(assetFile->guid)).c_str(), &selected)) {
						selectedAsset = assetFile;
					}
				}
				ImGui::EndListBox();
				if (ImGui::IsItemClicked()) {
					selectedAsset = nullptr;
				}
			}
			ImGui::Unindent();
		}
	}
	if (selectedAsset != nullptr) {
		if (ImGui::BeginChild("Preview", ImVec2(280, 280), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {
			auto asset = (TextureAsset*)(GETSYSTEM(AssetStore).GetAsset(selectedAsset->metaData->name).asset);
			auto windowWidth = ImGui::GetWindowSize().x;
			auto windowHeight = ImGui::GetWindowSize().y;

			ImGui::SetCursorPosX((windowWidth - 260) * 0.5f);
			ImGui::SetCursorPosY((windowHeight - 260) * 0.5f);
			ImGui::Image(asset->texture, ImVec2(260, 260), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), ImVec4(0.5, 0.5, 0.5, 0.8));
		}
		ImGui::EndChild();
	}
}

void Animator::AnimationAssetEditor(ImVec2 size)
{
	if (ImGui::Button("New Animation File")) {
		//TODO: new animation file
	}
	static char animationFilename[41] = "assets/AnimationData/a.anim";
	ImGui::InputText("##OpenAnimationFile", animationFilename, 41);
	if (ImGui::Button("Open Animation File")) {
		GETSYSTEM(AssetStore).LoadAnimation("animationFile", animationFilename);
		animationHandle = GETSYSTEM(AssetStore).GetAsset("animationFile");
	}
	if (animationHandle.asset != nullptr)
	{
		auto animation = (Animation*)animationHandle.asset;
		static char fileName[31] = "Sprite Atlas";
		if (animation->texture.capacity() < 31) {
			animation->texture.reserve(31);
		}
		ImGui::InputText("Sprite atlas", &animation->texture[0], 31, ImGuiInputTextFlags_CallbackResize, ResizeStringCallback, &animation->texture);
		ImGui::InputInt2("Sprite size##spriteSizeInput", &animation->spriteFrameWidth);
		if (ImGui::Button("Generate Frames")) {
		}
		int i = 0;
		for (auto frame : animation->frames) {
			RenderFrame(frame, i);
			i++;
		}
		if (ImGui::Button("Add Frame")) {
		}
	}
}
void Animator::RenderFrame(Frame* frame, int id)
{
	if (ImGui::BeginChild(("Frame##" + std::to_string(id)).c_str(), ImVec2(0, 100), true, ImGuiWindowFlags_NoCollapse)) {
		ImGui::Text("Frame");
		ImGui::DragInt(("Sprite offset##" + std::to_string(id)).c_str(), &frame->framePosition, 1, 0, GetFrameCount() - 1);
		ImGui::DragFloat(("Duration(s)##" + std::to_string(id)).c_str(), &frame->frameDuration, 0.05, 0, 200);
		if (ImGui::Button(("Delete##" + std::to_string(id)).c_str())) {

		}
	}
	ImGui::EndChild();
}


void Animator::AnimationPlayerEditor(ImVec2 size)
{
	{
		ImGui::Button("First frame");
		ImGui::SameLine();
		ImGui::Button("Prev frame");
		ImGui::SameLine();
		ImGui::Button("Stop");
		ImGui::SameLine();
		ImGui::Button("Play");
		ImGui::SameLine();
		ImGui::Button("Next frame");
		ImGui::SameLine();
		ImGui::Button("Last frame");
	}
	{
		animationDuration = player.GetAnimationDuration();
		ImGui::PushItemWidth(size.x);
		if (ImGui::SliderFloat("##Timeline", &currentAnimationTime, 0, animationDuration)) {
			player.SetTime(currentAnimationTime);
		}
	}
	{
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::PushStyleVar(ImGuiStyleVar_SeparatorTextPadding, ImVec2(0, 0));
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0);
		if (animationHandle.asset != nullptr)
		{
			auto animation = (Animation*)animationHandle.asset;
			int i = 0;
			for (auto frame : animation->frames) {
				RenderFrameImage(frame, i, size.x);
				if (i + 1 < animation->frames.size()) {
					ImGui::SameLine();
				}
				i++;
			}
		}
		ImGui::PopStyleVar(6);
	}
}
void Animator::RenderFrameImage(Frame* frame, int id, float fullWidth)
{
	fullWidth -= 20;
	auto animation = (Animation*)animationHandle.asset;
	auto sourceRect = SDL_Rect(animation->GetSourceRect(id));
	auto texture = (TextureAsset*)GETSYSTEM(AssetStore).GetAsset(animation->texture).asset;
	int texW, texH;
	SDL_QueryTexture(texture->texture, nullptr, nullptr, &texW, &texH);
	auto uv0 = ImVec2((float)sourceRect.x / texW, (float)sourceRect.y / texH);
	auto uv1 = ImVec2(((float)sourceRect.x + sourceRect.w) / texW, ((float)sourceRect.y + sourceRect.h) / texH);
	if (texture != nullptr && texture->texture != nullptr) {
		auto windowWidth = ImGui::GetWindowSize().x;
		auto windowHeight = ImGui::GetWindowSize().y;
		ImGui::Image(texture->texture, ImVec2(fullWidth * (frame->frameDuration / animationDuration), 100), uv0, uv1, ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 1));
	}
}

void Animator::AnimationViewportEditor(ImVec2 size)
{
	auto animation = (Animation*)animationHandle.asset;
	if (animation != nullptr) {
		auto sourceRect = SDL_Rect(animation->GetSourceRect(player.currentFrame));
		auto texture = (TextureAsset*)GETSYSTEM(AssetStore).GetAsset(animation->texture).asset;
		if (texture != nullptr) {
			currentAnimationTime += 0.01666;
			if (currentAnimationTime > animationDuration) {
				currentAnimationTime = 0;
			}
			player.Update(0.0166);
			int texW, texH;
			SDL_QueryTexture(texture->texture, nullptr, nullptr, &texW, &texH);
			auto uv0 = ImVec2((float)sourceRect.x / texW, (float)sourceRect.y / texH);
			auto uv1 = ImVec2(((float)sourceRect.x + sourceRect.w) / texW, ((float)sourceRect.y + sourceRect.h) / texH);
			if (texture != nullptr && texture->texture != nullptr) {
				auto windowWidth = ImGui::GetWindowSize().x;
				auto windowHeight = ImGui::GetWindowSize().y;

				ImGui::SetCursorPosX((windowWidth - 400) * 0.5f);
				ImGui::SetCursorPosY((windowHeight - 400) * 0.5f);
				ImGui::Image(texture->texture, ImVec2(400, 400), uv0, uv1, ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 1));
			}
		}
	}
}

bool Animator::IsAssetSelected() {
	return selectedAsset != nullptr;
}

void Animator::RenderSelectedAsset() {
	selectedAsset->Editor();
}

int Animator::GetFrameCount() {
	auto animation = (Animation*)animationHandle.asset;
	auto texture = (TextureAsset*)GETSYSTEM(AssetStore).GetAsset(animation->texture).asset;
	if (texture != nullptr && texture->texture != nullptr) {
		int texW, texH;
		SDL_QueryTexture(texture->texture, nullptr, nullptr, &texW, &texH);
		return texW / animation->spriteFrameWidth;
	}
	return 50;
}