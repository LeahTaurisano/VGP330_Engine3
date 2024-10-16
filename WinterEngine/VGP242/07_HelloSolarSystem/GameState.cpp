#include "GameState.h"

using namespace WinterEngine;
using namespace WinterEngine::Graphics;
using namespace WinterEngine::Input;

void GameState::Initialize()
{
	mCamera.SetPosition({ 0.0f, 1.0f, -5.0f });
	mCamera.SetLookAt({ 0.0f, 0.0f, 0.0f });

	mDirectionalLight.direction = Normalize({ 1.0f, -1.0f, 1.0f });
	mDirectionalLight.ambient = { 0.3f, 0.3f, 0.3f, 1.0f };
	mDirectionalLight.diffuse = { 0.7f, 0.7f, 0.7f, 1.0f };
	mDirectionalLight.specular = { 0.9f, 0.9f, 0.9f, 1.0f };

	Mesh mesh = MeshBuilder::CreateSphere(100, 100, 1.0f);
	float xPos = 0.0f;
	for (int i = 0; i < 1; ++i)
	{
		RenderObject& planet = mPlanets.emplace_back();
		planet.meshBuffer.Initialize(mesh);
		planet.diffuseMapId = TextureCache::Get()->LoadTexture("planets/earth/earth.jpg");
		planet.normalMapId = TextureCache::Get()->LoadTexture("planets/earth/earth_normal.jpg");
		planet.specMapId = TextureCache::Get()->LoadTexture("planets/earth/earth_spec.jpg");
		planet.bumpMapId = TextureCache::Get()->LoadTexture("planets/earth/earth_bump.jpg");
		planet.transform.position.x = xPos;
		++xPos;
	}

	std::filesystem::path shaderFilePath = L"../../Assets/Shaders/Standard.fx";
	mStandardEffect.Initialize(shaderFilePath);
	mStandardEffect.SetCamera(mCamera);
	mStandardEffect.SetDirectionalLight(mDirectionalLight);
}
void GameState::Terminate()
{
	mStandardEffect.Terminate();
	for (RenderObject& planet : mPlanets)
	{
		planet.Terminate();
	}
}

void GameState::Update(float deltaTime)
{
	auto input = Input::InputSystem::Get();
	const float moveSpeed = input->IsKeyDown(KeyCode::LSHIFT) ? 10.0f : 1.0f;
	const float turnSpeed = 0.1f;
	
	if (input->IsKeyDown(KeyCode::W))
	{
		mCamera.Walk(moveSpeed * deltaTime);
	}
	else if (input->IsKeyDown(KeyCode::S))
	{
		mCamera.Walk(-moveSpeed * deltaTime);
	}
	if (input->IsKeyDown(KeyCode::D))
	{
		mCamera.Strafe(moveSpeed * deltaTime);
	}
	else if (input->IsKeyDown(KeyCode::A))
	{
		mCamera.Strafe(-moveSpeed * deltaTime);
	}
	if (input->IsKeyDown(KeyCode::E))
	{
		mCamera.Rise(moveSpeed * deltaTime);
	}
	else if (input->IsKeyDown(KeyCode::Q))
	{
		mCamera.Rise(-moveSpeed * deltaTime);
	}
	if (input->IsMouseDown(MouseButton::RBUTTON))
	{
		mCamera.Yaw(input->GetMouseMoveX() * turnSpeed * deltaTime);
		mCamera.Pitch(input->GetMouseMoveY() * turnSpeed * deltaTime);
	}
}
void GameState::Render()
{
	mStandardEffect.Begin();
	for (RenderObject& planet : mPlanets)
	{
		mStandardEffect.Render(planet);
	}
	mStandardEffect.End();
}

void GameState::DebugUI()
{
	ImGui::Begin("Debug", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
		if (ImGui::CollapsingHeader("Light", ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (ImGui::DragFloat3("Direction", &mDirectionalLight.direction.x, 0.1f))
			{
				mDirectionalLight.direction = Normalize(mDirectionalLight.direction);
			}

			ImGui::ColorEdit4("Ambient##Light", &mDirectionalLight.ambient.r);
			ImGui::ColorEdit4("Diffuse##Light", &mDirectionalLight.diffuse.r);
			ImGui::ColorEdit4("Specular##Light", &mDirectionalLight.specular.r);
		}
		if (ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::ColorEdit4("Ambient##Material", &mPlanets[0].material.ambient.r);
			ImGui::ColorEdit4("Diffuse##Material", &mPlanets[0].material.diffuse.r);
			ImGui::ColorEdit4("Specular##Material", &mPlanets[0].material.specular.r);
			ImGui::ColorEdit4("Emissive##Material", &mPlanets[0].material.emissive.r);
			ImGui::DragFloat("SpecPower##Material", &mPlanets[0].material.power, 0.01f, 0.0f, 10000.0f);
		}
		mStandardEffect.DebugUI();
	ImGui::End();
}