#include "Animator.h"

#include "AssetStore/AssetStore.h"

Animator::Animator() {
	selectedAsset = nullptr;
	animationHandle = AssetHandle();
	player = AnimationComponent("animationFile");
	animationDuration = 1;
	currentAnimationTime = 0;
	isPlaying = false;
}

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
	if (ImGui::Button("Create new Animation File")) {
		//TODO: new animation file
	}
	static char animationFilename[41] = "assets/AnimationData/a.anim";
	static char saveAnimationFilename[41] = "assets/AnimationData/b.anim";
	ImGui::InputText("##OpenAnimationFile", animationFilename, 41);
	ImGui::SameLine();
	if (ImGui::Button("Open")) {
		GETSYSTEM(AssetStore).LoadAnimation("animationFile", animationFilename);
		strcpy_s(saveAnimationFilename, animationFilename);
		animationHandle = GETSYSTEM(AssetStore).GetAsset("animationFile");
	}
	if (animationHandle.asset != nullptr)
	{
		ImGui::SeparatorText("Animation File");
		ImGui::InputText("##SaveAnimationFile", saveAnimationFilename, 41);
		ImGui::SameLine();
		if (ImGui::Button("Save")) {
			GETSYSTEM(AssetStore).SaveAnimation("animationFile", saveAnimationFilename);
			animationHandle = GETSYSTEM(AssetStore).GetAsset("animationFile");
		}
		auto animation = (Animation*)animationHandle.asset;
		static char fileName[31] = "Sprite Atlas";
		if (animation->texture.capacity() < 31) {
			animation->texture.reserve(31);
		}
		ImGui::InputText("Sprite atlas", &animation->texture[0], 31, ImGuiInputTextFlags_CallbackResize, ResizeStringCallback, &animation->texture);
		ImGui::InputInt2("Sprite size##spriteSizeInput", &animation->spriteFrameWidth);
		ImGui::Checkbox("Loop", &animation->isLooping);
		if (ImGui::Button("Generate Frames From atlas")) {
		}
		int i = 0;
		int addFrame = -1;
		for (auto frame : animation->frames) {
			RenderFrame(frame, i);
			i++;
			auto cursor = ImGui::GetCursorPos();
			if (ImGui::Button(("+##" + std::to_string(frame->id)).c_str(), ImVec2(size.x, 20))) {
				addFrame = i;
			}
		}
		if (addFrame != -1) {
			auto animation = (Animation*)animationHandle.asset;
			animation->frames.insert(animation->frames.begin() + addFrame, new Frame());
		}
	}
}
void Animator::RenderFrame(Frame* frame, int index)
{
	if (ImGui::BeginChild(("Frame##" + std::to_string(frame->id)).c_str(), ImVec2(0, 60), true, ImGuiWindowFlags_NoCollapse)) {
		//ImGui::PushItemWidth(50);
		ImGui::Text("Frame");
		//ImGui::PushItemWidth(50);
		ImGui::PushItemWidth(50);
		//ImGui::SameLine();
		ImGui::DragInt(("offset##" + std::to_string(frame->id)).c_str(), &frame->framePosition, 1, 0, GetFrameCount() - 1);
		ImGui::PushItemWidth(50);
		ImGui::SameLine();
		ImGui::DragFloat(("(s)##" + std::to_string(frame->id)).c_str(), &frame->frameDuration, 0.05, 0, 200);
		ImGui::PushItemWidth(100);
		ImGui::SameLine();
		auto size = ImGui::GetContentRegionAvail();
		auto pos = ImGui::GetCursorPos();
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5, 0.2, 0.2, 1));
		ImGui::SetCursorPos(ImVec2(pos.x + size.x - 20, pos.y - 18));
		if (ImGui::Button(("-##" + std::to_string(frame->id)).c_str(), ImVec2(20, 45)))
		{
			auto animation = (Animation*)animationHandle.asset;
			animation->frames.erase(animation->frames.begin() + index);
		}
		ImGui::PopStyleColor();
	}
	ImGui::EndChild();
}


void Animator::AnimationPlayerEditor(ImVec2 size)
{
	{
		ImGuiStyle& style = ImGui::GetStyle();
		float width = 0.0f;
		width += ImGui::CalcTextSize("<<").x;
		width += ImGui::CalcTextSize("<").x;
		width += ImGui::CalcTextSize("O").x;
		width += ImGui::CalcTextSize("|>").x;
		width += ImGui::CalcTextSize(">").x;
		width += ImGui::CalcTextSize(">>").x;
		width += style.ItemSpacing.x * 10;
		float off = (size.x - width) * 0.5;
		ImGui::SetCursorPosX(off);
		if (ImGui::Button("<<")) {
			currentAnimationTime = 0;
			player.SetTime(0);
			isPlaying = false;
		}
		ImGui::SameLine();
		if (ImGui::Button("<")) {
			isPlaying = false;
			auto animation = (Animation*)animationHandle.asset;
			if (animation != nullptr && (player.currentFrame > 0 ||
				(currentAnimationTime >= animationDuration && animation->frames.size() > 1)
				))
			{
				int f = player.currentFrame - 1;
				if (currentAnimationTime >= animationDuration) {
					f = animation->frames.size() - 2;
				}
				float t = 0;
				for (int i = 0; i < f; i++) {
					t += animation->frames[i]->frameDuration;
				}
				currentAnimationTime = t;
				player.SetTime(currentAnimationTime);
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("O")) {
			currentAnimationTime = 0;
			player.SetTime(0);
			isPlaying = false;
		}
		ImGui::SameLine();
		if (!isPlaying) {
			if (ImGui::Button("|>")) {
				if (currentAnimationTime >= animationDuration) {
					currentAnimationTime = 0;
					player.Reset();
				}
				isPlaying = true;
			}
		}
		else {
			if (ImGui::Button("||")) {
				isPlaying = false;
			}
		}
		ImGui::SameLine();
		if (ImGui::Button(">")) {
			isPlaying = false;
			auto animation = (Animation*)animationHandle.asset;
			if (animation != nullptr && (player.currentFrame < animation->frames.size() - 1 && currentAnimationTime < animationDuration))
			{
				int f = player.currentFrame + 1;
				float t = 0;
				for (int i = 0; i < f; i++) {
					t += animation->frames[i]->frameDuration;
				}
				currentAnimationTime = t;
				player.SetTime(currentAnimationTime);
			}
		}
		ImGui::SameLine();
		if (ImGui::Button(">>")) {
			isPlaying = false;
			auto animation = (Animation*)animationHandle.asset;
			if (animation != nullptr) {
				float t = animationDuration - animation->frames[animation->frames.size() - 1]->frameDuration;
				currentAnimationTime = t;
				player.SetTime(currentAnimationTime);
			}
		}
	}
	{
		animationDuration = player.GetAnimationDuration();
		ImGui::PushItemWidth(size.x);
		if (ImGui::SliderFloat("##Timeline", &currentAnimationTime, 0, animationDuration, "%0.2fs")) {
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
	if (texture == nullptr) {
		return;
	}
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
			if (isPlaying) {
				currentAnimationTime += 0.01666;
				player.Update(0.0166);
				if (player.isOver) {
					player.isOver = false;
					isPlaying = false;
					currentAnimationTime = animationDuration;
				}
				else {
					if (player.justFinished) {
						currentAnimationTime = 0;
					}
				}
			}
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