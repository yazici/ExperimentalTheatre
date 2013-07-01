#include "CameraManBird.h"

#include "Camera.h"


namespace media
{
    CameraManBird::CameraManBird() :
        _camera(nullptr)
    {
    }

    CameraManBird::CameraManBird(Camera& camera) :
        _camera(nullptr)
    {
        setCamera( camera );
    }

    void CameraManBird::setCamera(Camera& camera)
    {
        _camera = & camera;
        setupCamera();
    }

    void CameraManBird::moveTo(const cellar::Vec3f& destination)
    {
        if(_camera == nullptr)
            return;

        cellar::Vec3f displacement = destination - _camera->tripod().from();
        _camera->setTripod(destination,
                           _camera->tripod().to() + displacement,
                           _camera->tripod().up());
    }

    void CameraManBird::moveBy(const cellar::Vec2f& displacement)
    {
        if(_camera == nullptr)
            return;

        cellar::Vec3f front = _camera->tripod().to() - _camera->tripod().from();
        cellar::Vec3f up = _camera->tripod().up();
        cellar::Vec3f side = cross(front, up);

        cellar::Vec3f move = displacement.x()*side + displacement.y()*up;

        _camera->setTripod(_camera->tripod().from() + move,
                           _camera->tripod().to() + move,
                           _camera->tripod().up());
    }

    void CameraManBird::rotate(float radians)
    {
        if(_camera == nullptr)
            return;

        cellar::Vec3f front = (_camera->tripod().to() - _camera->tripod().from()).normalized();
        cellar::Vec3f nUp =  cellar::rotate(front.x(), front.y(), front.z(), radians) * 
                             cellar::Vec4f(_camera->tripod().up(), 0.0);
        _camera->setTripod(_camera->tripod().from(),
                           _camera->tripod().to(),
                           nUp);
    }

    void CameraManBird::setupCamera()
    {
        if(_camera == nullptr)
            return;

        _camera->setMode(Camera::FRAME);

        _camera->setFrame((float)_camera->knowWindowWidth(),
                          (float)_camera->knowWindowHeight());

        _camera->setLens(Camera::Lens::ORTHOGRAPHIC,
                         -_camera->knowWindowWidth() / 2.0f,  _camera->knowWindowWidth() / 2.0f,
                         -_camera->knowWindowHeight() / 2.0f, _camera->knowWindowHeight() / 2.0f,
                         -1.0f, 1.0f);

        _camera->setTripod(cellar::Vec3f(_camera->knowWindowWidth() / 2.0f,
                                           _camera->knowWindowHeight() / 2.0f,
                                           0.0f),
                           cellar::Vec3f(_camera->knowWindowWidth() / 2.0f,
                                           _camera->knowWindowHeight() / 2.0f,
                                           -1.0f),
                           cellar::Vec3f(0.0f, 1.0f, 0.0f));
    }
}