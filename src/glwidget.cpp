#include "glwidget.h"

#include "cs123_lib/resourceloader.h"
#include "cs123_lib/errorchecker.h"
#include <QMouseEvent>
#include <QWheelEvent>
#include <iostream>
#include "settings.h"

#include "openglshape.h"
#include "gl/textures/Texture2D.h"
#include "gl/shaders/ShaderAttribLocations.h"
#include "sphere.h"

#define PI 3.14159265f

using namespace CS123::GL;

GLWidget::GLWidget(QGLFormat format, QWidget *parent)
    : QGLWidget(format, parent),
      m_width(width()), m_height(height()),
      m_phongProgram(0), m_textureProgram(0), m_horizontalBlurProgram(0), m_verticalBlurProgram(0),
      m_lavaProgram(0), m_lava(nullptr),
      m_quad(nullptr), m_sphere(nullptr),
      m_blurFBO1(nullptr), m_blurFBO2(nullptr),
      m_particlesFBO1(nullptr), m_particlesFBO2(nullptr),
      m_firstPass(true), m_evenPass(true), m_numParticles(5000),
      // particles
      //      m_angleX(-0.5f), m_angleY(0.5f), m_zoom(4.f)
      // end
      m_angleX(0), m_angleY(0.15f), m_zoom(10.f),
      mTexture(QOpenGLTexture::TargetCubeMap),
      mVertexBuf(QOpenGLBuffer::VertexBuffer),
      mSpeed(0.0f)
{
}

GLWidget::~GLWidget()
{
    glDeleteVertexArrays(1, &m_particlesVAO);
}

