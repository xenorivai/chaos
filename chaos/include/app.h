#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

//common
#include <iostream>
#include <vector>
#include <shader.h>
#include <camera.h>

//mafs
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct LIGHT {
	glm::vec3 pos;
	glm::vec3 intensity;

	LIGHT() {}
	LIGHT(glm::vec3 p, glm::vec3 i) : pos(p), intensity(i) {}
};

class Application {
public:
	//Light stuff
	std::vector<LIGHT> lights;
	int nLights = lights.size();

	//fps counter stuff
	float FPSdeltaTime = 0.0f;
	float FPSlastTime = 0.0f;
	float fps = 0.0f; //FPS
	float spf = 0.0f; //Seconds per frame
	unsigned int nbFrames = 0; //#Frames drawn
public:
	Application() {}
	void addLight(LIGHT l) {
		lights.push_back(l);
	}
	void addLight(glm::vec3 p, glm::vec3 i) {
		//LIGHT temp(p, i);
		lights.push_back(LIGHT(p, i));
	}
};

