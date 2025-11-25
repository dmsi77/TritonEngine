// application.cpp

#include <iostream>
#include <GLFW/glfw3.h>
#include "application.hpp"
#include "camera_manager.hpp"
#include "render_manager.hpp"
#include "render_context.hpp"
#include "sound_context.hpp"
#include "font_manager.hpp"
#include "sound_manager.hpp"
#include "filesystem_manager.hpp"
#include "physics_manager.hpp"
#include "gameobject_manager.hpp"
#include "texture_manager.hpp"
#include "memory_pool.hpp"
#include "event_manager.hpp"
#include "thread_manager.hpp"
#include "log.hpp"

using namespace types;

namespace realware
{
    void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        cApplication* app = (cApplication*)glfwGetWindowUserPointer(window);
        key &= app->K_KEY_BUFFER_MASK;

        if (action == GLFW_PRESS)
            app->SetKey(key, K_TRUE);
        else if (action == GLFW_RELEASE)
            app->SetKey(key, K_FALSE);
    }

    void WindowFocusCallback(GLFWwindow* window, int focused)
    {
        cApplication* app = (cApplication*)glfwGetWindowUserPointer(window);

        if (focused)
        {
            if (app->GetWindowFocus() == K_FALSE)
                app->SetWindowFocus(K_TRUE);
        }
        else
        {
            app->SetWindowFocus(K_FALSE);
        }
    }

    void WindowSizeCallback(GLFWwindow* window, int width, int height)
    {
        cApplication* app = (cApplication*)glfwGetWindowUserPointer(window);

        app->_desc._windowDesc._width = width;
        app->_desc._windowDesc._height = height;
            
        app->_render->ResizeWindow(glm::vec2(width, height));
    }

    void CursorCallback(GLFWwindow* window, double xpos, double ypos)
    {
        cApplication* app = (cApplication*)glfwGetWindowUserPointer(window);

        app->SetCursorPosition(glm::vec2(xpos, ypos));
    }

    void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
    {
        cApplication* app = (cApplication*)glfwGetWindowUserPointer(window);

        if (action == GLFW_RELEASE)
            app->SetMouseKey(button, 0);
        else if (action == GLFW_PRESS)
            app->SetMouseKey(button, 1);
    }

    cApplication::cApplication(const sApplicationDescriptor* desc) : _desc(*desc)
    {
        CreateMemoryPool();
        CreateAppWindow();
        CreateContexts();
        CreateAppManagers();
    }

    cApplication::~cApplication()
    {
        DestroyAppManagers();
        DestroyContexts();
        DestroyAppWindow();
        DestroyMemoryPool();
    }

    void cApplication::Run()
    {
        Start();

        _timepointLast = std::chrono::high_resolution_clock::now();

        while (GetRunState() == K_FALSE)
        {
            const auto currentTime = std::chrono::high_resolution_clock::now();
            std::chrono::duration<f32> elapsed = currentTime - _timepointLast;
            _deltaTime = elapsed.count();
            _timepointLast = currentTime;

            _physics->Simulate();
            _camera->Update();
            Update();
            _render->CompositeFinal();

            glfwSwapBuffers((GLFWwindow*)_window);
            glfwPollEvents();
        }

        Finish();
    }

    void cApplication::CreateAppWindow()
    {
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        if (_desc._windowDesc._isFullscreen == K_FALSE)
        {
            _window = (void*)glfwCreateWindow(_desc._windowDesc._width, _desc._windowDesc._height, _desc._windowDesc._title.c_str(), nullptr, nullptr);
        }
        else
        {
            glfwWindowHint(GLFW_DECORATED, 0);

            const glm::vec2 monitorSize = GetMonitorSize();
            _desc._windowDesc._width = monitorSize.x;
            _desc._windowDesc._height = monitorSize.y;
            _window = glfwCreateWindow(_desc._windowDesc._width, _desc._windowDesc._height, _desc._windowDesc._title.c_str(), glfwGetPrimaryMonitor(), nullptr);
        }

        glfwSetWindowUserPointer((GLFWwindow*)_window, this);

        if (!_window)
        {
            Print("Error: incompatible GL version!");
            return;
        }

        glfwMakeContextCurrent((GLFWwindow*)_window);

        glfwSwapInterval(1);

        glfwSetKeyCallback((GLFWwindow*)_window, &KeyCallback);
        glfwSetWindowFocusCallback((GLFWwindow*)_window, &WindowFocusCallback);
        glfwSetWindowSizeCallback((GLFWwindow*)_window, &WindowSizeCallback);
        glfwSetCursorPosCallback((GLFWwindow*)_window, &CursorCallback);
        glfwSetMouseButtonCallback((GLFWwindow*)_window, &MouseButtonCallback);
    }

    void cApplication::CreateMemoryPool()
    {
        _memoryPool = new cMemoryPool(this, _desc._memoryPoolByteSize, _desc._memoryPoolReservedAllocations, _desc._memoryPoolAlignment);
    }

    void cApplication::CreateContexts()
    {
        cOpenGLRenderContext* pOpenGLRenderContext = (cOpenGLRenderContext*)_memoryPool->Allocate(sizeof(cOpenGLRenderContext));
        cOpenALSoundContext* pOpenALSoundContext = (cOpenALSoundContext*)_memoryPool->Allocate(sizeof(cOpenALSoundContext));
            
        _renderContext = new (pOpenGLRenderContext) cOpenGLRenderContext(this);
        _soundContext = new (pOpenALSoundContext) cOpenALSoundContext(this);
    }

    void cApplication::CreateAppManagers()
    {
        mCamera* pCamera = (mCamera*)_memoryPool->Allocate(sizeof(mCamera));
        mTexture* pTexture = (mTexture*)_memoryPool->Allocate(sizeof(mTexture));
        mFileSystem* pFileSystem = (mFileSystem*)_memoryPool->Allocate(sizeof(mFileSystem));
        mRender* pRender = (mRender*)_memoryPool->Allocate(sizeof(mRender));
        mFont* pFont = (mFont*)_memoryPool->Allocate(sizeof(mFont));
        mSound* pSound = (mSound*)_memoryPool->Allocate(sizeof(mSound));
        mPhysics* pPhysics = (mPhysics*)_memoryPool->Allocate(sizeof(mPhysics));
        mGameObject* pGameObject = (mGameObject*)_memoryPool->Allocate(sizeof(mGameObject));
        mEvent* pEvent = (mEvent*)_memoryPool->Allocate(sizeof(mEvent));
        mThread* pThread = (mThread*)_memoryPool->Allocate(sizeof(mThread));

        _camera = new (pCamera) mCamera(this);
        _texture = new (pTexture) mTexture(this, _renderContext);
        _fileSystem = new (pFileSystem) mFileSystem(this);
        _render = new (pRender) mRender(this, _renderContext);
        _font = new (pFont) mFont(this, _renderContext);
        _sound = new (pSound) mSound(this, _soundContext);
        _physics = new (pPhysics) mPhysics(this);
        _gameObject = new (pGameObject) mGameObject(this);
        _event = new (pEvent) mEvent(this);
        _thread = new (pThread) mThread(this);
    }

    void cApplication::DestroyMemoryPool()
    {
        delete _memoryPool;
    }

    void cApplication::DestroyAppWindow()
    {
        glfwDestroyWindow((GLFWwindow*)_window);
    }

    void cApplication::DestroyContexts()
    {
        _soundContext->~iSoundContext();
        _renderContext->~iRenderContext();
        _memoryPool->Free(_soundContext);
        _memoryPool->Free(_renderContext);
    }

    void cApplication::DestroyAppManagers()
    {
        _gameObject->~mGameObject();
        _physics->~mPhysics();
        _sound->~mSound();
        _font->~mFont();
        _render->~mRender();
        _fileSystem->~mFileSystem();
        _texture->~mTexture();
        _camera->~mCamera();
        _memoryPool->Free(_gameObject);
        _memoryPool->Free(_physics);
        _memoryPool->Free(_sound);
        _memoryPool->Free(_font);
        _memoryPool->Free(_render);
        _memoryPool->Free(_fileSystem);
        _memoryPool->Free(_texture);
        _memoryPool->Free(_camera);
    }
}