void GLWidget::initializeGL() {
    ResourceLoader::initializeGlew();
    glEnable(GL_DEPTH_TEST);

    // Set the color to set the screen when the color buffer is cleared.
    glClearColor(0.164, 0.223, 0.396, 0.0f);

    // Create shader programs.
    m_phongProgram = ResourceLoader::createShaderProgram(
                ":/shaders/phong.vert", ":/shaders/phong.frag");
    m_textureProgram = ResourceLoader::createShaderProgram(
                ":/shaders/quad.vert", ":/shaders/texture.frag");
    m_horizontalBlurProgram = ResourceLoader::createShaderProgram(
                ":/shaders/quad.vert", ":/shaders/horizontalBlur.frag");
    m_verticalBlurProgram = ResourceLoader::createShaderProgram(
                ":/shaders/quad.vert", ":/shaders/verticalBlur.frag");
    m_particleUpdateProgram = ResourceLoader::createShaderProgram(
                ":/shaders/quad.vert", ":/shaders/particles_update.frag");
    m_particleDrawProgram = ResourceLoader::createShaderProgram(
                ":/shaders/particles_draw.vert", ":/shaders/particles_draw.frag");

    /****** TERRAIN *******/
    m_program = ResourceLoader::createShaderProgram(":/shaders/shader.vert", ":/shaders/shader.frag");

    // Initialize sphere OpenGLShape.
    std::vector<GLfloat> sphereData = SPHERE_VERTEX_POSITIONS;
    m_sphere = std::make_unique<OpenGLShape>();
    m_sphere->setVertexData(&sphereData[0], sphereData.size(), VBO::GEOMETRY_LAYOUT::LAYOUT_TRIANGLES, NUM_SPHERE_VERTICES);
    m_sphere->setAttribute(ShaderAttrib::POSITION, 3, 0, VBOAttribMarker::DATA_TYPE::FLOAT, false);
    m_sphere->setAttribute(ShaderAttrib::NORMAL, 3, 0, VBOAttribMarker::DATA_TYPE::FLOAT, false);
    m_sphere->buildVAO();

    std::vector<GLfloat> quadData = {
        -1, 1, 0,
        0, 1,
        -1,-1, 0,
        0, 0,
        1, 1, 0,
        1, 1,
        1,-1, 0,
        1, 0
    };
    m_quad = std::make_unique<OpenGLShape>();
    m_quad->setVertexData(&quadData[0], quadData.size(), VBO::GEOMETRY_LAYOUT::LAYOUT_TRIANGLE_STRIP, 4);
    m_quad->setAttribute(ShaderAttrib::POSITION, 3, 0, VBOAttribMarker::DATA_TYPE::FLOAT, false);
    m_quad->setAttribute(ShaderAttrib::TEXCOORD0, 2, 3*sizeof(GLfloat), VBOAttribMarker::DATA_TYPE::FLOAT, false);
    m_quad->buildVAO();
    /****** TERRAIN END *******/

    /****** LAVA SEA *******/
    m_lavaProgram = ResourceLoader::createShaderProgram(":/shaders/shader.vert", ":/shaders/lavashader.frag");
    std::vector<GLfloat> squareData = {-1.f, -1.f, 0.f,
                                       -1.f, +1.f, 0.f,
                                       +1.f, -1.f, 0.f,
                                       +1.f, +1.f, 0.f};
    m_lava = std::make_unique<OpenGLShape>();
    m_lava->setVertexData(&squareData[0], squareData.size(), VBO::GEOMETRY_LAYOUT::LAYOUT_TRIANGLE_STRIP, 4);
    m_lava->setAttribute(ShaderAttrib::POSITION, 3, 0, VBOAttribMarker::DATA_TYPE::FLOAT, false);
    m_lava->buildVAO();

    std::vector<GLfloat> lavaData;
    lavaData = {-1, -1, 0,
                0,  0,
                +1, -1, 0,
                1,  0,
                -1, +1, 0,
                0,  1,
                +1, +1, 0,
                1,  1    };
    m_quad = std::make_unique<OpenGLShape>();
    m_quad->setVertexData(&lavaData[0], lavaData.size(), VBO::GEOMETRY_LAYOUT::LAYOUT_TRIANGLE_STRIP, 4);
    m_quad->setAttribute(ShaderAttrib::POSITION, 3, 0, VBOAttribMarker::DATA_TYPE::FLOAT, false);
    m_quad->setAttribute(ShaderAttrib::TEXCOORD0, 2, 3*sizeof(GLfloat), VBOAttribMarker::DATA_TYPE::FLOAT, false);
    m_quad->buildVAO();
    /****** LAVA SEA END *******/

    // VAO to draw our particles' triangles
    glGenVertexArrays(1, &m_particlesVAO);

    m_particlesFBO1 = std::make_shared<FBO>(2, FBO::DEPTH_STENCIL_ATTACHMENT::NONE, m_numParticles, 1, TextureParameters::WRAP_METHOD::CLAMP_TO_EDGE, TextureParameters::FILTER_METHOD::NEAREST, GL_FLOAT);
    m_particlesFBO2 = std::make_shared<FBO>(2, FBO::DEPTH_STENCIL_ATTACHMENT::NONE, m_numParticles, 1, TextureParameters::WRAP_METHOD::CLAMP_TO_EDGE, TextureParameters::FILTER_METHOD::NEAREST, GL_FLOAT);

    // Print the max FBO dimension
    GLint maxRenderBufferSize;
    glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE_EXT, &maxRenderBufferSize);
    std::cout << "Max FBO size: " << maxRenderBufferSize << std::endl;

    /******* TERRAIN ********/
    ResourceLoader::initializeGlew();
    resizeGL(width(), height());

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    // Set the color to set the screen when the color buffer is cleared.
    //    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    m_program = ResourceLoader::createShaderProgram(":/shaders/shader.vert", ":/shaders/shader.frag");

    std::vector<glm::vec3> data = m_terrain.init();
    glPolygonMode(GL_FRONT_AND_BACK, m_terrain.isFilledIn() ? GL_FILL : GL_LINE);
    // Initialize openGLShape.
    m_terrain.openGLShape = std::make_unique<OpenGLShape>();
    m_terrain.openGLShape->setVertexData(&data[0][0], data.size() * 3, VBO::GEOMETRY_LAYOUT::LAYOUT_TRIANGLE_STRIP, 2 * data.size());
    m_terrain.openGLShape->setAttribute(ShaderAttrib::POSITION, 3, 0, VBOAttribMarker::DATA_TYPE::FLOAT, false);
    m_terrain.openGLShape->setAttribute(ShaderAttrib::NORMAL, 3, sizeof(glm::vec3), VBOAttribMarker::DATA_TYPE::FLOAT, false);
    m_terrain.openGLShape->buildVAO();

    rebuildMatrices();
    /******** TERRAIN END *******/
    //    initializeOpenGLFunctions();

    mProgram.addShaderFromSourceCode(
                QOpenGLShader::Vertex,
                R"(
                    attribute vec3 aPosition;
                    varying vec3 vTexCoord;
                    uniform mat4 mvpMatrix;

                    void main()
                    {
                        gl_Position = mvpMatrix * vec4(aPosition, 1.0);
                        vTexCoord = aPosition;
                    }
                    )");

    mProgram.addShaderFromSourceCode(
                QOpenGLShader::Fragment,
                R"(
                    uniform samplerCube uTexture;
                    varying vec3 vTexCoord;

                    void main()
                    {
                        gl_FragColor = textureCube(uTexture, vTexCoord);
                    }
                    )");

    mProgram.link();
    mProgram.bind();

    loadImages();

    QVector3D vertices[] =
    {
        {-1.0f,  1.0f, -1.0f},
        {-1.0f, -1.0f, -1.0f},
        {+1.0f, -1.0f, -1.0f},
        {+1.0f, -1.0f, -1.0f},
        {+1.0f, +1.0f, -1.0f},
        {-1.0f, +1.0f, -1.0f},

        {-1.0f, -1.0f, +1.0f},
        {-1.0f, -1.0f, -1.0f},
        {-1.0f, +1.0f, -1.0f},
        {-1.0f, +1.0f, -1.0f},
        {-1.0f, +1.0f, +1.0f},
        {-1.0f, -1.0f, +1.0f},

        {+1.0f, -1.0f, -1.0f},
        {+1.0f, -1.0f, +1.0f},
        {+1.0f, +1.0f, +1.0f},
        {+1.0f, +1.0f, +1.0f},
        {+1.0f, +1.0f, -1.0f},
        {+1.0f, -1.0f, -1.0f},

        {-1.0f, -1.0f, +1.0f},
        {-1.0f, +1.0f, +1.0f},
        {+1.0f, +1.0f, +1.0f},
        {+1.0f, +1.0f, +1.0f},
        {+1.0f, -1.0f, +1.0f},
        {-1.0f, -1.0f, +1.0f},

        {-1.0f, +1.0f, -1.0f},
        {+1.0f, +1.0f, -1.0f},
        {+1.0f, +1.0f, +1.0f},
        {+1.0f, +1.0f, +1.0f},
        {-1.0f, +1.0f, +1.0f},
        {-1.0f, +1.0f, -1.0f},

        {-1.0f, -1.0f, -1.0f},
        {-1.0f, -1.0f, +1.0f},
        {+1.0f, -1.0f, -1.0f},
        {+1.0f, -1.0f, -1.0f},
        {-1.0f, -1.0f, +1.0f},
        {+1.0f, -1.0f, +1.0f}
    };

    mVertexBuf.create();
    mVertexBuf.bind();
    mVertexBuf.allocate(vertices, 36 * sizeof(QVector3D));

    mProgram.enableAttributeArray("aPosition");
    mProgram.setAttributeBuffer("aPosition",
                                GL_FLOAT,
                                0,
                                3,
                                sizeof(QVector3D));

    mProgram.setUniformValue("uTexture", 0);
}

