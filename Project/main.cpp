#include <Engine/App.h>

int main(int argc, char* argv[])
{
	App::Init(static_cast<int32_t>(argc), argv);
	App& app = App::Get("Rocket");
	app.Run();
}

