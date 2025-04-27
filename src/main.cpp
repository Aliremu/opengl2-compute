#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "App.h"

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
    auto app = new App;
    app->init();
    app->createWindow();
    app->createShaders();
    app->runCompute();

    *appstate = std::move(app);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate) { return SDL_APP_CONTINUE; }

void SDL_AppQuit(void* appstate, SDL_AppResult result) {
    auto app = static_cast<App*>(appstate);
    delete app;
}