void GLWidget::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT);
    switch (settings.mode) {
    case MODE_BLUR:
        drawBlur();
        break;
    case MODE_PARTICLES:
        drawParticles();
        update();
        break;
    }

    /******* TERRAIN *******/
    //    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Bind shader program
    glUseProgram(m_program);

    // Set uniforms
    glUniformMatrix4fv(glGetUniformLocation(m_program, "model"), 1, GL_FALSE, glm::value_ptr(m_model));
    glUniformMatrix4fv(glGetUniformLocation(m_program, "view"), 1, GL_FALSE, glm::value_ptr(m_view));
    glUniformMatrix4fv(glGetUniformLocation(m_program, "projection"), 1, GL_FALSE, glm::value_ptr(m_projection));

    glUniform1i(glGetUniformLocation(m_program, "rock_tex"), 0);

    // Draw terrain
    m_terrain.draw();
    // Unbind shader program
    glUseProgram(0);
    /****** TERRAIN END *******/

    /****** LAVA SEA *******/
    // Bind lava shader program
    glUseProgram(m_lavaProgram);
    // Set uniforms
    glUniformMatrix4fv(glGetUniformLocation(m_lavaProgram, "model"), 1, GL_FALSE, glm::value_ptr(m_model));
    glUniformMatrix4fv(glGetUniformLocation(m_lavaProgram, "view"), 1, GL_FALSE, glm::value_ptr(m_view));
    glUniformMatrix4fv(glGetUniformLocation(m_lavaProgram, "projection"), 1, GL_FALSE, glm::value_ptr(m_projection));
    // Set color
    glUniform3f(glGetUniformLocation(m_lavaProgram, "color"), 2.8f, 0.7f, 0);
    // Move lava down
    glm::mat4 transMat  = glm::translate(glm::vec3(0, -0.4, 0));
    int modelUniLoc     = glGetUniformLocation(m_lavaProgram, "model");
    glUniformMatrix4fv(modelUniLoc, 1, GL_FALSE, glm::value_ptr(transMat));
    // Scale to be bigger and rotate
    glm::mat4 scaleMat  = glm::scale(glm::vec3(10.f));
    glm::mat4 rotMat    = glm::rotate(glm::radians(96.f), glm::vec3(1.0, 0, 0));
    glUniformMatrix4fv(modelUniLoc, 1, GL_FALSE, glm::value_ptr(rotMat * scaleMat + transMat));
    m_lava->draw();
    // Unbind shader program
    glUseProgram(0);
    /****** LAVA SEA END *******/
}

