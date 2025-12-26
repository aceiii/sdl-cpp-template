#include <argparse/argparse.hpp>
#include <magic_enum/magic_enum.hpp>
#include <spdlog/spdlog.h>
#include <SDL3/SDL.h>


static bool set_logging_level(const std::string &level_name) {
  auto level = magic_enum::enum_cast<spdlog::level>(level_name);
  if (level.has_value()) {
    spdlog::set_level(level.value());
    return true;
  }
  return false;
}


auto run() -> int {
  auto eventName = [](Uint32 event_type) -> std::string {
    switch (event_type) {
      case SDL_EVENT_QUIT: return "SDL_EVENT_QUIT";
      case SDL_EVENT_WINDOW_SHOWN: return "SDL_EVENT_WINDOW_SHOWN";
      case SDL_EVENT_WINDOW_HIDDEN: return "SDL_EVENT_WINDOW_HIDDEN";
      case SDL_EVENT_WINDOW_EXPOSED: return "SDL_EVENT_WINDOW_EXPOSED";
      case SDL_EVENT_WINDOW_MOVED: return "SDL_EVENT_WINDOW_MOVED";
      case SDL_EVENT_WINDOW_RESIZED: return "SDL_EVENT_WINDOW_RESIZED";
      case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED: return "SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED";
      case SDL_EVENT_WINDOW_METAL_VIEW_RESIZED: return "SDL_EVENT_WINDOW_METAL_VIEW_RESIZED";
      case SDL_EVENT_WINDOW_MINIMIZED: return "SDL_EVENT_WINDOW_MINIMIZED";
      case SDL_EVENT_WINDOW_MAXIMIZED: return "SDL_EVENT_WINDOW_MAXIMIZED";
      case SDL_EVENT_WINDOW_RESTORED: return "SDL_EVENT_WINDOW_RESTORED";
      case SDL_EVENT_WINDOW_MOUSE_ENTER: return "SDL_EVENT_WINDOW_MOUSE_ENTER";
      case SDL_EVENT_WINDOW_MOUSE_LEAVE: return "SDL_EVENT_WINDOW_MOUSE_LEAVE";
      case SDL_EVENT_WINDOW_FOCUS_GAINED: return "SDL_EVENT_WINDOW_FOCUS_GAINED";
      case SDL_EVENT_WINDOW_FOCUS_LOST: return "SDL_EVENT_WINDOW_FOCUS_LOST";
      case SDL_EVENT_WINDOW_CLOSE_REQUESTED: return "SDL_EVENT_WINDOW_CLOSE_REQUESTED";
      case SDL_EVENT_WINDOW_HIT_TEST: return "SDL_EVENT_WINDOW_HIT_TEST";
      case SDL_EVENT_WINDOW_ICCPROF_CHANGED: return "SDL_EVENT_WINDOW_ICCPROF_CHANGED";
      case SDL_EVENT_WINDOW_DISPLAY_CHANGED: return "SDL_EVENT_WINDOW_DISPLAY_CHANGED";
      case SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED: return "SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED";
      case SDL_EVENT_WINDOW_SAFE_AREA_CHANGED: return "SDL_EVENT_WINDOW_SAFE_AREA_CHANGED";
      case SDL_EVENT_WINDOW_OCCLUDED: return "SDL_EVENT_WINDOW_OCCLUDED";
      case SDL_EVENT_WINDOW_ENTER_FULLSCREEN: return "SDL_EVENT_WINDOW_ENTER_FULLSCREEN";
      case SDL_EVENT_WINDOW_LEAVE_FULLSCREEN: return "SDL_EVENT_WINDOW_LEAVE_FULLSCREEN";
      case SDL_EVENT_WINDOW_DESTROYED: return "SDL_EVENT_WINDOW_DESTROYED";
      default: return std::format("0x{:X}", event_type);
    }
  };

  auto fillWithNoise = [](SDL_Surface *surface) -> void {
    for (int y = 0; y < surface->h; y++) {
      for (int x = 0; x < surface->w; x++) {
        const auto val = SDL_rand(256);
        SDL_WriteSurfacePixel(surface, x, y, val, val, val, 255);
      }
    }
  };

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    spdlog::error("Failed to initialize SDL: {}", SDL_GetError());
    return 1;
  }

  SDL_Window *window = SDL_CreateWindow("Toy UI", 600, 400, SDL_WINDOW_RESIZABLE);
  if (!window) {
    spdlog::error("Failed to create window: {}", SDL_GetError());
    return 1;
  }

  SDL_Surface *surface = SDL_GetWindowSurface(window);
  if (!surface) {
    spdlog::error("Failed to get window surface: {}", SDL_GetError());
    return 1;
  }

  SDL_Event event;
  bool quit = false;
  bool redraw = true;

  std::vector<SDL_Window*> child_windows;

  while (!quit) {
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_EVENT_QUIT: quit = true; break;
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED: {
          if (SDL_Window *window_to_destroy = SDL_GetWindowFromID(event.window.windowID); window_to_destroy == window) {
            quit = true;
          } else {
            std::erase(child_windows, window_to_destroy);
            SDL_DestroyWindow(window_to_destroy);
          }
          break;
        }
        case SDL_EVENT_WINDOW_RESIZED:
          SDL_DestroySurface(surface);
          surface = SDL_GetWindowSurface(window);
          redraw = true;
          break;
        case SDL_EVENT_MOUSE_MOTION: redraw = true; break;
        case SDL_EVENT_KEY_UP:
          if (event.key.key == SDLK_N && event.key.mod & SDL_KMOD_GUI) {
            if (SDL_Window *new_window = SDL_CreatePopupWindow(window, 150, 150, 300, 300, 0); !new_window) {
              spdlog::error("Failed to create child window: {}", SDL_GetError());
            } else {
              child_windows.push_back(new_window);
            }
          }
          else if (event.key.key == SDLK_S && event.key.mod & SDL_KMOD_GUI) {
            SDL_ShowSaveFileDialog([] (void* userdata, const char* const *fileList, int filter) {
              auto current = fileList;
              auto file = *current;
              while (file) {
                spdlog::info("Selected files: {}", *current);
                current++;
                file = *current;
              }
            }, static_cast<void*>(window), window, nullptr, 0, nullptr);
          }
          else if (event.key.key == SDLK_O && event.key.mod & SDL_KMOD_GUI) {
            SDL_ShowOpenFileDialog([] (void* userdata, const char* const *fileList, int filter) {
              auto current = fileList;
              auto file = *current;
              while (file) {
                spdlog::info("Selected files: {}", *current);
                current++;
                file = *current;
              }
            }, static_cast<void*>(window), window, nullptr, 0, nullptr, true);
          }
          break;
        default:
          spdlog::info("Unhandled event: {}", eventName(event.type));
          break;
      }
    }

    if (redraw) {
      // update
      fillWithNoise(surface);
      SDL_UpdateWindowSurface(window);
      redraw = false;
    }
  }

  for (auto child_window: child_windows) {
    SDL_DestroyWindow(child_window);
  }

  SDL_DestroySurface(surface);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}

auto main(int argc, char *argv[]) -> int {
  spdlog::set_level(spdlog::level::info);

  argparse::ArgumentParser program("aceboy", "0.0.1");

  program.add_argument("--log-level")
      .help("Set the verbosity for logging")
      .default_value(std::string("info"))
      .nargs(1);

  try {
    program.parse_args(argc, argv);
  } catch (const std::exception &err) {
    std::cerr << err.what() << std::endl;
    std::cerr << program;
    return 1;
  }

  const std::string level = program.get("--log-level");
  if (!set_logging_level(level)) {
    std::cerr << fmt::format("Invalid argument \"{}\" - allowed options: "
                             "{{trace, debug, info, warn, err, critical, off}}",
                             level)
              << std::endl;
    std::cerr << program;
    return 1;
  }

  int result = run();

  spdlog::info("Exiting.");

  return result;
}
