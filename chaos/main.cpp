#include <app.h>
#include <gui.h>

//callback functions for GLFW
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);


const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 720;
const glm::vec2 iResolution = glm::vec2(SCR_WIDTH, SCR_HEIGHT);

bool maximized = false;
bool firstMouse = true;
double lastX = SCR_WIDTH / 2.0f;
double lastY = SCR_HEIGHT / 2.0f;

//perFrame time
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

//opacity level for changing b/w textures
float opLevel = 0.5f;

//camera
Camera camera(glm::vec3(0.0f, 2.0f, 5.0f));

struct fractal_uniforms {
	// Renderer
	int maxMarchingSteps = 1000;
	float baseMinDistance = 0.00001f;
	float minDistance = baseMinDistance;
	float maxDistance = 1000.0f;
	int minDistanceFactor = 0;
	int fractalIters = 100;
	float bailLimit = 5.0f;

	// Mandelbulb
	float power = 8.0f;
	int derivativeBias = 1;

	// Box
	int boxFoldFactor = 1;
	float boxFoldingLimit = 1.0f;

	// Sphere
	int sphereFoldFactor = 1;
	float sphereMinRadius = 0.01f;
	float sphereFixedRadius = 2.0f;
	bool sphereMinTimeVariance = false;
};

fractal_uniforms u;
Application app;
GUI gui;

int main() {
	glfwInit();
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Fractals", NULL, NULL);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);	//idk if this is VSYNC {https://github.com/glfw/glfw/issues/1321}
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, cursor_pos_callback);
	glfwSetScrollCallback(window, scroll_callback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	//glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);
	//glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	std::string vendor((char*)glGetString(GL_VENDOR));
	std::string renderer((char*)glGetString(GL_RENDERER));


	glDisable(GL_DEPTH_TEST);

	Shader shader("./shaders/fVert.glsl", "./shaders/fFrag.glsl");

	//canvas to raymarch on
	const float quad[8] = {
		-1.0f , -1.0f,
		1.0f  , -1.0f,
		-1.0f ,  1.0f,
		1.0f  ,  1.0f
	};
	//set buffers
	unsigned int VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);

	//attribs
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	gui.init(window);

	camera.Front = glm::normalize(glm::vec3(-0.01f, -0.3f, -1.0f));
	/* TO-FIX!! Attempt to extract yaw and pitch from camera.Front to prevent camera from jumping on the first mouse motion
		https://stackoverflow.com/a/60385998

		camera.Pitch = glm::degrees(-glm::asin(glm::dot(camera.Front, glm::vec3(0.0f, 1.0f, 0.0f))));
		glm::vec3 tempFront = camera.Front;
		tempFront.y = 0;
		glm::normalize(tempFront);
		camera.Yaw = glm::degrees(glm::acos(glm::dot(tempFront, glm::vec3(1.0f, 0.0f, 0.0f))));
		if (glm::dot(tempFront, glm::vec3(0.0f, 0.0f, 1.0f)) > 0.0f) camera.Yaw = 360 - camera.Yaw;

	*/

	//array of lights
	//LIGHT lights[2];
	//for (int i = 0; i < sizeof(lights) / sizeof(LIGHT); i++) {
	//	app.addLight(lights[i]);
	//}

	app.nLights = 2;
	app.lights.resize(2);

	while (!glfwWindowShouldClose(window)) {
		float currentFrame = (float)glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		app.FPSdeltaTime = currentFrame - app.FPSlastTime;
		app.nbFrames++;

		processInput(window);

		shader.use();
		shader.setFloat("iTime", (float)glfwGetTime());
		shader.setFloat("iTimeDelta", deltaTime);
		shader.setVec2("iResolution", iResolution);

		shader.setVec3("camPos", camera.Position);
		shader.setVec3("camFront", camera.Front);
		shader.setVec3("camUp", camera.Up);
		shader.setFloat("camFOV", camera.Zoom);

		shader.setInt("MAX_MARCHING_STEPS", u.maxMarchingSteps);
		shader.setFloat("MIN_DIST", u.minDistance);
		shader.setFloat("MAX_DIST", u.maxDistance);
		shader.setInt("ITERATIONS", u.fractalIters);


		app.lights[0].pos = glm::vec3(4.0 * sin((float)glfwGetTime()), 2.0, 4.0 * cos((float)glfwGetTime()));
		app.lights[0].intensity = glm::vec3(0.8);

		//app.lights[1].pos = vec3(2.0 * sin(0.37 * (float)glfwGetTime()), 2.0 * cos(0.37 * (float)glfwGetTime()), 2.0);
		app.lights[1].pos = glm::vec3(0.1 * sin(0.5 * (float)glfwGetTime()), 0.2 * (sin(0.5 * (float)glfwGetTime()) + 1) + 0.95, 0.1 * cos(0.5 * (float)glfwGetTime()));
		app.lights[1].intensity = glm::vec3(1.2, 0.2, 0.2);


		shader.setInt("nLights", app.lights.size());
		shader.setVec3("lights[0].pos", app.lights[0].pos);
		shader.setVec3("lights[0].intensity", app.lights[0].intensity);

		shader.setVec3("lights[1].pos", app.lights[1].pos);
		shader.setVec3("lights[1].intensity", app.lights[1].intensity);

		//draw
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		if (app.FPSdeltaTime >= 1.0 / 15) {//update FPS counter every 1/15th of a second
			app.fps = (float)app.nbFrames / app.FPSdeltaTime;
			app.spf = app.FPSdeltaTime * 1000 / app.nbFrames; //in milliseconds
			app.nbFrames = 0;
			app.FPSlastTime = currentFrame;
		}
		gui.render(camera, app);

		glfwSwapBuffers(window);
		glfwPollEvents();

	}
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glfwTerminate();
	return 0;

}


void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window) {
	//Q to quit
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	//maximize & minimize
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		//glfwSetWindowSize(window, 1200, 720);
		glfwRestoreWindow(window);

	}
	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
		glfwMaximizeWindow(window);
		glfwSetWindowSize(window, 1920, 1080);
	}

	//camera movement
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		camera.ProcessKeyboard(UP, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		camera.ProcessKeyboard(DOWN, deltaTime);

	//change textures
	if (glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS) {
		if (opLevel > 1.0f) opLevel = 1.0f;
		else opLevel += 0.001f;
	}
	if (glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS) {
		if (opLevel < 0.0f) opLevel = 0.0f;
		else opLevel -= 0.001f;
	}

	//Movement speed
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
		if (camera.MovementSpeed > 10.0f) camera.MovementSpeed = 10.0f;
		else camera.MovementSpeed += 0.01f;
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		if (camera.MovementSpeed < 0.5f) camera.MovementSpeed = 0.5f;
		else camera.MovementSpeed -= 0.01f;
	}

	//Reset
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
		camera.Reset();
	}

}

void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos) {
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}
	int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
	double xoffset = xpos - lastX;
	double yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
	lastX = xpos;
	lastY = ypos;
	if (state == GLFW_PRESS)
		camera.ProcessMouseMovement((float)xoffset, (float)yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	camera.ProcessMouseScroll((float)yoffset);
}