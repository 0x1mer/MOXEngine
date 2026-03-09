#include "pch.h"
#include "Renderer.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Camera.h"

#include <iostream>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

uint64_t totalTriangles = 0;
uint64_t totalDrawCalls = 0;

// Window settings
constexpr unsigned int kDefaultScreenWidth = 1920;
constexpr unsigned int kDefaultScreenHeight = 1080;

bool isMouseLocked = true;

float lastX = kDefaultScreenWidth / 2.0f;
float lastY = kDefaultScreenHeight / 2.0f;
bool firstMouse = true;

namespace {

	void MouseCallback(GLFWwindow* window, double xposIn, double yposIn)
	{
		auto* renderer = static_cast<Renderer*>(glfwGetWindowUserPointer(window));
		if (!renderer) return;

		Camera& camera = renderer->camera;

		if (!isMouseLocked) return;

		float xpos = static_cast<float>(xposIn);
		float ypos = static_cast<float>(yposIn);

		if (firstMouse)
		{
			lastX = xpos;
			lastY = ypos;
			firstMouse = false;
		}

		float xoffset = lastX - xpos;
		float yoffset = lastY - ypos;

		lastX = xpos;
		lastY = ypos;

		camera.ProcessMouseMovement(xoffset, yoffset);
	}

	void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
	{
		auto* renderer = static_cast<Renderer*>(glfwGetWindowUserPointer(window));
		if (!renderer) return;

		ImGuiIO& io = ImGui::GetIO();
		if (io.WantCaptureMouse)
			return;

		if (renderer->m_mouseButtonCallback)
			renderer->m_mouseButtonCallback(button, action, mods);
	}