void GLWidget::drawBlur() {
    // TODO: [Task 1] Do drawing here!
    m_blurFBO1->bind();

    glClear(GL_COLOR_BUFFER_BIT);
    glClear(GL_DEPTH_BUFFER_BIT);

    glUseProgram(m_phongProgram);

    GLint viewLoc   = glGetUniformLocation(m_phongProgram, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(m_view));
    GLint projecLoc = glGetUniformLocation(m_phongProgram, "projection");
    glUniformMatrix4fv(projecLoc, 1, GL_FALSE, glm::value_ptr(m_projection));
    GLint modelLoc  = glGetUniformLocation(m_phongProgram, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(glm::translate(glm::vec3(0, 1.2, 0))));
    glViewport(0, 0, m_width, m_height);

    m_sphere->draw();

    m_blurFBO1->getColorAttachment(0).bind();
    m_blurFBO1->unbind();

    glClear(GL_COLOR_BUFFER_BIT);
    glClear(GL_DEPTH_BUFFER_BIT);
    glUseProgram(m_horizontalBlurProgram);

    glViewport(0, 0, m_width, m_height);
    m_blurFBO2->bind();
    glClear(GL_COLOR_BUFFER_BIT);
    glClear(GL_DEPTH_BUFFER_BIT);
    m_quad->draw();
    m_blurFBO2->unbind();

    m_blurFBO2->getColorAttachment(0).bind();
    glUseProgram(m_verticalBlurProgram);
    glViewport(0, 0, m_width, m_height);

    m_quad->draw();
}

