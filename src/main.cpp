#include "engine/engine.h"

int main()
{
	app::Engine engine;

	engine.StartupEngine();

	auto pistolMeshId = engine.AssetManager.LoadMeshInfo("res/models/pistol.fbx");
	auto pistolMesh = engine.AssetManager.GetMeshInfo(pistolMeshId);

	auto carlMeshId = engine.AssetManager.LoadMeshInfo("res/models/koenigsegg.fbx");
	auto carMesh = engine.AssetManager.GetMeshInfo(carlMeshId);

	auto cubeMeshId = engine.AssetManager.LoadMeshInfo("res/models/cube.obj");
	auto cubeMesh = engine.AssetManager.GetMeshInfo(cubeMeshId);


	auto material = std::make_shared<render::PbrMaterial>();
	material->Info.Albedo = glm::vec3(0.91f, 0.92f, 0.92f);
	material->Info.Metallic = 1.0f;
	material->Info.Roughness = 0.3f;
	material->Info.Ao = 1.0f;
	
	render::Mesh mesh;
	mesh.MeshInfo = pistolMesh;
	mesh.Material = material;

	engine.SceneManager.RegisterMesh(mesh);
	
	graphics::Camera camera;
	camera.SetupAsPerspective(glm::vec3(0.0f, 0.0f, -5.0f), 45.0f, 1.77f, 5.0f, 0.1f, 1000.0f);

	engine.SceneManager.RegisterCamera(camera);
	engine.SceneManager.SetActiveCamera(0);


	graphics::PointLight pl;
	pl.Position = glm::vec3(0.0f, 5.0f, 0.0f);
	pl.Color = glm::vec3(150.0f);

	engine.SceneManager.RegisterLight(pl);

	engine.Run(
		[&]()
		{
			if(engine.InputManager.IsGesturePerformed(input::Gesture::MouseX)
			   || engine.InputManager.IsGesturePerformed(input::Gesture::MouseY))
			{
				auto offset = engine.InputManager.GetCursorOffset();

				camera.AddRotation(offset.x / engine.WindowWidth,
								   offset.y / engine.WindowHeight,
								   engine.DeltaTime);
			}

			if (engine.InputManager.IsKeyStillPressed(input::Key::W))
				camera.Move(graphics::CameraMoveDirection::Forward, engine.DeltaTime);
			if (engine.InputManager.IsKeyStillPressed(input::Key::S))
				camera.Move(graphics::CameraMoveDirection::Backward, engine.DeltaTime);
			if (engine.InputManager.IsKeyStillPressed(input::Key::D))
				camera.Move(graphics::CameraMoveDirection::Right, engine.DeltaTime);
			if (engine.InputManager.IsKeyStillPressed(input::Key::A))
				camera.Move(graphics::CameraMoveDirection::Left, engine.DeltaTime);
			if (engine.InputManager.IsKeyStillPressed(input::Key::E))
				camera.Move(graphics::CameraMoveDirection::Up, engine.DeltaTime);
			if (engine.InputManager.IsKeyStillPressed(input::Key::Q))
				camera.Move(graphics::CameraMoveDirection::Down, engine.DeltaTime);
		});

	engine.CleanupEngine();

	return 0;
}