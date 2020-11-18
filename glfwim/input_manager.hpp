#ifndef INPUT_MANAGER_HPP
#define INPUT_MANAGER_HPP

#include <functional>
#include <vector>
#include <cstring>
#include <string>

struct GLFWwindow;
namespace glfwim {
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

    public:
        static InputManager& instance() {
            static InputManager instance;
            return instance;
        }
		InputManager() = default;
		InputManager(const InputManager&) = delete;
		InputManager(InputManager&&) = delete;
		InputManager& operator=(const InputManager&) = delete;
		InputManager& operator=(InputManager&&) = delete;
		
    public:
        void initialize(GLFWwindow* window);
        void pollEvents();
        void setMouseMode(MouseMode mouseMode);
        void waitUntilNextEventHandling();
        void waitUntilNextEventHandling(double timeout);
        void pauseInputHandling();
        void continueInputHandling();

    public:
        enum class CallbackType { Key, Utf8Key, MouseButton, MouseScroll, CursorMovement, CursorPosition, WindowResize, CursorHold, PathDrop };

        class CallbackHandler {
        public:
            CallbackHandler(InputManager* inputManager, CallbackType type, size_t index) : pInputManager{inputManager}, type{type}, index{index} {}

            void enable() { enable_impl(true); }
            void disable() { enable_impl(false); }

        private:
            void enable_impl(bool enable);

        private:
            InputManager* pInputManager;
            CallbackType type;
            size_t index;
        };

        friend class CallbackHandler;

    public:
        CallbackHandler registerKeyHandler(std::function<void(int, Modifier, Action)> handler);

        template <typename H>
        CallbackHandler registerKeyHandler(int scancode, H handler) {
            return registerKeyHandler([h = std::move(handler), s = scancode](int scancode, Modifier modifier, Action action){
                if (scancode == s) h(modifier, action);
            });
        }

        template <typename H>
        CallbackHandler registerKeyHandler(int scancode, Modifier modifier, H handler) {
            return registerKeyHandler([h = std::move(handler), s = scancode, m = modifier](int scancode, Modifier modifier, Action action){
                if (scancode == s && modifier == m) h(action);
            });
        }

        template <typename H>
        CallbackHandler registerKeyHandler(int scancode, Modifier modifier, Action action, H handler) {
            return registerKeyHandler([h = std::move(handler), s = scancode, m = modifier, a = action](int scancode, Modifier modifier, Action action){
                if (scancode == s && modifier == m && action == a) h();
            });
        }

        CallbackHandler registerUtf8KeyHandler(std::function<void(const char*, Modifier, Action)> handler);

        template <typename H>
        CallbackHandler registerUtf8KeyHandler(const char* utf8code, H handler) {
            if (!strcmp(utf8code, " ")) {
                return registerKeyHandler(getSpaceScanCode(), handler);
            }
            else if (!strcmp(utf8code, "\n")) {
                return registerKeyHandler(getEnterScanCode(), handler);
            }
            else if (!strcmp(utf8code, "->")) {
                return registerKeyHandler(getRightArrowScanCode(), handler);
            }
            else if (!strcmp(utf8code, "<-")) {
                return registerKeyHandler(getLeftArrowScanCode(), handler);
            }
            else {
                return registerUtf8KeyHandler([h = std::move(handler), u = utf8code](const char* utf8code, Modifier modifier, Action action){
                    if (!strcmp(u, utf8code)) h(modifier, action);
                });
            }
        }

        template <typename H>
        CallbackHandler registerUtf8KeyHandler(const char* utf8code, Modifier modifier, H handler) {
            if (!strcmp(utf8code, " ")) {
                return registerKeyHandler(getSpaceScanCode(), modifier, handler);
            }
            else if (!strcmp(utf8code, "\n")) {
                return registerKeyHandler(getEnterScanCode(), modifier, handler);
            }
            else if (!strcmp(utf8code, "->")) {
                return registerKeyHandler(getRightArrowScanCode(), modifier, handler);
            }
            else if (!strcmp(utf8code, "<-")) {
                return registerKeyHandler(getLeftArrowScanCode(), modifier, handler);
            }
            else {
                return registerUtf8KeyHandler([h = std::move(handler), u = utf8code, m = modifier](const char* utf8code, Modifier modifier, Action action){
                    if (!strcmp(u, utf8code) && m == modifier) h(action);
                });
            }
        }

        template <typename H>
        CallbackHandler registerUtf8KeyHandler(const char* utf8code, Modifier modifier, Action action, H handler) {
            if (!strcmp(utf8code, " ")) {
                return registerKeyHandler(getSpaceScanCode(), modifier, action, handler);
            }
            else if (!strcmp(utf8code, "\n")) {
                return registerKeyHandler(getEnterScanCode(), modifier, action, handler);
            }
            else if (!strcmp(utf8code, "->")) {
                return registerKeyHandler(getRightArrowScanCode(), modifier, action, handler);
            }
            else if (!strcmp(utf8code, "<-")) {
                return registerKeyHandler(getLeftArrowScanCode(), modifier, action, handler);
            }
            else {
                return registerUtf8KeyHandler([h = std::move(handler), u = utf8code, m = modifier, a = action](const char* utf8code, Modifier modifier, Action action){
                    if (!strcmp(u, utf8code) && m == modifier && a == action) h();
                });
            }
        }

        CallbackHandler registerMouseButtonHandler(std::function<void(MouseButton, Modifier, Action)> handler);

        template <typename H>
        CallbackHandler registerMouseButtonHandler(MouseButton mouseButton, H handler) {
            return registerMouseButtonHandler([h = std::move(handler), mb = mouseButton](MouseButton mouseButton, Modifier modifier, Action action){
                if (mb == mouseButton) h(modifier, action);
            });
        }

