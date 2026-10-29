#include "ray_tracer_app.h"
#include <iostream>

static RayTracerApp::Config parseConfig(int argc, char* argv[]) {
    RayTracerApp::Config config{
        .windowConfig{
            .title = "Ray Tracer",
            .width = 640,
            .height = 360,
            .vsync = false,

            .resizable = false,
            .fullscreen = false,
            .maximize = false,
            .msaa = false,

            .driverVersion = {4, 1}
        },
        .assetRootDir{std::filesystem::absolute(argv[0]).parent_path() / "assets"}
    };

    return config;
}

int main(int argc, char* argv[]) {
    try {
        RayTracerApp app{parseConfig(argc, argv)};
        app.run();
    }
    catch (std::exception const& error) {
        std::cerr << error.what() << '\n';
        return EXIT_FAILURE;
    }
    catch (...) {
        std::cerr << "Unknown exception.\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
