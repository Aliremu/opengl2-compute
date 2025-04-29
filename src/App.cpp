#include "App.h"

#include <chrono>
#include <cmath>
#include <memory>

#include <SDL3/SDL_init.h>
#include <SDL3/SDL_video.h>
#include <spdlog/spdlog.h>

#include "Shader.h"

App::~App() { spdlog::info("Shutting down!"); }

bool App::init() {
    spdlog::info("Initializing opengl2-compute!");

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        spdlog::error("Couldn't initialize SDL: {}", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    return SDL_APP_SUCCESS;
}

bool App::createWindow() {
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

#if defined(_WIN32)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#elif defined(__APPLE__)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
#endif

    auto window = SDL_CreateWindow("opengl2-compute example", 640, 480, SDL_WINDOW_OPENGL);
    if (!window) {
        spdlog::error("Couldn't create SDL window: {}", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    auto cx = SDL_GL_CreateContext(window);

    m_window = std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)>(std::move(window), &SDL_DestroyWindow);
    m_glContext = std::move(cx);
    glewInit();

    auto version = glGetString(GL_VERSION);
    spdlog::info("OpenGL version: {}", reinterpret_cast<const char*>(version));

    int major = 0, minor = 0;
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major);
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor);
    spdlog::info("Got OpenGL version: {}.{}", major, minor);

    return SDL_APP_SUCCESS;
}

void App::createShaders() {
    std::string_view vertSrc = R"(
    #version 120

    attribute vec2 a_Position;

    varying vec2 v_TexCoord;
        
    void main() {
        v_TexCoord = a_Position * 0.5 + 0.5;
        gl_Position = vec4(a_Position, 0.0, 1.0);
    }
)";
    std::string_view fragSrc = R"(
    #version 120
    
    varying vec2 v_TexCoord;

    uniform int u_Length;

    float function(float i) {
        float o = 0;
        for (int j = 0; j < i; j++) {
            o += pow(j / 10.0, 2);
        }

        return o;
    }

    void main() {
        vec2 pixel = v_TexCoord * float(u_Length);
        float index = 4.0 * (floor(pixel.y) * float(u_Length) + floor(pixel.x));
        float r = function(index);    
        float g = function(index + 1);    
        float b = function(index + 2);    
        float a = function(index + 3);    
        
        gl_FragColor = vec4(r, g, b, a);
    }
)";
    if (auto shader = Shader::createShader(vertSrc, fragSrc)) {
        m_shader = shader;
    } else {
        spdlog::error("Failed to create shader!");
    }
}

void App::runCompute() {
    assert(m_window);
    assert(m_shader);

    float quadVerts[] = {
        -1.0f,
        -1.0f,
        1.0f,
        -1.0f,
        -1.0f,
        1.0f,
        1.0f,
        1.0f,
    };

    int length = 150; // length is squared * 4 so length 150 = 90,000 to calculate
    int query = 3273;
    float expected = 116820096;
    {
        std::vector<float> data(length * length * 4);

        auto t1 = std::chrono::high_resolution_clock::now();

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, quadVerts);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        GLuint tex;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F_ARB, length, length, 0, GL_RGBA, GL_FLOAT, NULL);

        GLuint fbo;
        glGenFramebuffersEXT(1, &fbo);
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, tex, 0);

        m_shader->use();
        m_shader->setInt("u_Length", length);
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);

        glViewport(0, 0, length, length);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glReadPixels(0, 0, length, length, GL_RGBA, GL_FLOAT, data.data());

        auto t2 = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> ms_double = t2 - t1;

        spdlog::info("GPU: Calculated {}th element is: {:.2f}", query, data[query]);
        spdlog::info("GPU: took {}ms", ms_double.count()); // not accurate way to benchmark gpu performance
    }

    {
        std::vector<float> data(length * length * 4);

        auto t1 = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < length * length * 4; i++) {
            float sum = 0;
            for (int j = 0; j < i; j++) {
                sum += std::pow(j / 10.0, 2);
            }
            data[i] = sum;
        }

        auto t2 = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> ms_double = t2 - t1;

        spdlog::info("CPU: Calculated {}th element is: {:.2f}", query, data[query]);
        spdlog::info("CPU: took {}ms", ms_double.count());
    }
}

