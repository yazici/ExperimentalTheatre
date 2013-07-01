#ifndef MEDIAWORKBENCH_CAMERA_H
#define MEDIAWORKBENCH_CAMERA_H

#include "libMediaWorkbench_global.h"
#include <DesignPattern/SpecificSubject.h>
#include <DataStructure/Vector.h>
#include <DataStructure/Matrix.h>


namespace media
{
    struct CameraMsg;

    class MEDIA_EXPORT Camera : public cellar::SpecificSubject<CameraMsg>
    {
    public:

        class MEDIA_EXPORT Frame
        {
            friend class Camera;

        public:
            Frame();
            Frame(float width, float height);

            float width() const;
            float height() const;

        private:
            void set(float width, float height);

            float _width;
            float _height;
        };

        class MEDIA_EXPORT Lens
        {            
            friend class Camera;

        public:
            enum Type {ORTHOGRAPHIC, PERSPECTIVE};

            Lens();
            Lens(Type type,
                 float left,       float right,
                 float bottom,     float top,
                 float nearPlane,  float farPlane);

            Type   type() const;
            float left() const;
            float right() const;
            float bottom() const;
            float top() const;
            float nearPlane() const;
            float farPlane() const;
            float width() const;
            float height() const;

        private:
            void set(Type type,
                     float left,         float right,
                     float bottom,       float top,
                     float nearPlane,    float farPlane);

            Type _type;
            float _left;
            float _right;
            float _bottom;
            float _top;
            float _nearPlane;
            float _farPlane;
        };

        class MEDIA_EXPORT Tripod
        {
            friend class Camera;

        public:
            Tripod();
            Tripod(const cellar::Vec3f& from,
                   const cellar::Vec3f& to,
                   const cellar::Vec3f& up);

            const cellar::Vec3f& from() const;
            const cellar::Vec3f& to() const;
            const cellar::Vec3f& up() const;

        private :
            void set(const cellar::Vec3f& from,
                     const cellar::Vec3f& to,
                     const cellar::Vec3f& up);
            void moveBy(const cellar::Vec3f& dist);

            cellar::Vec3f _from;
            cellar::Vec3f _to;
            cellar::Vec3f _up;
        };

    public:
        enum Mode {EXPAND, FRAME, STRETCH};

        Camera();
        Camera(Mode mode, const Frame &frame,
               const Lens &lens, const Tripod &tripod,
               int windowWidth, int windowHeight);

        Mode    mode() const;
        void    setMode(Mode mode);

        const Frame&  frame();
        const Lens&   lens();
        const Tripod& tripod();


        // Modifiers
        void setFrame(float width, float height);
        void setLens(float left,         float right,
                     float bottom,       float top);
        void setLens(Lens::Type type,
                     float left,         float right,
                     float bottom,       float top,
                     float nearPlane,    float farPlane);
        void setTripod(const cellar::Vec3f& from,
                       const cellar::Vec3f& to,
                       const cellar::Vec3f& up);
        void moveBy(const cellar::Vec3f& dist);
        void refresh();


        // Notify new window size
        void notifyNewWindowSize(int width, int height);
        int  knowWindowWidth() const;
        int  knowWindowHeight() const;

        void zoom(float ratio);

        cellar::Mat4f& projectionMatrix();
        cellar::Mat4f& viewMatrix();

    private:
        void updateMatrices();
        void updateProjectionMatrix();
        void updateViewMatrix();

        Mode _mode;
        Frame _frame;
        Lens _lens;
        Tripod _tripod;        
        int _windowWidth;
        int _windowHeight;
        cellar::Mat4f _projMatrix;
        cellar::Mat4f _viewMatrix;
    };

    struct CameraMsg
    {
        enum Change {PROJECTION, VIEW};

        Camera& camera;
        Change change;

    private:
        friend class Camera;
        CameraMsg(Camera& camera, Change change) :
            camera(camera), change(change) {}
    };



    // IMPLEMENTATION //
    // CAMERA //
    inline Camera::Mode Camera::mode() const {return _mode;}
    inline void Camera::setMode(Mode mode) {_mode = mode;}
    inline const Camera::Frame&  Camera::frame() {return _frame;}
    inline const Camera::Lens& Camera::lens() {return _lens;}
    inline const Camera::Tripod &Camera::tripod() {return _tripod;}
    inline int Camera::knowWindowWidth() const {return _windowWidth;}
    inline int Camera::knowWindowHeight() const {return _windowHeight;}
    inline cellar::Mat4f& Camera::projectionMatrix() {return _projMatrix;}
    inline cellar::Mat4f& Camera::viewMatrix() {return _viewMatrix;}

    // FRAME //
    inline float Camera::Frame::width() const {return _width;}
    inline float Camera::Frame::height() const {return _height;}
    inline void  Camera::Frame::set(float width, float height) {_width = width; _height = height;}

    // CAMERA::LENS //
    inline Camera::Lens::Type Camera::Lens::type() const {return _type;}
    inline float Camera::Lens::left() const {return _left;}
    inline float Camera::Lens::right() const {return _right;}
    inline float Camera::Lens::bottom() const {return _bottom;}
    inline float Camera::Lens::top() const {return _top;}
    inline float Camera::Lens::nearPlane() const {return _nearPlane;}
    inline float Camera::Lens::farPlane() const {return _farPlane;}
    inline float Camera::Lens::width() const {return _right - _left;}
    inline float Camera::Lens::height() const {return _top - _bottom;}


    // CAMERA::TRIPOD //
    inline const cellar::Vec3f& Camera::Tripod::from() const {return _from;}
    inline const cellar::Vec3f& Camera::Tripod::to() const {return _to;}
    inline const cellar::Vec3f& Camera::Tripod::up() const {return _up;}

    inline void Camera::Tripod::set(const cellar::Vec3f& from,
                                    const cellar::Vec3f& to,
                                    const cellar::Vec3f& up)
    {
        _from = from; _to = to; _up = up;
    }

    inline void Camera::Tripod::moveBy(const cellar::Vec3f& dist)
        {_from += dist; _to += dist;}
}


#endif // MEDIAWORKBENCH_CAMERA_H