#include <WinterEngine/Inc/WinterEngine.h>

using namespace WinterEngine;
using namespace WinterEngine::Graphics;

class MainState : public AppState
{
public:
	void Initialize()
	{
		LOG("MAIN STATE INITIALIZED");
		GraphicsSystem::Get()->SetClearColor(Colors::Green);

		mLifeTime = 2.0f;
	}
	void Terminate()
	{
		LOG("MAIN STATE TERMINATED");
	}

	void Update(float deltaTime)
	{
		mLifeTime -= deltaTime;
		if (mLifeTime <= 0.0f)
		{
			App& myApp = WinterEngine::MainApp();
			myApp.ChangeState("GameState");
		}
	}

private:
	float mLifeTime = 0.0f;
};

class GameState : public AppState
{
public:
	void Initialize()
	{
		LOG("GAME STATE INITIALIZED");
		GraphicsSystem::Get()->SetClearColor(Colors::Black);
		mLifeTime = 2.0f;
	}
	void Terminate()
	{
		LOG("GAME STATE TERMINATED");
	}

	void Update(float deltaTime)
	{
		mLifeTime -= deltaTime;
		if (mLifeTime <= 0.0f)
		{
			App& myApp = WinterEngine::MainApp();
			myApp.ChangeState("MainState");
		}
	}

private:
	float mLifeTime = 0.0f;
};

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int)
{
	App& myApp = WinterEngine::MainApp();
	myApp.AddState<MainState>("MainState");
	myApp.AddState<GameState>("GameState");

	AppConfig config;
	config.appName = L"Hello Window";

	myApp.Run(config);
	return(0);
}