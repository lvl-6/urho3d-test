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

using namespace Urho3D;

class FirstApp : public Application
{
public:
	int framecount_;
	float time_;
//	SharedPtr<Text> text_;
	SharedPtr<Scene> scene_;
//	SharedPtr<Node> boxNode_;
	SharedPtr<Node> cameraNode_;

	// Constructor creates the context (before the engine is initialised), and initialises some member variables.
	FirstApp(Context* context) :
		Application(context),framecount_(0),time_(0)
	{
	}

	virtual void Setup()
	{
		// Called before engine initialization. engineParameters_ member variable can be modified here.
		#ifdef __DEBUG__ // Symbol '__DEBUG__' is usually defined with cmake.
		engineParameters_[EP_FULL_SCREEN] = false;
		engineParameters_[EP_WINDOW_RESIZABLE] = true;
		#else
		engineParameters_[EP_FULL_SCREEN] = true; // Release build has fullscreen
		#endif
		// All 'EP_' constants defined in Urho3D's /include/Urho3D/Engine/EngineDefs.h
	}

	virtual void Start()
	{
		// Called after engine initialization. Setup application & subscribe to events here.
		// Load resources
		ResourceCache* cache = GetSubsystem<ResourceCache>();

		// Use default Urho3D UI style
		GetSubsystem<UI>()->GetRoot()->SetDefaultStyle(cache->GetResource<XMLFile>("UI/DefaultStyle.xml"));

		// Setup a scene to render (MESSY SECTION!)
		scene_ = new Scene(context_);
		SharedPtr<File> sceneFile = GetSubsystem<ResourceCache>()->GetFile("Scenes/TestScene.xml");
		scene_->LoadXML(*sceneFile);
		scene_->SetName("MainScene");
		cameraNode_ = new Node(context_);
		Camera* camera = cameraNode_->CreateComponent<Camera>();
		camera->SetFarClip(300.0f);
		cameraNode_->SetPosition(Vector3(0.0f, 3.0f, -20.0f));
		SharedPtr<Viewport> viewport(new Viewport(context_, scene_, cameraNode_->GetComponent<Camera>()));
		GetSubsystem<Renderer>()->SetViewport(0, viewport);
//		XMLFile *sceneFile = cache->GetResource<XMLFile>("Scenes/TestScene.xml");
//		scene_->LoadXML(sceneFile->GetRoot());

//		GetSubsystem<Renderer>()->SetViewport(0, new Viewport(context_, scene_, Camera));

/*
		// Set up the viewport
		Renderer* renderer = GetSubsystem<Renderer>();
		SharedPtr<Viewport> viewport(new Viewport(context_, scene_, cameraNode_->GetComponent<Camera>()));
		renderer->SetViewport(0, viewport);
*///END OF MESSY SECTION

		// Subscribe to the events we want to handle (in this example, that's most of them)
		SubscribeToEvent(E_BEGINFRAME, URHO3D_HANDLER(FirstApp, HandleBeginFrame));
		SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(FirstApp, HandleKeyDown));
		SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(FirstApp, HandleUpdate));
		SubscribeToEvent(E_POSTUPDATE, URHO3D_HANDLER(FirstApp, HandlePostUpdate));
		SubscribeToEvent(E_RENDERUPDATE, URHO3D_HANDLER(FirstApp, HandleRenderUpdate));
		SubscribeToEvent(E_POSTRENDERUPDATE, URHO3D_HANDLER(FirstApp, HandlePostRenderUpdate));
		SubscribeToEvent(E_ENDFRAME, URHO3D_HANDLER(FirstApp, HandleEndFrame));
	}

	virtual void Stop()
	{
		// Perform optional cleanup after main loop has terminated
	}

	void HandleBeginFrame(StringHash eventType, VariantMap& eventData)
	{
		// Nothing to do here atm, but the name speaks for itself anyway.
	}

	void HandleKeyDown(StringHash eventType, VariantMap& eventData)
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
		// F will toggle flashlight (green light)
		if (key == KEY_F)
		{	/*
			Node* flashlight = cameraNode_->GetChild("Flashlight");
			flashlight->SetEnabled( !flashlight->IsEnabled() );
			*/
		}
	}

	void HandleClosePressed(StringHash eventType, VariantMap& eventData)
	{
		engine_->Exit();
	}

	void HandleUpdate(StringHash eventType, VariantMap& eventData)
	{
		// Non-rendering logic in here i.e. collision checks.
		float timeStep = eventData[Update::P_TIMESTEP].GetFloat();
		framecount_++;
		time_ += timeStep;
		// Movement speed as world units per second
		float MOVE_SPEED = 10.0f;
		// Mouse sensitivity as degrees per pixel
		const float MOUSE_SENSITIVITY = 0.1f;

		if(time_ >= 1)
		{
			std::string str;
			str.append("Keys: tab = toggle mouse, AWSD = move camera, Shift = fast mode, Esc = quit.\n");
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
//			text_->SetText(s);
			URHO3D_LOGINFO(s); // put stuff in the log
			framecount_ = 0;
			time_ = 0;
		}

		/*
		Input* input = GetSubsystem<Input>();

		if(input->GetKeyDown(KEY_SHIFT))
			MOVE_SPEED *= 10;
		if(input->GetKeyDown(KEY_W))
			cameraNode_->Translate(Vector3(0,0, 1) * MOVE_SPEED * timeStep);
		if(input->GetKeyDown(KEY_S))
			cameraNode_->Translate(Vector3(0,0,-1) * MOVE_SPEED * timeStep);
		if(input->GetKeyDown(KEY_A))
			cameraNode_->Translate(Vector3(-1,0,0) * MOVE_SPEED * timeStep);
		if(input->GetKeyDown(KEY_D))
			cameraNode_->Translate(Vector3( 1,0,0) * MOVE_SPEED * timeStep);

		if(!GetSubsystem<Input>()->IsMouseVisible())
		{
			// Use this frame's mouse motion to adjust camera node yaw and pitch
			IntVector2 mouseMove = input->GetMouseMove();
			static float yaw_ = 0;
			static float pitch_ = 0;
			yaw_ += MOUSE_SENSITIVITY * mouseMove.x_;
			pitch_ += MOUSE_SENSITIVITY * mouseMove.y_;
			pitch_ = Clamp(pitch_, -90.0f, 90.0f); // Clamp pitch between -90 and 90 degrees
			// Reset rotation and set yaw and pitch again
			cameraNode_->SetDirection(Vector3::FORWARD);
			cameraNode_->Yaw(yaw_);
			cameraNode_->Pitch(pitch_);
		}*/
	}

	void HandlePostUpdate(StringHash eventType, VariantMap& eventData)
	{
		// Nothing to do here.
	}

	void HandleRenderUpdate(StringHash eventType, VariantMap& eventData)
	{
		// Nothing to do here.
	}

	void HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData)
	{
		// Nothing to do here.
		//scene_->GetComponent<Octree>()->DrawDebugGeometry(true); // Show debug geometry
	}

	void HandleEndFrame(StringHash eventType, VariantMap& eventData)
	{
		// Nothing to do here.
	}

};

// Macro to start the application.
URHO3D_DEFINE_APPLICATION_MAIN(FirstApp)
