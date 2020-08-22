#ifndef MAIN_H
#define MAIN_H

#include <Urho3D/Engine/Application.h>
#include <Urho3D/Input/Input.h>

using namespace Urho3D;

class FirstApp : public Application
{
	// Enable type information (just copied it from Urho's Sample.h)
	URHO3D_OBJECT(FirstApp, Application);

public:
	/// Construct.
	FirstApp(Context* context);

	int framecount_;
	float time_;
	SharedPtr<Scene> scene_;
	SharedPtr<Node> cameraNode_;

	virtual void Setup();
	virtual void Start();
	virtual void Stop();

protected:
	
private:

	
	// Handle key down event
	void HandleKeyDown(StringHash eventType, VariantMap& eventData);
	// Handle key up event
	void HandleKeyUp(StringHash eventType, VariantMap& eventData);
	// Handle update
	void HandleUpdate(StringHash eventType, VariantMap& eventData);
	// Load the scene
	void LoadScene();
};

#endif
