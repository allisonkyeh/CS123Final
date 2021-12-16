#include "terrain.h"

#include <math.h>
#include "gl/shaders/ShaderAttribLocations.h"
#include "iostream"

Terrain::Terrain() : m_numRows(100), m_numCols(m_numRows), m_isFilledIn(true), m_rock_image(QString::fromStdString(":/images/rocky.jpg"))
{
}


/**
 * Returns a pseudo-random value between -1.0 and 1.0 for the given row and
 * column.
 */
float Terrain::randValue(int row, int col)
{

    if (row >= 40 && row <= 60) {
        if (col >= 40 && col <= 45) {
            return .2;
        }
    }

    if (row >= 40 && row <= 60) {
        if (col >= 45 && col <= 55) {
            return 1;
        }
    }


    if (row >= 40 && row <= 60) {
        if (col >= 55 && col <= 60) {
            return .2;
        }
    }

    return -1.0 + 2.0 * glm::fract(sin(row * 127.1f + col * 311.7f) * 43758.5453123f);
}


/**
 * Returns the object-space position for the terrain vertex at the given row
 * and column.
 */
glm::vec3 Terrain::getPosition(int row, int col)
{
    glm::vec3 position;

    position.x = 10 * row/m_numRows - 5;
    position.z = 10 * col/m_numCols - 5;

    float scale = 20.0;
    // TODO: Adjust position.y using value noise.
    for (float i=1; i<10; i *=2.f) {
        float new_row = float(row)/scale * i;
        float new_col = float(col)/scale * i;

        float a = randValue(glm::floor(new_row), glm::floor(new_col)) / 2;
        float b = randValue(glm::floor(new_row), glm::floor(new_col) + 1) /2;
        float c = randValue(glm::floor(new_row) + 1, glm::floor(new_col))/2;
        float d = randValue(glm::floor(new_row) + 1, glm::floor(new_col) + 1)/2;

        float y = glm::fract(new_row);
        float x = glm::fract(new_col);

        //        if (row >= 40 && row <= 60) {
        //            if (col >= 40 && col <= 45) {
        //                a /= 2;
        //                b /= 2;
        //                c /= 2;
        //                d /= 2;
        //            }
        //        }

        //        if (row >= 40 && row <= 60) {
        //            if (col >= 45 && col <= 55) {
        //                a /= 3;
        //                b /= 3;
        //                c /= 3;
        //                d /= 3;
        //            }
        //        }


        //        if (row >= 40 && row <= 60) {
        //            if (col >= 55 && col <= 60) {
        //                a /= 2;
        //                b /= 2;
        //                c /= 2;
        //                d /= 2;
        //            }
        //        }

        //        if (row >= 40 && row <= 60) {
        //            if (col >= 40 && col <= 60) {
        //                position.y = glm::mix(a, b, x);
        ////                a= 2;
        ////                b=2;
        ////                c=2;
        ////                d=2;
        //            }
        //        }

        position.y += glm::mix(glm::mix(a,b,3*x*x-2*x*x*x),glm::mix(c,d,3*x*x-2*x*x*x), 3*y*y-2*y*y*y) / pow(2,i-1);
    }




    //    float d = std::sqrt((center-row)*(center-row) + (center-col)*(center-col));
    //                // linear dropoff function
    //                float linear = (r-d) / r;
    //                if (d<r) {
    //                    m_mask[row * maskW + col] = linear;

    return position;
}


/**
 * Returns the normal vector for the terrain vertex at the given row and
 * column.
 */
glm::vec3 Terrain::getNormal(int row, int col)
{
    // TODO: Compute the normal at the given row and column using the positions
    //       of the neighboring vertices.

    glm::vec3 p = getPosition(row, col);
    glm::vec3 n0 = getPosition(row, col+1);
    glm::vec3 n1 = getPosition(row-1, col+1);
    glm::vec3 n2 = getPosition(row-1, col);
    glm::vec3 n3 = getPosition(row-1, col-1);
    glm::vec3 n4 = getPosition(row, col-1);
    glm::vec3 n5 = getPosition(row+1, col-1);
    glm::vec3 n6 = getPosition(row+1, col);
    glm::vec3 n7 = getPosition(row+1, col+1);


    glm::vec3 normal1 = glm::normalize(glm::cross(n1-p, n0-p));
    glm::vec3 normal2 = glm::normalize(glm::cross(n2-p, n1-p));
    glm::vec3 normal3 = glm::normalize(glm::cross(n3-p, n2-p));
    glm::vec3 normal4 = glm::normalize(glm::cross(n4-p, n3-p));
    glm::vec3 normal5 = glm::normalize(glm::cross(n5-p, n4-p));
    glm::vec3 normal6 = glm::normalize(glm::cross(n6-p, n5-p));
    glm::vec3 normal7 = glm::normalize(glm::cross(n7-p, n6-p));
    glm::vec3 normal8 = glm::normalize(glm::cross(n0-p, n7-p));

    return glm::normalize(normal1 + normal2 + normal3 + normal4 + normal5 + normal6 + normal7 + normal8);
}

bool Terrain::isFilledIn() {
    return m_isFilledIn;
}

/**
 * Initializes the terrain by storing positions and normals in a vertex buffer.
 */
std::vector<glm::vec3> Terrain::init() {
    // Initializes a grid of vertices using triangle strips.
    int numVertices = (m_numRows - 1) * (2 * m_numCols + 2);
//    std::vector<glm::vec3> data(3 * numVertices);

    std::vector<glm::vec3> data(2 * numVertices);
    int index = 0;
    for (int row = 0; row < m_numRows - 1; row++) {
        for (int col = m_numCols - 1; col >= 0; col--) {
            data[index++] = getPosition(row, col);
            data[index++] = getNormal  (row, col);
//            data[index++] = getTextureUV(row, col);
            data[index++] = getPosition(row + 1, col);
            data[index++] = getNormal  (row + 1, col);
//            data[index++] = getTextureUV(row + 1, col);
        }
        data[index++] = getPosition(row + 1, 0);
        data[index++] = getNormal  (row + 1, 0);
//        data[index++] = getTextureUV(row + 1, 0);
        data[index++] = getPosition(row + 1, m_numCols - 1);
        data[index++] = getNormal  (row + 1, m_numCols - 1);
//        data[index++] = getTextureUV(row + 1, m_numCols - 1);
    }

//    addTexture();

    return data;
}


/**
 * Draws the terrain.
 */
void Terrain::draw()
{
//    bindTextures();
    openGLShape->draw();
}

void Terrain::addTexture() {
//    glGenTextures(1, &m_rock_textureID);
//    glBindTexture(GL_TEXTURE_2D, m_rock_textureID);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);// repeats and mirrors the tiled texture
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_rock_image.width(), m_rock_image.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, m_rock_image.bits());
//    glGenerateMipmap(GL_TEXTURE_2D);
}

void Terrain::bindTextures() {
//    glActiveTexture(GL_TEXTURE0 + 0); // Texture unit 0
//    glBindTexture(GL_TEXTURE_2D, m_rock_textureID);
}

glm::vec3 Terrain::getTextureUV(float row, float col) {
//    int numTextureTiles = 9;
//    float u = numTextureTiles * (static_cast<float>(col) / m_numCols);
//    float v = numTextureTiles * (static_cast<float>(row) / m_numRows);
//    return glm::vec3(u, v, 0);
}