        template <typename H>
        CallbackHandler registerMouseButtonHandler(MouseButton mouseButton, Modifier modifier, H handler) {
            return registerMouseButtonHandler([h = std::move(handler), mb = mouseButton, m = modifier](MouseButton mouseButton, Modifier modifier, Action action){
                if (mb == mouseButton && m == modifier) h(action);
            });
        }

        template <typename H>
        CallbackHandler registerMouseButtonHandler(MouseButton mouseButton, Modifier modifier, Action action, H handler) {
            return registerMouseButtonHandler([h = std::move(handler), mb = mouseButton, m = modifier, a = action](MouseButton mouseButton, Modifier modifier, Action action){
                if (mb == mouseButton && m == modifier && a == action) h();
            });
        }

        CallbackHandler registerMouseScrollHandler(std::function<void(double, double)> handler);

        CallbackHandler registerCursorMovementHandler(std::function<void(CursorMovement)> handler);

        template <typename H>
        CallbackHandler registerCursorMovementHandler(CursorMovement movement, H handler) {
            return registerCursorMovementHandler([h = std::move(handler), m = movement](CursorMovement movement){
                if (m == movement) h();
            });
        }

        CallbackHandler registerCursorPositionHandler(std::function<void(double, double)> handler);

        CallbackHandler registerCursorHoldHandler(double triggerTimeInMs, double threshold, std::function<void(double, double)> handler);

        CallbackHandler registerWindowResizeHandler(std::function<void(int, int)> handler);
		
		template <typename Head, typename Second, typename... Args>
        CallbackHandler registerPathDropHandler(Head&& head, Second&& second, Args&&... args) {
            std::vector<std::string> filters; filters.reserve(sizeof...(args) + 1);
            return registerPathDropHandler_impl(std::move(filters), std::forward<Head>(head), std::forward<Second>(second), std::forward<Args>(args)...);
        }

        template <typename Handler>
        CallbackHandler registerPathDropHandler(Handler&& handler) {
            return registerPathDropHandler_impl(std::vector<std::string>{}, std::forward<Handler>(handler));
        }
		
    private:
        template <typename T, typename = void> struct helper : std::false_type {};
        template <typename T> struct helper<T, std::void_t<decltype(std::declval<T>()(std::declval<std::string>()))>> : std::true_type {};

        CallbackHandler registerPathDropHandler_impl2(std::function<void(const std::vector<std::string>&)> handler);

        template <typename Handler>
        CallbackHandler registerPathDropHandler_impl(std::vector<std::string>&& filters, Handler&& handler) {
            if constexpr (helper<Handler>::value) {
                return registerPathDropHandler_impl2([h = std::move(handler), f = std::move(filters)](const std::vector<std::string>& paths) {
                    for (auto& it : paths) {
                        std::string ext;
                        auto found = it.find_last_of(".");
                        if (found != std::string::npos) {
                            ext = it.substr(found + 1);
                        }
                        if (f.empty()) h(it);
                        else for (auto& fExt : f) {
                            if (ext == fExt) {
                                h(it);
                                break;
                            }
                        }
                    }
                });
            } else { // vector<string>
                return registerPathDropHandler_impl2([h = std::move(handler), f = std::move(filters)](const std::vector<std::string>& paths) {
                    std::vector<std::string> pps;
                    for (auto& it : paths) {
                        std::string ext;
                        auto found = it.find_last_of(".");
                        if (found != std::string::npos) {
                            ext = it.substr(found + 1);
                        }
                        if (f.empty()) pps.push_back(it);
                        else for (auto& fExt : f) {
                            if (ext == fExt) {
                                pps.push_back(it);
                                break;
                            }
                        }
                    }
                    h(pps);
                });
            }
        }

        template <typename... Tail>
        CallbackHandler registerPathDropHandler_impl(std::vector<std::string>&& filters, std::string&& head, Tail&&... tail) {
            filters.push_back(std::move(head));
            return registerPathDropHandler_impl(std::move(filters), std::forward<Tail>(tail)...);
        }

    private:
        bool isKeyboardCaptured();
        bool isMouseCaptured();

    private:
        void elapsedTime();
        static int getSpaceScanCode();
        static int getEnterScanCode();
        static int getRightArrowScanCode();
        static int getLeftArrowScanCode();

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

        template <typename T>
        struct HandlerHolder {
            HandlerHolder(T handler) : handler{std::move(handler)} {}

            T handler;
            volatile bool enabled = true;
        };

    private:
        GLFWwindow* window;
        volatile bool finishedInputHandling = false;
        volatile bool paused = false;

    private:
        std::vector<HandlerHolder<std::function<void(int, Modifier, Action)>>> keyHandlers;
        std::vector<HandlerHolder<std::function<void(const char*, Modifier, Action)>>> utf8KeyHandlers;
        std::vector<HandlerHolder<std::function<void(MouseButton, Modifier, Action)>>> mouseButtonHandlers;
        std::vector<HandlerHolder<std::function<void(double, double)>>> mouseScrollHandlers;
        std::vector<HandlerHolder<std::function<void(CursorMovement)>>> cursorMovementHandlers;
        std::vector<HandlerHolder<std::function<void(double, double)>>> cursorPositionHandlers;
        std::vector<HandlerHolder<std::function<void(int, int)>>> windowResizeHandlers;
        std::vector<HandlerHolder<CursorHoldData>> cursorHoldHandlers;
		std::vector<HandlerHolder<std::function<void(const std::vector<std::string>& paths)>>> pathDropHandlers;
        std::vector<char> keyStates;
    };
}
#endif
