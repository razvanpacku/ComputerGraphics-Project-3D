#include <Engine/App.h>
#include "Demo/TestScene.h"

// ======================================================
// To create custom behavior, derive from Scene and implement your logic there
// =====================================================

int main(int argc, char* argv[])
{
	App::Init(static_cast<int32_t>(argc), argv);
	App& app = App::Get("Rocket");
	app.SetScene(new TestScene());		// Set your custom scene here
	app.Run();
}

