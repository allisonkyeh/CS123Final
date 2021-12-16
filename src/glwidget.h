#ifndef GLWIDGET_H
#define GLWIDGET_H
#include "GL/glew.h"
#include <QGLWidget>
#include <QTimer>

#include "glm/glm.hpp"            // glm::vec*, mat*, and basic glm functions
#include "glm/gtx/transform.hpp"  // glm::translate, scale, rotate
#include "glm/gtc/type_ptr.hpp"   // glm::value_ptr

#include <memory>  // std::unique_ptr

#include "gl/datatype/FBO.h"
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLBuffer>

// terrain
#ifdef __APPLE__
#include <glu.h>
#else
#include <GL/glu.h>
#endif
#include "terrain.h"

class OpenGLShape;

using namespace CS123::GL;

class GLWidget : public QGLWidget {
    Q_OBJECT

public:
    GLWidget(QGLFormat format,
             QWidget *parent = 0);
    ~GLWidget();


protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int w, int h);
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void wheelEvent(QWheelEvent *e);
//    void timerEvent(QTimerEvent *);

private:
    void drawBlur();
    void drawParticles();
    void setParticleViewport();

    void rebuildMatrices();

    int m_width;
    int m_height;

    GLuint m_phongProgram;
    GLuint m_textureProgram;
    GLuint m_horizontalBlurProgram;
    GLuint m_verticalBlurProgram;
    GLuint m_particleUpdateProgram;
    GLuint m_particleDrawProgram;
    GLuint m_lavaProgram;
    std::unique_ptr<OpenGLShape> m_lava;
    std::unique_ptr<OpenGLShape> m_quad;
    std::unique_ptr<OpenGLShape> m_sphere;

    std::unique_ptr<FBO> m_blurFBO1;
    std::unique_ptr<FBO> m_blurFBO2;

    GLuint m_particlesVAO;
    std::shared_ptr<FBO> m_particlesFBO1;
    std::shared_ptr<FBO> m_particlesFBO2;
    bool m_firstPass;
    bool m_evenPass;
    int m_numParticles;

    glm::mat4 m_model, m_view, m_projection;

    /** ID for the shader program. */
    GLuint m_program;

    Terrain m_terrain;

    /** For mouse interaction. */
    float m_angleX, m_angleY, m_zoom;
    QPoint m_prevMousePos;

    /** skybox */
    void loadImages();

    QOpenGLShaderProgram mProgram;
    QOpenGLTexture mTexture;
    QOpenGLBuffer mVertexBuf;
    QBasicTimer mTimer;

    struct
    {
        float verticalAngle;
        float aspectRatio;
        float nearPlane;
        float farPlane;
    } mPerspective;

    struct
    {
        glm::vec3 eye;
        glm::vec3 center;
        glm::vec3 up;
    } mLookAt;


//    QVector3D mRotationAxis;
//    QQuaternion mRotation;

    glm::vec2 mMousePressPosition;
    float mSpeed;

    QString mFrontImagePath;
    QString mBackImagePath;
    QString mTopImagePath;
    QString mBottomImagePath;
    QString mLeftImagePath;
    QString mRightImagePath;



};

#endif // GLWIDGET_H
