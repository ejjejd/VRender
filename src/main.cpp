#include "engine/engine.h"



int main()
{
	app::Engine engine;

	engine.StartupEngine();

	auto cubeMeshId = engine.AssetManager.LoadMeshInfo("models/pistol.obj");
	auto cubeMesh = engine.AssetManager.GetMeshInfo(cubeMeshId);

	render::Mesh mesh;
	mesh.MeshInfo = cubeMesh;
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