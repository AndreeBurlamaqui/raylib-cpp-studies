// Resolve Windows/Raylib CloseWindow conflict
#if defined(_WIN32)
#define NOGDI             // All of these macros help prevent 
#define NOUSER            // namespace pollution from Windows.h
#define WIN32_LEAN_AND_MEAN
#endif

#include "raylib.h"
#include "resource_dir.h"	// utility header for SearchAndSetResourceDir
#include <vector>

#include "raylib.h"
#include "rlgl.h" // Required to access internal Raylib GL state

#include <NoesisPCH.h>
#include <NsCore/Noesis.h>
#include <NsGui/IRenderer.h>
#include <NsGui/IView.h>
#include <NsGui/Grid.h>
#include <NsApp/LocalXamlProvider.h>
#include <NsApp/LocalFontProvider.h>

// You must include the GLRenderDevice provided in the Noesis SDK Samples
#include "GLRenderDevice.h"

namespace Ns = Noesis;
namespace NsApp = NoesisApp;

// Forward declarations
class IInputHandler;  // Forward declare IInputHandler
class InputSystem;    // Forward declare InputSystem

class IGameObject {
private:
	virtual ~IGameObject() = default;

protected:
	friend int main(); // Only main can call protected
	virtual void Update(const float& deltaTime) = 0;
	virtual void Draw() const = 0;
};

class IInputHandler {
private:
	virtual ~IInputHandler() = default;

protected:
	friend InputSystem; // Only input manager can call protected
	virtual void InputPoll(const float& deltaTime) = 0;
};

class InputSystem {
	friend int main(); // Only main can instantiate InputSystem

public:
	static InputSystem& Get() {
		return GetInstance();
	}

	void AddHandler(IInputHandler* handler) {
		handlers.push_back(handler);
	}

	void RemoveHandler(IInputHandler* handler) {
		handlers.erase(std::remove(handlers.begin(), handlers.end(), handler), handlers.end());
	}

private:
	InputSystem() = default; // Private constructor

	// Delete copy constructor and assignment operator to prevent copies
	InputSystem(const InputSystem&) = delete;
	InputSystem& operator=(const InputSystem&) = delete;

	// subscribed handlers
	std::vector<IInputHandler*> handlers;

	static InputSystem& GetInstance() {
		static InputSystem instance;
		return instance;
	}

protected:
	friend int main(); // Only main can call protected
	void Poll(const float& deltaTime) {
		for (auto* handler : handlers) {
			handler->InputPoll(deltaTime);
		}
	}
};

int main ()
{
	// Setup project and windows
	const char* projectName = "QUICKSTART";
	const int initalScreenWidth = 800;
	const int initalScreenHeight = 600;
	InitWindow(initalScreenWidth, initalScreenHeight, projectName);
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI); // Use vsync and work on high DPI displays
	SetTargetFPS(60);

	// Utility function from resource_dir.h to find the resources folder and set it as the current working directory so we can load from it
	SearchAndSetResourceDir("resources");

	// Initiate Noesis
	Noesis::SetLogHandler([](const char*, uint32_t, uint32_t level, const char*, const char* msg)
		{
			// [TRACE] [DEBUG] [INFO] [WARNING] [ERROR]
			const char* prefixes[] = { "T", "D", "I", "W", "E" };
			printf("[NOESIS/%s] %s\n", prefixes[level], msg);
		});

	Noesis::GUI::Init();
	Ns::GUI::SetXamlProvider(Ns::MakePtr <NsApp::LocalXamlProvider>("../build/Plugins/NoesisGUI/Data"));
	Ns::Ptr<Ns::RenderDevice> device = *new NsApp::GLRenderDevice();

	Ns::Ptr<Ns::FrameworkElement> root = Ns::GUI::LoadXaml<Ns::FrameworkElement>("Reflections.xaml");
	Ns::Ptr<Ns::IView> view = Ns::GUI::CreateView(root);

	view->GetRenderer()->Init(device);
	view->SetSize(initalScreenWidth, initalScreenHeight);

	InputSystem& inputSystem = InputSystem::Get();

	std::vector<IGameObject*> gameObjects; // TODO: Add game loop to add game objects at runtime easily
	
	while (!WindowShouldClose()) // Game Loop until the user presses ESCAPE or the Close button on the window
	{
		const float deltaTime = GetFrameTime();

		// Following basic game loop structure
		// 1. Input
		inputSystem.Poll(deltaTime);

		// 2. Update
		view->Update(GetTime());
		for (auto* gameObject : gameObjects) {
			gameObject->Update(deltaTime);
		}

		// 3. Draw
		BeginDrawing();

		// Game world
		ClearBackground(BLACK);
		for (const auto* gameObject : gameObjects) {
			gameObject->Draw();
		}
		
		// UI
		rlDrawRenderBatchActive(); // Force Raylib to finish its work
		view->GetRenderer()->UpdateRenderTree();
		view->GetRenderer()->Render();

		EndDrawing();

	}

	view->GetRenderer()->Shutdown();
	Noesis::GUI::Shutdown();
	CloseWindow();
	return 0;
}