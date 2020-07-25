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
	SharedPtr<Text> text_;
	SharedPtr<Scene> scene_;
	SharedPtr<Node> boxNode_;
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

		// Create some text (this kinda stuff should probably be scene-specific or something)
		text_ = new Text(context_);
		text_->SetText("Keys: tab = toggle mouse, AWSD = move camera, Shift = fast mode, Esc = quit.\n"
				"Wait a bit to see FPS.");
		text_->SetFont(cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"), 20);
		text_->SetColor(Color(0.3, 0, 0.3));
		text_->SetHorizontalAlignment(HA_CENTER);
		text_->SetVerticalAlignment(VA_TOP);
		GetSubsystem<UI>()->GetRoot()->AddChild(text_);

		// Add a button (same note as above BTW)
		Button* button = new Button(context_);
		// Button must be part of the UI system before SetSize calls!
		GetSubsystem<UI>()->GetRoot()->AddChild(button);
		button->SetName("Button Quit");
		button->SetStyle("Button");
		button->SetSize(32, 32);
		button->SetPosition(16, 116);
		// Subscribe to button release event
		SubscribeToEvent(button, E_RELEASED, URHO3D_HANDLER(FirstApp, HandleClosePressed));

		// Setup a scene to render
		scene_ = new Scene(context_);
		// Give the scene an Octree component
		scene_->CreateComponent<Octree>();
		// Create PhysicsWorld component for physics components (i.e. RigidBody, CollisionShape).
		scene_->CreateComponent<PhysicsWorld>();
		// Add an additional component (DebugRenderer)
		scene_->CreateComponent<DebugRenderer>();

		// Add a skybox
		Node* skyNode = scene_->CreateChild("Sky");
		skyNode->SetScale(500.0f); // apparently does not matter
		Skybox* skybox = skyNode->CreateComponent<Skybox>();
		skybox->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
		skybox->SetMaterial(cache->GetResource<Material>("Materials/Skybox.xml"));

		// Add a geometrical box
		boxNode_ = scene_->CreateChild("Box");
		boxNode_->SetPosition(Vector3(0, 2, 15));
		boxNode_->SetScale(Vector3(3, 3, 3));
		StaticModel* boxObject = boxNode_->CreateComponent<StaticModel>();
		boxObject->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
		boxObject->SetMaterial(cache->GetResource<Material>("Materials/Stone.xml"));
		boxObject->SetCastShadows(true);

		// Create 400 similar boxes in a grid
		for(int x=-30; x<30; x+=3)
			for(int z=0; z<60; z+=3)
			{
				Node* boxNode_ = scene_->CreateChild("Box");
				boxNode_->SetPosition(Vector3(x, -3, z));
				boxNode_->SetScale(Vector3(2, 2, 2));
				StaticModel* boxObject = boxNode_->CreateComponent<StaticModel>();
				boxObject->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
				boxObject->SetMaterial(cache->GetResource<Material>("Materials/Stone.xml"));
				boxObject->SetCastShadows(true);
			}

		// Create a camera from which the viewpoint can render
		cameraNode_ = scene_->CreateChild("Camera");
		Camera* camera = cameraNode_->CreateComponent<Camera>();
		camera->SetFarClip(2000);

		// Create a red directional light (sun)
		{
			Node* lightNode = scene_->CreateChild();
			lightNode->SetDirection(Vector3::FORWARD);
			lightNode->Yaw(50); // horizontal
			lightNode->Pitch(10); // vertical
			Light* light = lightNode->CreateComponent<Light>();
			light->SetLightType(LIGHT_DIRECTIONAL);
			light->SetBrightness(1.6);
			light->SetColor(Color(1.0, 0.6, 0.3, 1));
			light->SetCastShadows(true);
		}

		// Create a blue point light
		{
			Node* lightNode = scene_->CreateChild("Blue_Light");
			lightNode->SetPosition(Vector3(-10, 2, 5));
			Light* light = lightNode->CreateComponent<Light>();
			light->SetLightType(LIGHT_POINT);
			light->SetRange(25);
			light->SetBrightness(1.7);
			light->SetColor(Color(0.5, 0.5, 1.0, 1));
			light->SetCastShadows(true);
		}

		// Add green spot light to the camera node
		{
			Node* node_light = cameraNode_->CreateChild("Flashlight");
			Light* light = node_light->CreateComponent<Light>();
			node_light->Pitch(15); // point slightly downward
			light->SetLightType(LIGHT_SPOT);
			light->SetRange(20);
			light->SetColor(Color(0.6, 1, 0.6, 1.0));
			light->SetBrightness(2.8);
			light->SetFov(25);
		}

		// Set up the viewport
		Renderer* renderer = GetSubsystem<Renderer>();
		SharedPtr<Viewport> viewport(new Viewport(context_, scene_, cameraNode_->GetComponent<Camera>()));
		renderer->SetViewport(0, viewport);

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
		{
			Node* flashlight = cameraNode_->GetChild("Flashlight");
			flashlight->SetEnabled( !flashlight->IsEnabled() );
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
			text_->SetText(s);
			URHO3D_LOGINFO(s); // put stuff in the log
			framecount_ = 0;
			time_ = 0;
		}

		// Rotate the box - this would be better done with a LogicComponent,
		// which makes it easier to control things like movement/animation.
		boxNode_->Rotate(Quaternion(8 * timeStep, 16 * timeStep, 0));

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
		}
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
