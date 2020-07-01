#include <Urho3D/Engine/Application.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Input/InputEvents.h>

using namespace Urho3D;

class FirstApp : public Application
{
public:
	FirstApp(Context* context) :
		Application(context)
	{
	}

	virtual void Setup()
	{
		// Called before engine initialization. engineParameters_ member variable can be modified here.
	}

	virtual void Start()
	{
		// Called after engine initialization. Setup application & subscribe to events here.
		SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(FirstApp, HandleKeyDown));
	}

	virtual void Stop()
	{
		// Perform optional cleanup after main loop has terminated
	}

	void HandleKeyDown(StringHash eventType, VariantMap& eventData)
	{
		using namespace KeyDown;

		// Check for ESC pressed. Note the engine_ member variable for convenient access to the Engine object.
		// ESC is our exit key, of course.
		int key = eventData[P_KEY].GetInt();
		if (key == KEY_ESCAPE)
			engine_->Exit();
	}
	
};


URHO3D_DEFINE_APPLICATION_MAIN(FirstApp)

/*
int main()
{
	return 0;
}
*/
