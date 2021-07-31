#include "engine/engine.h"

bool keys[1024];

std::function<void(float, float)> cursorCallback;

void GlfwKeyCallback(GLFWwindow* w, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
		keys[key] = true;
	else if (action == GLFW_RELEASE)
		keys[key] = false;
}

void GlfwMouseCallback(GLFWwindow* w, double x, double y)
{
	static double lastX = x;
	static double lastY = y;

	float cursorDiffX = x - lastX;
	float cursorDiffY = y - lastY;

	if(cursorCallback)
		cursorCallback(cursorDiffX, cursorDiffY);

	lastX = x;
	lastY = y;
}

int main()
{
	app::Engine engine;

	engine.StartupEngine();

	glfwSetKeyCallback(engine.VulkanApp.GlfwWindow, GlfwKeyCallback);
	glfwSetCursorPosCallback(engine.VulkanApp.GlfwWindow, GlfwMouseCallback);


	auto pistolMeshId = engine.AssetManager.LoadMeshInfo("models/pistol.obj");
	auto pistolMesh = engine.AssetManager.GetMeshInfo(pistolMeshId);

	auto cubeMeshId = engine.AssetManager.LoadMeshInfo("models/cube.obj");
	auto cubeMesh = engine.AssetManager.GetMeshInfo(cubeMeshId);

	render::Mesh mesh;
	mesh.MeshInfo = cubeMesh;
	mesh.VertexShader = "shaders/vert.spv";
	mesh.FragmentShader = "shaders/frag.spv";

	engine.SceneManager.RegisterMesh(mesh);
	
	graphics::Camera camera;
	camera.SetupAsPerspective(glm::vec3(0.0f, 0.0f, -5.0f), 45.0f, 1.77f, 5.0f, 0.1f, 1000.0f);

	engine.SceneManager.RegisterCamera(camera);
	engine.SceneManager.SetActiveCamera(0);

	cursorCallback = 
		[&](const float x, const float y)
		{
			camera.AddRotation(x / engine.WindowWidth, 
							   y / engine.WindowHeight,
							   engine.DeltaTime);
		};

	engine.Run(
		[&]()
		{
			if (keys[GLFW_KEY_W])
				camera.Move(graphics::CameraMoveDirection::Forward, engine.DeltaTime);
			if (keys[GLFW_KEY_S])
				camera.Move(graphics::CameraMoveDirection::Backward, engine.DeltaTime);
			if (keys[GLFW_KEY_D])
				camera.Move(graphics::CameraMoveDirection::Right, engine.DeltaTime);
			if (keys[GLFW_KEY_A])
				camera.Move(graphics::CameraMoveDirection::Left, engine.DeltaTime);
			if (keys[GLFW_KEY_E])
				camera.Move(graphics::CameraMoveDirection::Up, engine.DeltaTime);
			if (keys[GLFW_KEY_Q])
				camera.Move(graphics::CameraMoveDirection::Down, engine.DeltaTime);
		});

	engine.CleanupEngine();

	return 0;
}