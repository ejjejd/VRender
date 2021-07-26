#include "engine/engine.h"

int main()
{
	app::Engine engine;

	engine.StartupEngine();

	render::Mesh mesh;
	mesh.Positions =
	{
		{ -0.5f, 0.5f, 0.0f },
		{ 0.0f, -0.5f, 0.0f },
		{ 0.5f, 0.5f, 0.0f }
	};
	mesh.VertexShader = "shaders/vert.spv";
	mesh.FragmentShader = "shaders/frag.spv";

	engine.SceneManager.RegisterMesh(mesh);

	engine.Run(
		[&]()
		{

		});

	engine.CleanupEngine();

	return 0;
}