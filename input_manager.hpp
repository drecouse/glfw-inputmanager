#ifndef INPUT_MANAGER_HPP
#define INPUT_MANAGER_HPP

#include <functional>
#include <vector>
#include <cstring>

struct GLFWwindow;
namespace sgui {

    enum class Modifier {
        None = 0, Shift = 1, Control = 2, Alt = 4, Super = 8
    };

    enum class Action {
        Release = 0, Press = 1, Repeat = 2
    };

    enum class MouseButton {
        Left = 0, Right = 1, Middle = 2
    };

    enum class CursorMovement {
        Leave = 0, Enter = 1
    };

    enum class MouseMode {
        Disabled = 0, Enabled = 1
    };

    class InputManager {
    private:
        struct CursorHoldData {
            std::function<void(double, double)> handler;
            double x, y;
            double threshold2, timeToTrigger, startTime;
        };

        InputManager() = default;

    public:
        static InputManager& Instance() {
            static InputManager instance;
            return instance;
        }

        void initialize(GLFWwindow* window);
        void elapsedTime(double dt);

    public:
        void registerKeyHandler(std::function<void(int, Modifier, Action)> handler);

        template <typename H>
        void registerKeyHandler(int scancode, H handler) {
            registerKeyHandler([h = std::move(handler), s = scancode](int scancode, Modifier modifier, Action action){
                if (scancode == s) h(modifier, action);
            });
        }

        template <typename H>
        void registerKeyHandler(int scancode, Modifier modifier, H handler) {
            registerKeyHandler([h = std::move(handler), s = scancode, m = modifier](int scancode, Modifier modifier, Action action){
                if (scancode == s && modifier == m) h(action);
            });
        }

        template <typename H>
        void registerKeyHandler(int scancode, Modifier modifier, Action action, H handler) {
            registerKeyHandler([h = std::move(handler), s = scancode, m = modifier, a = action](int scancode, Modifier modifier, Action action){
                if (scancode == s && modifier == m && action == a) h();
            });
        }

        void registerUtf8KeyHandler(std::function<void(const char*, Modifier, Action)> handler);

        template <typename H>
        void registerUtf8KeyHandler(const char* utf8code, H handler) {
            if (!strcmp(utf8code, " ")) {
                registerKeyHandler(getSpaceScanCode(), handler);
            }
            else if (!strcmp(utf8code, "\n")) {
                registerKeyHandler(getEnterScanCode(), handler);
            }
            else {
                registerUtf8KeyHandler([h = std::move(handler), u = utf8code](const char* utf8code, Modifier modifier, Action action){
                    if (!strcmp(u, utf8code)) h(modifier, action);
                });
            }
        }

        template <typename H>
        void registerUtf8KeyHandler(const char* utf8code, Modifier modifier, H handler) {
            if (!strcmp(utf8code, " ")) {
                registerKeyHandler(getSpaceScanCode(), modifier, handler);
            }
            else if (!strcmp(utf8code, "\n")) {
                registerKeyHandler(getEnterScanCode(), modifier, handler);
            }
            else {
                registerUtf8KeyHandler([h = std::move(handler), u = utf8code, m = modifier](const char* utf8code, Modifier modifier, Action action){
                    if (!strcmp(u, utf8code) && m == modifier) h(action);
                });
            }
        }

        template <typename H>
        void registerUtf8KeyHandler(const char* utf8code, Modifier modifier, Action action, H handler) {
            if (!strcmp(utf8code, " ")) {
                registerKeyHandler(getSpaceScanCode(), modifier, action, handler);
            }
            else if (!strcmp(utf8code, "\n")) {
                registerKeyHandler(getEnterScanCode(), modifier, action, handler);
            }
            else {
                registerUtf8KeyHandler([h = std::move(handler), u = utf8code, m = modifier, a = action](const char* utf8code, Modifier modifier, Action action){
                    if (!strcmp(u, utf8code) && m == modifier && a == action) h();
                });
            }
        }

        void registerMouseButtonHandler(std::function<void(MouseButton, Modifier, Action)> handler);

        template <typename H>
        void registerMouseButtonHandler(MouseButton mouseButton, H handler) {
            registerMouseButtonHandler([h = std::move(handler), mb = mouseButton](MouseButton mouseButton, Modifier modifier, Action action){
                if (mb == mouseButton) h(modifier, action);
            });
        }

        template <typename H>
        void registerMouseButtonHandler(MouseButton mouseButton, Modifier modifier, H handler) {
            registerMouseButtonHandler([h = std::move(handler), mb = mouseButton, m = modifier](MouseButton mouseButton, Modifier modifier, Action action){
                if (mb == mouseButton && m == modifier) h(action);
            });
        }

        template <typename H>
        void registerMouseButtonHandler(MouseButton mouseButton, Modifier modifier, Action action, H handler) {
            registerMouseButtonHandler([h = std::move(handler), mb = mouseButton, m = modifier, a = action](MouseButton mouseButton, Modifier modifier, Action action){
                if (mb == mouseButton && m == modifier && a == action) h();
            });
        }

        void registerMouseScrollHandler(std::function<void(double, double)> handler);

        void registerCursorMovementHandler(std::function<void(CursorMovement)> handler);

        template <typename H>
        void registerCursorMovementHandler(CursorMovement movement, H handler) {
            registerCursorMovementHandler([h = std::move(handler), m = movement](CursorMovement movement){
                if (m == movement) h();
            });
        }

        void registerCursorPositionHandler(std::function<void(double, double)> handler);
        
        void registerCursorHoldHandler(double triggerTimeInMs, double threshold, std::function<void(double, double)> handler);

        void registerWindowResizeHandler(std::function<void(int, int)> handler);

    public:
        void setMouseMode(MouseMode mouseMode);

    private:
        bool isKeyboardCaptured();
        bool isMouseCaptured();
        static int getSpaceScanCode();
        static int getEnterScanCode();

        template <typename Container>
        static Container backupContainer(Container& container)
        {
            auto ret = std::move(container);
            container.clear();
            return ret;
        }

        template <typename Container>
        static void restoreContainer(Container& container, Container& backup)
        {
            std::swap(container, backup);
            if (!backup.empty()) {
                container.insert(std::end(container), std::make_move_iterator(std::begin(backup)), std::make_move_iterator(std::end(backup)));
            }
        }

    private:
        GLFWwindow* window;

    private:
        std::vector<std::function<void(int, Modifier, Action)>> keyHandlers;
        std::vector<std::function<void(const char*, Modifier, Action)>> utf8KeyHandlers;
        std::vector<std::function<void(MouseButton, Modifier, Action)>> mouseButtonHandlers;
        std::vector<std::function<void(double, double)>> mouseScrollHandlers;
        std::vector<std::function<void(CursorMovement)>> cursorMovementHandlers;
        std::vector<std::function<void(double, double)>> cursorPositionHandlers;
        std::vector<std::function<void(int, int)>> windowResizeHandlers;
        std::vector<CursorHoldData> cursorHoldHandlers;
        std::vector<char> keyStates;
    };

    inline InputManager& inputManager = InputManager::Instance();
}
#endif