void GLWidget::drawParticles() {
    auto prevFBO = m_evenPass ? m_particlesFBO1 : m_particlesFBO2;
    auto nextFBO = m_evenPass ? m_particlesFBO2 : m_particlesFBO1;
    float firstPass = m_firstPass ? 1.0f : 0.0f;

    // Move the particles from prevFBO to nextFBO while updating them
    nextFBO->bind();
    glUseProgram(m_particleUpdateProgram);

    glActiveTexture(GL_TEXTURE0);
    prevFBO->getColorAttachment(0).bind();
    glActiveTexture(GL_TEXTURE1);
    prevFBO->getColorAttachment(1).bind();

    GLint firstPassLoc = glGetUniformLocation(m_particleUpdateProgram, "firstPass");
    glUniform1f(firstPassLoc, firstPass);
    GLint numParticlesLoc = glGetUniformLocation(m_particleUpdateProgram, "numParticles");
    glUniform1i(numParticlesLoc, m_numParticles);


    // Two textures sent as ints
    GLint prevPosLoc = glGetUniformLocation(m_particleUpdateProgram, "prevPos");
    glUniform1i(prevPosLoc, 0);
    GLint prevVelLoc = glGetUniformLocation(m_particleUpdateProgram, "prevVel");
    glUniform1i(prevVelLoc, 1);

    m_quad->draw();

    // Draw the particles from nextFBO
    nextFBO->unbind();

    glClear(GL_COLOR_BUFFER_BIT);
    glClear(GL_DEPTH_BUFFER_BIT);

    glUseProgram(m_particleDrawProgram);
    setParticleViewport();

    glActiveTexture(GL_TEXTURE0);
    nextFBO->getColorAttachment(0).bind();
    glActiveTexture(GL_TEXTURE1);
    nextFBO->getColorAttachment(1).bind();

    GLint modelLoc = glGetUniformLocation(m_particleDrawProgram, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &m_model[0][0]);
    GLint viewLoc = glGetUniformLocation(m_particleDrawProgram, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &m_view[0][0]);
    GLint projectionLoc = glGetUniformLocation(m_particleDrawProgram, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, &m_projection[0][0]);

    GLint posLoc = glGetUniformLocation(m_particleDrawProgram, "pos");
    glUniform1i(posLoc, 0);
    GLint velLoc = glGetUniformLocation(m_particleDrawProgram, "vel");
    glUniform1i(velLoc, 1);
    numParticlesLoc = glGetUniformLocation(m_particleDrawProgram, "numParticles");
    glUniform1i(numParticlesLoc, m_numParticles);

    // draw triangles
    glBindVertexArray(m_particlesVAO);
    glDrawArrays(GL_TRIANGLES, 0, 3 * m_numParticles);  // m_particlesVAO for second argument??
    glBindVertexArray(0);                               // Unbind the VAO?

    glActiveTexture(GL_TEXTURE0);

    m_firstPass = false;
    m_evenPass = !m_evenPass;
}

// This is called at the beginning of the program between initializeGL and
// the first paintGL call, as well as every time the window is resized.
void GLWidget::resizeGL(int w, int h) {
    m_width = w;
    m_height = h;

    // TODO: [Task 5] Initialize FBOs here, with dimensions m_width and m_height.
    m_blurFBO1 = std::make_unique<FBO>(1, FBO::DEPTH_STENCIL_ATTACHMENT::DEPTH_ONLY, w, h, TextureParameters::WRAP_METHOD::CLAMP_TO_EDGE);
    m_blurFBO2 = std::make_unique<FBO>(1, FBO::DEPTH_STENCIL_ATTACHMENT::NONE, w, h, TextureParameters::WRAP_METHOD::CLAMP_TO_EDGE);
    //       [Task 12] Pass in TextureParameters::WRAP_METHOD::CLAMP_TO_EDGE as the last parameter

    /****** TERRAIN ******/
    glViewport(0, 0, w, h);

    rebuildMatrices();
}

// Sets the viewport to ensure that {0,0} is always in the center of the viewport
// in clip space, and to ensure that the aspect ratio is 1:1
void GLWidget::setParticleViewport() {
    int maxDim = std::max(m_width, m_height);
    int x = (m_width - maxDim) / 2.0f;
    int y = (m_height - maxDim) / 2.0f;
    glViewport(x, y, maxDim, maxDim);
}

