#pragma once

#define MOX_ENABLE_PIPELINE_STATS

#include "ShaderManager.h"
#include "Model.h"
#include "Camera.h"
#include "Scene.h"
#include "MOX_API.h"

struct GLFWwindow;

// TODO: THIS CLASS IS A BIG MESS, IT NEEDS TO BE REWORKED, SPLIT INTO SEVERAL CLASSES, AND CAMERA LOGIC MUST BE REMOVED FROM HERE
// it is must be not MOX_API
class MOX_API Renderer
{
private:
	GLFWwindow* m_window = nullptr;
	bool m_vsyncEnabled = false;
	bool m_depthTestEnabled = true;
	bool m_cullingEnabled = true;
	bool m_wireframeEnabled = false;
	bool m_isLightEnabled = true;

#ifdef MOX_ENABLE_PIPELINE_STATS
    struct PipelineStats
    {
        bool supported = false;

        static constexpr int kBuffered = 3;
        int frameIndex = 0;

        GLuint qPrimitivesGenerated[kBuffered]{};
        GLuint qFragInvocations[kBuffered]{};

        // last resolved values
        uint64_t primitivesGenerated = 0;
        uint64_t fragmentInvocations = 0;

        void Init()
        {
            supported = (GLAD_GL_ARB_pipeline_statistics_query != 0);
            if (!supported) return;

            glGenQueries(kBuffered, qPrimitivesGenerated);
            glGenQueries(kBuffered, qFragInvocations);
        }

        void Shutdown()
        {
            if (!supported) return;
            glDeleteQueries(kBuffered, qPrimitivesGenerated);
            glDeleteQueries(kBuffered, qFragInvocations);
            supported = false;
        }

        void BeginFrame()
        {
            if (!supported) return;

            const int i = frameIndex;

            glBeginQuery(GL_PRIMITIVES_GENERATED, qPrimitivesGenerated[i]);
            glBeginQuery(GL_FRAGMENT_SHADER_INVOCATIONS_ARB, qFragInvocations[i]);
        }

        void EndFrameAndResolvePrevious()
        {
            if (!supported) return;

            glEndQuery(GL_FRAGMENT_SHADER_INVOCATIONS_ARB);
            glEndQuery(GL_PRIMITIVES_GENERATED);

            const int prev = (frameIndex + kBuffered - 1) % kBuffered;

            GLuint64 v = 0;
            glGetQueryObjectui64v(qPrimitivesGenerated[prev], GL_QUERY_RESULT, &v);
            primitivesGenerated = static_cast<uint64_t>(v);

            glGetQueryObjectui64v(qFragInvocations[prev], GL_QUERY_RESULT, &v);
            fragmentInvocations = static_cast<uint64_t>(v);

            frameIndex = (frameIndex + 1) % kBuffered;
        }
    };

    PipelineStats m_stats;
#endif

public:

    // Camera
    Camera camera = {};

	int m_width = 800;
	int m_height = 600;

	Renderer() = default;
	~Renderer() = default;

	Renderer(const Renderer&) = delete;
	Renderer& operator=(const Renderer&) = delete;
	Renderer(Renderer&&) = delete;
	Renderer& operator=(Renderer&&) = delete;

	[[nodiscard]] bool ShouldClose() const;
	double GetTime() const;

    using MouseButtonCallbackFn =
        std::function<void(int button, int action, int mods)>;

    // When camera logic will be removed from cpp, we need to move this into ptivate section
    MouseButtonCallbackFn m_mouseButtonCallback;
    void SetMouseButtonCallback(MouseButtonCallbackFn callback);

	int Init();

	void BeginFrame(double deltaTime);
	void RenderModels(const Scene& scene, double totalTime);
	void Render(
        const Scene& scene,
        double fps,
        double frameTimeMs,
        double deltaTime,
		double totalTime);
	void EndFrame();

    Camera& GetCamera() {
		return camera;
    }
    
	void Shutdown();
};