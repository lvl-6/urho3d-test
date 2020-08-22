#include <string>
#include <sstream>

#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Engine/Application.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Engine/EngineDefs.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Input/InputEvents.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Geometry.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Skybox.h>
#include <Urho3D/Physics/PhysicsWorld.h>

#include "main.h"
#include "scene_main.h"

using namespace Urho3D;

//TestScene testScene;

// Constructor creates the context (before the engine is initialised), and initialises some member variables.
FirstApp::FirstApp(Context* context) :
	Application(context),
	framecount_(0),
	time_(0)
{
}

// Macro to start the application.
URHO3D_DEFINE_APPLICATION_MAIN(FirstApp)

void FirstApp::Setup()
{
	// Called before engine initialization. engineParameters_ member variable can be modified here.
	// All 'EP_' constants defined in Urho3D's /include/Urho3D/Engine/EngineDefs.h
	#ifdef __DEBUG__ // Symbol '__DEBUG__' is usually defined with cmake.
	engineParameters_[EP_FULL_SCREEN]	 = false;
	engineParameters_[EP_WINDOW_RESIZABLE]	 = true;
	#else
	engineParameters_[EP_FULL_SCREEN]	 = true; // Release build has fullscreen
	#endif
}

void FirstApp::Start()
{
	// Called after engine initialization. Setup application & subscribe to events here.

	// Load resources (maybe move this block to own function).
	ResourceCache* cache = GetSubsystem<ResourceCache>();
	// Use default Urho3D UI style
	GetSubsystem<UI>()->GetRoot()->SetDefaultStyle(cache->GetResource<XMLFile>("UI/DefaultStyle.xml"));
	scene_ = new Scene(context_);

	// Load the main scene
	LoadScene();

	// Subscribe to the events we want to handle (in this example, that's most of them)
	SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(FirstApp, HandleKeyDown));
	// For some reason this is throwing an "undefined reference" error.
	//SubscribeToEvent(E_KEYUP, URHO3D_HANDLER(FirstApp, HandleKeyUp));
	SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(FirstApp, HandleUpdate));
}

void FirstApp::Stop()
{
	// Perform optional cleanup after main loop has terminated
}

void FirstApp::HandleKeyDown(StringHash eventType, VariantMap& eventData)
{
	using namespace KeyDown;

	int key = eventData[P_KEY].GetInt();
	// Note the engine_ member variable for convenient access to the Engine object.
	// ESC will exit
	if (key == KEY_ESCAPE)
		engine_->Exit();
	// TAB will toggle mouse cursor visibility (visibility = opposite of IsMouseVisible())
	if (key == KEY_TAB)
	{
		GetSubsystem<Input>()->SetMouseVisible(!GetSubsystem<Input>()->IsMouseVisible());
	}
}

void FirstApp::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
	// Non-rendering logic in here i.e. collision checks.
	float timeStep = eventData[Update::P_TIMESTEP].GetFloat();
	framecount_++;
	time_ += timeStep;
	// Movement speed as world units per second
	float MOVE_SPEED = 10.0f;
	// Mouse sensitivity as degrees per pixel
	const float MOUSE_SENSITIVITY = 0.1f;

	// Draw some basic debug information
	if(time_ >= 1)
	{
		std::string str;
		str.append("Keys: AWSD = move camera, Esc = quit.\n");
		{
			std::ostringstream ss;
			ss << framecount_;
			std::string s(ss.str());
			str.append(s.substr(0, 6));
		}
		str.append(" frames in ");
		{
			std::ostringstream ss;
			ss << time_;
			std::string s(ss.str());
			str.append(s.substr(0, 6));
		}
		str.append(" seconds = ");
		{
			std::ostringstream ss;
			ss << (float)framecount_ / time_;
			std::string s(ss.str());
			str.append(s.substr(0, 6));
		}
		str.append(" fps");
		String s(str.c_str(), str.size());
		URHO3D_LOGINFO(s); // put stuff in the log
		framecount_ = 0;
		time_ = 0;
	}
}

void FirstApp::LoadScene()
{
	SharedPtr<File> sceneFile = GetSubsystem<ResourceCache>()->GetFile("Scenes/TestScene.xml");
	scene_->LoadXML(*sceneFile);
	scene_->SetName("MainScene");

	cameraNode_ = new Node(context_);
	Camera* camera = cameraNode_->CreateComponent<Camera>();
	camera->SetFarClip(300.0f);
	cameraNode_->SetPosition(Vector3(0.0f, 3.0f, -20.0f));
	SharedPtr<Viewport> viewport(new Viewport(context_, scene_, cameraNode_->GetComponent<Camera>()));
	GetSubsystem<Renderer>()->SetViewport(0, viewport);
}