/// Mouse interaction code below.

void GLWidget::mousePressEvent(QMouseEvent *event) {
    m_prevMousePos = event->pos();
}

void GLWidget::mouseMoveEvent(QMouseEvent *event) {
    m_angleX += 3 * (event->x() - m_prevMousePos.x()) / (float) width();
    //    m_angleY += 3 * (event->y() - m_prevMousePos.y()) / (float) height();
    m_prevMousePos = event->pos();
    rebuildMatrices();
}

void GLWidget::wheelEvent(QWheelEvent *event) {
    m_zoom -= event->delta() / 100.f;
    double temp = m_zoom;
    m_zoom = std::min(temp, 12.0);
    rebuildMatrices();
}

void GLWidget::rebuildMatrices() {
    m_model = glm::mat4(1.f);

    m_view = glm::translate(glm::vec3(0, 0, -m_zoom)) *
            glm::rotate(m_angleY, glm::vec3(1,0,0)) *
            glm::rotate(m_angleX, glm::vec3(0,1,0));

    m_projection = glm::perspective(0.8f, (float)width()/height(), 0.1f, 100.f);
    update();
}

void GLWidget::loadImages()
{
    //    const QImage posx = QImage(":/images/posx.jpg").convertToFormat(QImage::Format_RGBA8888);
    //    const QImage negx = QImage(":/images/negx.jpg").convertToFormat(QImage::Format_RGBA8888);

    //    const QImage posy = QImage(":/images/posy.jpg").convertToFormat(QImage::Format_RGBA8888);
    //    const QImage negy = QImage(":/images/negy.jpg").convertToFormat(QImage::Format_RGBA8888);

    //    const QImage posz = QImage(":/images/posz.jpg").convertToFormat(QImage::Format_RGBA8888);
    //    const QImage negz = QImage(":/images/negz.jpg").convertToFormat(QImage::Format_RGBA8888);

    //    mTexture.create();
    //    mTexture.setSize(posx.width(), posx.height(), posx.depth());
    //    mTexture.setFormat(QOpenGLTexture::RGBA8_UNorm);
    //    mTexture.allocateStorage();

    //    mTexture.setData(0, 0, QOpenGLTexture::CubeMapPositiveX,
    //                     QOpenGLTexture::RGBA, QOpenGLTexture::UInt8,
    //                     posx.constBits(), Q_NULLPTR);

    //    mTexture.setData(0, 0, QOpenGLTexture::CubeMapPositiveY,
    //                     QOpenGLTexture::RGBA, QOpenGLTexture::UInt8,
    //                     posy.constBits(), Q_NULLPTR);

    //    mTexture.setData(0, 0, QOpenGLTexture::CubeMapPositiveZ,
    //                     QOpenGLTexture::RGBA, QOpenGLTexture::UInt8,
    //                     posz.constBits(), Q_NULLPTR);

    //    mTexture.setData(0, 0, QOpenGLTexture::CubeMapNegativeX,
    //                     QOpenGLTexture::RGBA, QOpenGLTexture::UInt8,
    //                     negx.constBits(), Q_NULLPTR);

    //    mTexture.setData(0, 0, QOpenGLTexture::CubeMapNegativeY,
    //                     QOpenGLTexture::RGBA, QOpenGLTexture::UInt8,
    //                     negy.constBits(), Q_NULLPTR);

    //    mTexture.setData(0, 0, QOpenGLTexture::CubeMapNegativeZ,
    //                     QOpenGLTexture::RGBA, QOpenGLTexture::UInt8,
    //                     negz.constBits(), Q_NULLPTR);

    //    mTexture.setWrapMode(QOpenGLTexture::ClampToEdge);
    //    mTexture.setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
    //    mTexture.setMagnificationFilter(QOpenGLTexture::LinearMipMapLinear);
}

