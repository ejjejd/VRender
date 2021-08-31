#include "engine/engine.h"

int main()
{
	app::Engine engine;

	engine.StartupEngine();

	auto cubeMeshId = engine.AssetManager.LoadMeshInfo("res/models/cube.obj");
	auto cubeMesh = engine.AssetManager.GetMeshInfo(cubeMeshId);

	//auto pistolMeshId = engine.AssetManager.LoadMeshInfo("res/models/pistol.fbx");
	//auto pistolMesh = engine.AssetManager.GetMeshInfo(pistolMeshId);

	//auto pistolAldeboId = engine.AssetManager.LoadImageInfo("res/textures/pistol/handgun_C.jpg");
	//auto pistolSpecId = engine.AssetManager.LoadImageInfo("res/textures/pistol/handgun_S.jpg");
	//auto pistolNormalId = engine.AssetManager.LoadImageInfo("res/textures/pistol/handgun_N.jpg");

	auto helmetMeshId = engine.AssetManager.LoadMeshInfo("res/models/DamagedHelmet.blend");
	auto helmetMesh = engine.AssetManager.GetMeshInfo(helmetMeshId);

	auto helmetAlbedoId = engine.AssetManager.LoadImageInfo("res/textures/helmet/Default_albedo.jpg");
	auto helmetMetalRoughnessId = engine.AssetManager.LoadImageInfo("res/textures/helmet/Default_metalRoughness.jpg");
	auto helmetAoId = engine.AssetManager.LoadImageInfo("res/textures/helmet/Default_AO.jpg");
	auto helmetNormalId = engine.AssetManager.LoadImageInfo("res/textures/helmet/Default_normal.jpg");

	auto hdrMapId = engine.AssetManager.LoadImageInfo("res/textures/hdr/Chiricahua_Plaza/GravelPlaza_REF.hdr");

	auto material = std::make_shared<render::PbrMaterial>();
	material->Textures.Albedo.ImageId = helmetAlbedoId;
	material->Textures.Normal.ImageId = helmetNormalId;
	material->Textures.Metallic.ImageId = helmetMetalRoughnessId;
	material->Textures.Roughness.ImageId = helmetMetalRoughnessId;
	material->Textures.Ao.ImageId = helmetAoId;

	//material->Textures.IrradianceMap.ImageId = engine.RenderManager.GenerateIrradianceMap(hdrMapId);

	auto hdrMaterial = std::make_shared<render::HdrMaterial>();
	hdrMaterial->HdrTexture.ImageId = engine.RenderManager.GenerateCubemapFromHDR(hdrMapId, 1024);

	scene::Mesh mesh;
	mesh.MeshInfo = helmetMesh;
	mesh.Material = material;
	mesh.Transform.Rotation = { 0.0f, 1.0f, 0.0f, glm::pi<float>() };

	scene::Mesh meshCubemap;
	meshCubemap.MeshInfo = cubeMesh;
	meshCubemap.Material = hdrMaterial;
	meshCubemap.Info.DepthCompareOP = VK_COMPARE_OP_LESS_OR_EQUAL;
	meshCubemap.Info.CullMode = VK_CULL_MODE_FRONT_BIT;

	engine.SceneManager.Register(mesh);
	engine.SceneManager.Register(meshCubemap);
	
	render::Camera camera;
	camera.SetupAsPerspective(glm::vec3(0.0f, 0.0f, -5.0f), 45.0f, 1.77f, 5.0f, 0.1f, 1000.0f);

	engine.SceneManager.Register(camera);
	engine.SceneManager.SetActiveCamera(0);


	scene::PointLight pl;
	pl.Position = glm::vec3(0.0f, 0.0f, 10.0f);
	pl.Color = glm::vec3(250.0f);

	engine.SceneManager.Register(pl);

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
				camera.Move(render::CameraMoveDirection::Forward, engine.DeltaTime);
			if (engine.InputManager.IsKeyStillPressed(input::Key::S))
				camera.Move(render::CameraMoveDirection::Backward, engine.DeltaTime);
			if (engine.InputManager.IsKeyStillPressed(input::Key::D))
				camera.Move(render::CameraMoveDirection::Right, engine.DeltaTime);
			if (engine.InputManager.IsKeyStillPressed(input::Key::A))
				camera.Move(render::CameraMoveDirection::Left, engine.DeltaTime);
			if (engine.InputManager.IsKeyStillPressed(input::Key::E))
				camera.Move(render::CameraMoveDirection::Up, engine.DeltaTime);
			if (engine.InputManager.IsKeyStillPressed(input::Key::Q))
				camera.Move(render::CameraMoveDirection::Down, engine.DeltaTime);
		});

	engine.CleanupEngine();

	return 0;
}