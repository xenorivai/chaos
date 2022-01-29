#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <shader.h>
#include <camera.h>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 720;
const glm::vec2 iResolution = glm::vec2(SCR_WIDTH, SCR_HEIGHT);

bool maximized = false;


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

//camera
Camera camera(glm::vec3(0.0f, 0.0f, 5.0f));
bool firstMouse = true;
double lastX = SCR_WIDTH / 2.0f;
double lastY = SCR_HEIGHT / 2.0f;

//perFrame time
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

//opacity level for changing b/w textures
float opLevel = 0.5f;

const float quad[8] = {
	-1.0f, -1.0f,
	1.0f, -1.0f,
	-1.0f,1.0f,
	1.0f,1.0f
};

int main() {
	glfwInit();
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Fractals", NULL, NULL);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glDisable(GL_DEPTH_TEST);

	Shader shader("./shaders/fVert.glsl", "./shaders/fFrag.glsl");

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

	while (!glfwWindowShouldClose(window)) {
		float currentFrame = (float)glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window);

		shader.use();
		shader.setFloat("iTime", (float)glfwGetTime());
		shader.setFloat("iTimeDelta", deltaTime);
		shader.setVec2("iResolution", iResolution);

		shader.setVec3("camPos", camera.Position);
		shader.setVec3("camFront", camera.Front);
		shader.setVec3("camUp", camera.Up);
		shader.setFloat("camFOV", camera.Zoom);
		//draw
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
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

	//Mouse Sensi
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
		if (camera.MouseSensitivity > 1.0f) camera.MouseSensitivity = 0.5f;
		else camera.MouseSensitivity += 0.001f;
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		if (camera.MouseSensitivity < 0.0f) camera.MouseSensitivity = 0.01f;
		else camera.MouseSensitivity -= 0.001f;
	}

	//Reset
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
		camera.Reset();
	}
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	double xoffset = xpos - lastX;
	double yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement((float)xoffset, (float)yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	camera.ProcessMouseScroll((float)yoffset);
}