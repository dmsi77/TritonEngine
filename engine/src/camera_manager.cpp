// camera_manager.cpp

#include "../../thirdparty/glm/glm/gtc/matrix_transform.hpp"
#include "../../thirdparty/glm/glm/gtc/quaternion.hpp"
#include "../../thirdparty/glm/glm/gtx/quaternion.hpp"
#include "camera_manager.hpp"
#include "physics_manager.hpp"
#include "render_manager.hpp"
#include "render_context.hpp"
#include "application.hpp"
#include "gameobject_manager.hpp"

using namespace types;

namespace realware
{
    mCamera::mCamera(cContext* context) : iObject(context)
    {
    }

    void mCamera::CreateCamera()
    {
        mGameObject* gameObjectManager = GetApplication()->GetGameObjectManager();
        _cameraGameObject = gameObjectManager->CreateGameObject(K_CAMERA_ID);
        _cameraGameObject->SetVisible(K_FALSE);
        _cameraGameObject->SetOpaque(K_FALSE);
    }

    void mCamera::DestroyCamera()
    {
        mGameObject* gameObjectManager = GetApplication()->GetGameObjectManager();
        gameObjectManager->DestroyGameObject(K_CAMERA_ID);
    }

    void mCamera::Update()
    {
        cApplication* app = GetApplication();
        const f32 deltaTime = app->GetDeltaTime();

        if (_euler.x > glm::radians(65.0f))
            _euler.x = glm::radians(65.0f);
        else if (_euler.x < glm::radians(-65.0f))
            _euler.x = glm::radians(-65.0f);

        const glm::quat quatX = glm::angleAxis(_euler.x, glm::vec3(1.0f, 0.0f, 0.0f));
        const glm::quat quatY = glm::angleAxis(_euler.y, glm::vec3(0.0f, 1.0f, 0.0f));
        const glm::quat quatZ = glm::angleAxis(_euler.z, glm::vec3(0.0f, 0.0f, 1.0f));
        _direction = quatZ * quatY * quatX * glm::vec3(0.0f, 0.0f, -1.0f);

        sTransform* transform = _cameraGameObject->GetTransform();
        _view = glm::lookAtRH(transform->_position, transform->_position + _direction, glm::vec3(0.0f, 1.0f, 0.0f));
        _projection = glm::perspective(glm::radians(_fov), app->GetWindowSize().x / app->GetWindowSize().y, _zNear, _zFar);
        _viewProjection = _projection * _view;

        double x = 0.0, y = 0.0;
        glfwGetCursorPos((GLFWwindow*)app->GetWindow(), &x, &y);

        _prevCursorPosition = _cursorPosition;
        _cursorPosition = glm::vec2(x, y);

        const glm::vec2 mouseDelta = _prevCursorPosition - _cursorPosition;
        AddEuler(eCategory::CAMERA_ANGLE_PITCH, mouseDelta.y * _mouseSensitivity * deltaTime);
        AddEuler(eCategory::CAMERA_ANGLE_YAW, mouseDelta.x * _mouseSensitivity * deltaTime);
        
        const f32 forward = app->GetKey('W') * _moveSpeed * deltaTime;
        const f32 backward = app->GetKey('S') * _moveSpeed * deltaTime;
        const f32 left = app->GetKey('A') * _moveSpeed * deltaTime;
        const f32 right = app->GetKey('D') * _moveSpeed * deltaTime;
        if (forward > 0.0f || backward > 0.0f || left > 0.0f || right > 0.0f)
        {
            Move(forward);
            Move(-backward);
            Strafe(-left);
            Strafe(right);

            _isMoving = K_TRUE;
        }
        else
        {
            _isMoving = K_FALSE;
        }
            
        _cameraGameObject->SetViewProjectionMatrix(_viewProjection);
    }

    void mCamera::AddEuler(eCategory angle, f32 value)
    {
        if (angle == eCategory::CAMERA_ANGLE_PITCH)
            _euler[0] += value;
        else if (angle == eCategory::CAMERA_ANGLE_YAW)
            _euler[1] += value;
        else if (angle == eCategory::CAMERA_ANGLE_ROLL)
            _euler[2] += value;
    }

    void mCamera::Move(f32 value)
    {
        mPhysics* physics = GetApplication()->GetPhysicsManager();
        const cPhysicsController* controller = _cameraGameObject->GetPhysicsController();
        const sTransform* transform = _cameraGameObject->GetTransform();
        const glm::vec3 position = transform->_position;
        const glm::vec3 newPosition = transform->_position + _direction * value;
        
        physics->MoveController(
            controller,
            newPosition - position
        );
        const glm::vec3 cameraPosition = physics->GetControllerPosition(controller);

        _cameraGameObject->GetTransform()->_position = cameraPosition;
    }

    void mCamera::Strafe(f32 value)
    {
        mPhysics* physics = GetApplication()->GetPhysicsManager();
        const cPhysicsController* controller = _cameraGameObject->GetPhysicsController();
        const sTransform* transform = _cameraGameObject->GetTransform();
        const glm::vec3 right = glm::cross(_direction, glm::vec3(0.0f, 1.0f, 0.0f));
        const glm::vec3 position = transform->_position;
        const glm::vec3 newPosition = transform->_position + right * value;

        physics->MoveController(
            controller,
            newPosition - position
        );
        const glm::vec3 cameraPosition = physics->GetControllerPosition(controller);

        _cameraGameObject->GetTransform()->_position = cameraPosition;
    }

    void mCamera::Lift(f32 value)
    {
        mPhysics* physics = GetApplication()->GetPhysicsManager();
        const cPhysicsController* controller = _cameraGameObject->GetPhysicsController();
        const sTransform* transform = _cameraGameObject->GetTransform();
        const glm::vec3 position = transform->_position;
        const glm::vec3 newPosition = transform->_position + glm::vec3(0.0f, 1.0f, 0.0f) * value;

        physics->MoveController(
            controller,
            newPosition - position
        );
        const glm::vec3 cameraPosition = physics->GetControllerPosition(controller);

        _cameraGameObject->GetTransform()->_position = cameraPosition;
    }
}