#pragma once

#include <memory>

#include <SDL3/SDL_video.h>

class Shader;

class App {
public:
    App() : m_window{nullptr, &SDL_DestroyWindow}, m_glContext{}, m_shader{nullptr} {}
    ~App();

    bool init();
    bool createWindow();
    void createShaders();
    void runCompute();

private:
    std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> m_window;
    SDL_GLContext m_glContext;
    std::shared_ptr<Shader> m_shader;
};