	void CursorOn(GLFWwindow* window) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		isMouseLocked = false;
	}

	void CursorOff(GLFWwindow* window) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		isMouseLocked = true;
	}

	void KeyCallback(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/)
	{
		if (key == GLFW_KEY_E && action == GLFW_PRESS)
		{
			if (isMouseLocked) CursorOn(window);
			else CursorOff(window);

			firstMouse = true;
		}

		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);
	}

	void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
	{
		auto* renderer = static_cast<Renderer*>(glfwGetWindowUserPointer(window));
		if (!renderer) return;

		renderer->camera.ProcessMouseScroll(static_cast<float>(yoffset));
	}

	void ProcessInput(GLFWwindow* window, double deltaTime, Camera& camera)
	{
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			camera.ProcessKeyboard(CameraMovement::Forward, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			camera.ProcessKeyboard(CameraMovement::Backward, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			camera.ProcessKeyboard(CameraMovement::Left, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			camera.ProcessKeyboard(CameraMovement::Right, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
			camera.ProcessKeyboard(CameraMovement::Down, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
			camera.ProcessKeyboard(CameraMovement::Up, deltaTime);
	}

	void PrintGlInfo()
	{
		const GLubyte* version = glGetString(GL_VERSION);
		const GLubyte* renderer = glGetString(GL_RENDERER);
		const GLubyte* vendor = glGetString(GL_VENDOR);

		std::cout << "OpenGL:   " << (version ? reinterpret_cast<const char*>(version) : "unknown") << "\n";
		std::cout << "Renderer: " << (renderer ? reinterpret_cast<const char*>(renderer) : "unknown") << "\n";
		std::cout << "Vendor:   " << (vendor ? reinterpret_cast<const char*>(vendor) : "unknown") << "\n";
	}
}

bool Renderer::ShouldClose() const
{
	return m_window && glfwWindowShouldClose(m_window);
}

double Renderer::GetTime() const
{
	return glfwGetTime();
}

void Renderer::SetMouseButtonCallback(MouseButtonCallbackFn callback)
{
	m_mouseButtonCallback = std::move(callback);
}

void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);

	auto* renderer = static_cast<Renderer*>(glfwGetWindowUserPointer(window));
	if (renderer)
	{
		renderer->m_width = width;
		renderer->m_height = height;
	}
}

int Renderer::Init()
{
	if (!glfwInit()) {
		std::cout << "Failed to initialize GLFW\n";
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// TODO: make configurable
	m_window = glfwCreateWindow(
		static_cast<int>(kDefaultScreenWidth),
		static_cast<int>(kDefaultScreenHeight),
		"MOX",
		nullptr,
		nullptr
	);

	m_width = kDefaultScreenWidth;
	m_height = kDefaultScreenHeight;

	if (!m_window) {
		std::cout << "Failed to create GLFW window\n";
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(m_window);

	glfwSwapInterval(m_vsyncEnabled ? 1 : 0);

	glfwSetWindowUserPointer(m_window, this);
	glfwSetFramebufferSizeCallback(m_window, FramebufferSizeCallback);

	glfwSetCursorPosCallback(m_window, MouseCallback);
	glfwSetScrollCallback(m_window, ScrollCallback);
	glfwSetKeyCallback(m_window, KeyCallback);
	glfwSetMouseButtonCallback(m_window, MouseButtonCallback);

	// capture mouse
	glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// GLAD init
	if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
		std::cout << "Failed to initialize GLAD\n";
		glfwDestroyWindow(m_window);
		glfwTerminate();
		return -1;
	}

	PrintGlInfo();

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	ImGui::StyleColorsDark();

	// GLFW + OpenGL3 backends
	ImGui_ImplGlfw_InitForOpenGL(m_window, true);
	ImGui_ImplOpenGL3_Init("#version 430");

	// Global state
	// TODO: make configurable
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

#ifdef MOX_ENABLE_PIPELINE_STATS
	m_stats.Init();
#endif

	return 0;
}

void Renderer::BeginFrame(double deltaTime)
{
	ProcessInput(m_window, deltaTime, camera);

	glClearColor(130.0f / 255.0f, 175.0f / 255.0f, 255.0f / 255.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void Renderer::RenderModels(
	const Scene& scene,
	double totalTime)
{
	const float aspect = static_cast<float>(m_width) / static_cast<float>(m_height);

	// const Frustum fr = camera.GetFrustum(aspect, 0.1f, 100.0f);
	const glm::mat4 projection = camera.GetProjectionMatrix(aspect);
	const glm::mat4 view = camera.GetViewMatrix();

	Shader* currentShader = nullptr;

	for (const auto& [id, model] : scene.GetModels())
	{
		for (const auto& r : model.GetRenderables())
		{
			if (!r.mesh || !r.mesh->IsValid())
				continue;
			if (!r.shader)
				continue;

			// Switch shader only when needed
			if (currentShader != r.shader)
			{
				currentShader = r.shader;
				currentShader->use();

				currentShader->setMat4("projection", projection);
				currentShader->setMat4("view", view);

				currentShader->setVec3("lightDir", glm::normalize(glm::vec3(-0.3f, -1.0f, -0.2f)));
				currentShader->setVec3("ambient", glm::vec3(0.15f));
				currentShader->setFloat("uTime", static_cast<float>(totalTime));
				currentShader->setBool("isLightEnabled", m_isLightEnabled);

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, 1);
				currentShader->setInt("uAtlas", 0);
			}

			// Model matrix from your Transform
			const glm::mat4 modelMat = r.transform.Matrix();
			currentShader->setMat4("model", modelMat);

			if (!r.mesh->vao.BindArray())
				continue;

			if (r.mesh->IsIndexed())
			{
				glDrawElements(
					GL_TRIANGLES,
					static_cast<GLsizei>(r.mesh->indexCount),
					GL_UNSIGNED_INT,
					(void*)0
				);

				totalTriangles += r.mesh->indexCount / 3;
			}
			else
			{
				glDrawArrays(
					GL_TRIANGLES,
					0,
					static_cast<GLsizei>(r.mesh->vertexCount)
				);

				totalTriangles += r.mesh->vertexCount / 3;
			}

			totalDrawCalls++;
		}
	}

	VAO::UnbindArray();
}

void Renderer::Render(
	const Scene& scene,
	double fps,
	double frameTimeMs,
	double deltaTime,
	double totalTime)
{
	totalDrawCalls = 0;
	totalTriangles = 0;

	// --- GL states ---
	(m_depthTestEnabled ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST));
	(m_cullingEnabled ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE));
	glPolygonMode(GL_FRONT_AND_BACK, m_wireframeEnabled ? GL_LINE : GL_FILL);

	// --- Draw models ---
#ifdef MOX_ENABLE_PIPELINE_STATS
	m_stats.BeginFrame();
#endif
	RenderModels(scene, totalTime);
#ifdef MOX_ENABLE_PIPELINE_STATS
	m_stats.EndFrameAndResolvePrevious();
#endif

	// --- UI ---
	ImGui::Begin("Stats");
	ImGui::Text("FPS: %.1f", fps);
	ImGui::Text("Frame time: %.2f ms", frameTimeMs);
	ImGui::Text("Delta time: %.4f", deltaTime);

	const glm::vec3& p = camera.Position();
	ImGui::Text("Position: (%.2f, %.2f, %.2f)", p.x, p.y, p.z);

	if (ImGui::Checkbox("VSync", &m_vsyncEnabled))
		glfwSwapInterval(m_vsyncEnabled ? 1 : 0);

	if (ImGui::Checkbox("Depth Test", &m_depthTestEnabled))
		(m_depthTestEnabled ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST));

	if (ImGui::Checkbox("Face Culling", &m_cullingEnabled))
		(m_cullingEnabled ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE));

	if (ImGui::Checkbox("Wireframe", &m_wireframeEnabled))
		glPolygonMode(GL_FRONT_AND_BACK, m_wireframeEnabled ? GL_LINE : GL_FILL);

	ImGui::Separator();

#ifdef MOX_ENABLE_PIPELINE_STATS
	ImGui::Text("Submitted triangles: %llu", (unsigned long long)totalTriangles);

	if (m_stats.supported) {
		ImGui::Text("GPU primitives generated: %llu", (unsigned long long)m_stats.primitivesGenerated);
		ImGui::Text("Fragment shader invocations: %llu", (unsigned long long)m_stats.fragmentInvocations);
	}
	else {
		ImGui::Text("Pipeline statistics query: NOT supported");
	}
#else
	ImGui::Text("Pipeline statistics query: NOT enabled. \nuse #define MOX_ENABLE_PIPELINE_STATS");
#endif

	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		GLFWwindow* backupCurrentContext = glfwGetCurrentContext();
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		glfwMakeContextCurrent(backupCurrentContext);
	}
}

void Renderer::EndFrame()
{
	glfwSwapBuffers(m_window);
	glfwPollEvents();
}

void Renderer::Shutdown()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(m_window);
	m_window = nullptr;
#ifdef MOX_ENABLE_PIPELINE_STATS
	m_stats.Shutdown();
#endif
	glfwTerminate();
}
