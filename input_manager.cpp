#include "input_manager.hpp"
#include <GLFW/glfw3.h>

namespace sgui {
    void InputManager::initialize(GLFWwindow* window) {
        keyStates.resize(GLFW_KEY_LAST, 0);
        this->window = window;

        glfwSetWindowUserPointer(window, this);

        glfwSetKeyCallback(window, [](auto window, int key, int scancode, int action, int mods) {
            auto& inputManager = *(InputManager*)glfwGetWindowUserPointer(window);

            if (inputManager.isKeyboardCaptured() && !(inputManager.keyStates[key - 1] && action == GLFW_RELEASE)) return;

            if (key >= 1) {
                if (!inputManager.keyStates[key - 1] && action == GLFW_RELEASE) return;
                if (action == GLFW_PRESS) inputManager.keyStates[key - 1] = true;
                else if (action == GLFW_RELEASE) inputManager.keyStates[key - 1] = false;
            }

            auto tempKeyHandlers = backupContainer(inputManager.keyHandlers);
            auto tempUtf8KeyHandlers = backupContainer(inputManager.utf8KeyHandlers);

            for (auto& handler : tempKeyHandlers) {
                handler(scancode, Modifier{ mods }, Action{ action });
            }

            if (!tempUtf8KeyHandlers.empty()) {
                const char* utf8key = glfwGetKeyName(key, scancode);
                if (utf8key) {
                    for (auto& handler : tempUtf8KeyHandlers) {
                        handler(utf8key, Modifier{ mods }, Action{ action });
                    }
                }
            }

            restoreContainer(tempKeyHandlers, inputManager.keyHandlers);
            restoreContainer(tempUtf8KeyHandlers, inputManager.utf8KeyHandlers);
        });

        glfwSetMouseButtonCallback(window, [](auto window, int button, int action, int mods) {
            auto& inputManager = *(InputManager*)glfwGetWindowUserPointer(window);

            if (inputManager.isMouseCaptured()) return;

            auto tempHandlers = backupContainer(inputManager.mouseButtonHandlers);
            
            for (auto& handler : tempHandlers) {
                handler(MouseButton{ button }, Modifier{ mods }, Action{ action });
            }

            restoreContainer(tempHandlers, inputManager.mouseButtonHandlers);
        });

        glfwSetScrollCallback(window, [](auto window, double x, double y) {
            auto& inputManager = *(InputManager*)glfwGetWindowUserPointer(window);

            if (inputManager.isMouseCaptured()) return;

            auto tempHandlers = backupContainer(inputManager.mouseScrollHandlers);

            for (auto& handler : tempHandlers) {
                handler(x, y);
            }

            restoreContainer(tempHandlers, inputManager.mouseScrollHandlers);
        });

        glfwSetCursorEnterCallback(window, [](auto window, int entered) {
            auto& inputManager = *(InputManager*)glfwGetWindowUserPointer(window);
            auto movement = CursorMovement{ !!entered };

            if (inputManager.isMouseCaptured()) return;

            auto tempHandlers = backupContainer(inputManager.cursorMovementHandlers);

            for (auto& handler : tempHandlers) {
                handler(movement);
            }

            restoreContainer(tempHandlers, inputManager.cursorMovementHandlers);
        });

        glfwSetCursorPosCallback(window, [](auto window, double x, double y) {
            auto& inputManager = *(InputManager*)glfwGetWindowUserPointer(window);

            if (inputManager.isMouseCaptured()) return;

            auto tempHandlers = backupContainer(inputManager.cursorPositionHandlers);

            for (auto& handler : tempHandlers) {
                handler(x, y);
            }

            restoreContainer(tempHandlers, inputManager.cursorPositionHandlers);
        });

        glfwSetFramebufferSizeCallback(window, [](auto window, int x, int y) {
            auto& inputManager = *(InputManager*)glfwGetWindowUserPointer(window);

            auto tempHandlers = backupContainer(inputManager.windowResizeHandlers);

            for (auto& handler : tempHandlers) {
                handler(x, y);
            }

            restoreContainer(tempHandlers, inputManager.windowResizeHandlers);
        });
    }

    void InputManager::elapsedTime(double dt) {
        if (isMouseCaptured()) return;

        auto tempHandlers = backupContainer(cursorHoldHandlers);

        double x, y;
        glfwGetCursorPos(window, &x, &y);
        for (auto& it : tempHandlers) {
            if (it.startTime < 0) {
                it.startTime = glfwGetTime() * 1000.0;
                it.x = x;
                it.y = y;
                continue;
            }

            double dx = x - it.x, dy = y - it.y;
            double dv2 = dx * dx + dy * dy;
            if (dv2 <= it.threshold2) {
                double elapsedTime = glfwGetTime() * 1000.0 - it.startTime;
                if (elapsedTime >= it.timeToTrigger) it.handler(it.x, it.y);
            } else {
                it.startTime = glfwGetTime() * 1000.0;
                it.x = x;
                it.y = y;
            }
        }

        restoreContainer(tempHandlers, cursorHoldHandlers);
    }

    void InputManager::registerKeyHandler(std::function<void(int, Modifier, Action)> handler) {
        keyHandlers.push_back(std::move(handler));
    }

    void InputManager::registerUtf8KeyHandler(std::function<void(const char*, Modifier, Action)> handler) {
        utf8KeyHandlers.push_back(std::move(handler));
    }

    void InputManager::registerMouseButtonHandler(std::function<void(MouseButton, Modifier, Action)> handler) {
        mouseButtonHandlers.push_back(std::move(handler));
    }

    void InputManager::registerMouseScrollHandler(std::function<void(double, double)> handler) {
        mouseScrollHandlers.push_back(std::move(handler));
    }

    void InputManager::registerCursorMovementHandler(std::function<void(CursorMovement)> handler) {
        cursorMovementHandlers.push_back(std::move(handler));
    }

    void InputManager::registerCursorPositionHandler(std::function<void(double, double)> handler) {
        cursorPositionHandlers.push_back(std::move(handler));
    }

    void InputManager::registerCursorHoldHandler(double triggerTimeInMs, double threshold, std::function<void(double, double)> handler) {
        CursorHoldData data;
        data.handler = std::move(handler);
        data.threshold2 = threshold * threshold;
        data.timeToTrigger = triggerTimeInMs;
        data.x = data.y = 0;
        data.startTime = -1;
        cursorHoldHandlers.push_back(std::move(data));
    }
    
    void InputManager::registerWindowResizeHandler(std::function<void(int, int)> handler) {
        windowResizeHandlers.push_back(std::move(handler));
    }

    void InputManager::setMouseMode(MouseMode mouseMode) {
        int mod;
        switch (mouseMode) {
        case MouseMode::Disabled: mod = GLFW_CURSOR_DISABLED; break;
        case MouseMode::Enabled: mod = GLFW_CURSOR_NORMAL; break;
        }
        glfwSetInputMode(window, GLFW_CURSOR, mod);
    }

    bool InputManager::isKeyboardCaptured()
    {
        return theGUIManager.isKeyboardCaptured();
    }
    
    bool InputManager::isMouseCaptured() 
    {
        return theGUIManager.isMouseCaptured();
    }

    int InputManager::getSpaceScanCode()
    {
        return glfwGetKeyScancode(GLFW_KEY_SPACE);
    }

    int InputManager::getEnterScanCode()
    {
        return glfwGetKeyScancode(GLFW_KEY_ENTER);
    }
}
