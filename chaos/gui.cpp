#include <gui.h>

void GUI::init(GLFWwindow* window) {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 430");
}

void GUI::render(Camera& camera, Application& app) {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	/*Stats*/
	ImGui::Begin("Stats");
	/*ImGui::SetWindowPos(ImVec2(2, 3));
	ImGui::SetWindowSize(ImVec2(353, 62));*/
	ImGui::Text("%f FPS\n%f   ms/frame", app.fps, app.spf);
	ImGui::End();

	/*Camera*/
	ImGui::Begin("Camera Settings");
	/*ImGui::SetWindowPos(ImVec2(2, 66));
	ImGui::SetWindowSize(ImVec2(353, 172));*/
	ImGui::Text("Controls :\nWASD : Move arouund\nRMB + Drag : Look around\nR : Reset camera position\nQ : Quit\nF : Fullscreen");
	ImGui::Separator();
	ImGui::Text("Position : [%.2f, %.2f, %.2f] ", camera.Position.x, camera.Position.y, camera.Position.z);
	ImGui::Text("Front    : [%.2f, %.2f, %.2f] ", camera.Front.x, camera.Front.y, camera.Front.z);
	ImGui::SliderFloat("Zoom(FOV)", &camera.Zoom, 1.0f, 89.0f, "%.2f");
	//ImGui::SliderFloat("Speed", &camera.MovementSpeed, 2, 10);
	if (ImGui::Button("Reset Camera")) {
		camera.Reset();
	}
	ImGui::End();

	/*TO-FIX!! Add slider for controlling light pos and intensity*/
	/*Lights*/
	/*
	ImGui::Begin("Lights");
	float temp[3];
	ImGui::SliderFloat3("Light 1", temp, 0, 5, "%.2f");
	app.lights[1].pos = glm::vec3(temp[0], temp[1], temp[2]);
	ImGui::End();
	*/

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

GUI::~GUI() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}
