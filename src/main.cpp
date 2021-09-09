#include "engine/engine.h"

int main()
{
	app::Engine engine;
                                                    
	engine.StartupEngine();

	auto cubeMeshId = engine.AssetManager.LoadMeshInfo("res/models/cube.obj");
	auto cubeMesh = engine.AssetManager.GetMeshInfo(cubeMeshId);

	auto helmetMeshId = engine.AssetManager.LoadMeshInfo("res/models/DamagedHelmet.blend");
	auto helmetMesh = engine.AssetManager.GetMeshInfo(helmetMeshId);

	auto helmetAlbedoId = engine.AssetManager.LoadImageInfo("res/textures/helmet/Default_albedo.jpg");
	auto helmetMetalRoughnessId = engine.AssetManager.LoadImageInfo("res/textures/helmet/Default_metalRoughness.jpg");
	auto helmetAoId = engine.AssetManager.LoadImageInfo("res/textures/helmet/Default_AO.jpg");
	auto helmetNormalId = engine.AssetManager.LoadImageInfo("res/textures/helmet/Default_normal.jpg");

	auto hdrMapId = engine.AssetManager.LoadImageInfo("res/textures/hdr/Chiricahua_Plaza/GravelPlaza_REF.hdr");

	auto hdrMaterial = std::make_shared<render::HdrMaterial>();
	hdrMaterial->HdrTexture.ImageId = engine.RenderManager.GenerateCubemapFromHDR(hdrMapId, 1024);

	auto material = std::make_shared<render::PbrMaterial>();
	material->Textures.Albedo.ImageId = helmetAlbedoId;
	material->Textures.Normal.ImageId = helmetNormalId;

	material->Textures.Metallic.ImageId = helmetMetalRoughnessId;
	material->Textures.Metallic.Channels = { IC::B, IC::B, IC::B, IC::B };

	material->Textures.Roughness.ImageId = helmetMetalRoughnessId;
	material->Textures.Roughness.Channels = { IC::G, IC::G, IC::G, IC::G };

	material->Textures.Ao.ImageId = helmetAoId;


	material->Textures.IrradianceMap.ImageId = engine.RenderManager.GenerateIrradianceMap(hdrMaterial->HdrTexture.ImageId, 64);

	scene::Node rootNode;

	scene::PointLight pl;
	pl.Position = glm::vec3(0.0f, 0.0f, 10.0f);
	pl.Color = glm::vec3(250.0f);

	scene::MeshRenderable generalMesh;
	generalMesh.Info = helmetMesh;
	generalMesh.Material = material;
	generalMesh.Rotation = { 0.0f, 1.0f, 0.0f, glm::pi<float>() };

	scene::MeshRenderable cubemapMesh;
	cubemapMesh.Info = cubeMesh;
	cubemapMesh.Material = hdrMaterial;
	cubemapMesh.Render.DepthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	cubemapMesh.Render.FacesCullMode = VK_CULL_MODE_FRONT_BIT;

	rootNode.AttachChild(&pl);
	rootNode.AttachChild(&generalMesh);
	rootNode.AttachChild(&cubemapMesh);

	engine.SceneManager.SetRoot(&rootNode);


	render::Camera camera;
	camera.SetupAsPerspective(glm::vec3(0.0f, 0.0f, -5.0f), 45.0f, 1.77f, 5.0f, 0.1f, 1000.0f);

	engine.SceneManager.Register(camera);
	engine.SceneManager.SetActiveCamera(0);

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