#include "terrain.h"

#include <math.h>
#include "gl/shaders/ShaderAttribLocations.h"

Terrain::Terrain() : m_numRows(100), m_numCols(m_numRows), m_isFilledIn(true)
{
}


/**
 * Returns a pseudo-random value between -1.0 and 1.0 for the given row and
 * column.
 */
float Terrain::randValue(int row, int col)
{
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

    for (float i = 1; i <= 16; i *= 2.0) {
        double gridRow = row / 20.0 * i; // ???????
        double gridCol = col / 20.0 * i;

        float a = randValue(glm::floor(gridRow), glm::floor(gridCol));
        float b = randValue(glm::floor(gridRow), glm::floor(gridCol) + 1);

        float c = randValue(glm::floor(gridRow) + 1, glm::floor(gridCol));
        float d = randValue(glm::floor(gridRow) + 1, glm::floor(gridCol) + 1);

        float x = glm::fract(gridCol);
        float y = glm::fract(gridRow);

        position.y += glm::mix(glm::mix(a, b, 3*(x*x) - 2*(x*x*x)), glm::mix(c, d, 3*(x*x) - 2*(x*x*x)), 3*(y*y) - 2*(y*y*y)) / glm::exp2(i - 1);
    }

    position.z = 10 * col/m_numCols - 5;

    // TODO: Adjust position.y using value noise.

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
    glm::vec3 n0 = getPosition(row, col + 1);
    glm::vec3 n1 = getPosition(row - 1, col + 1);
    glm::vec3 n2 = getPosition(row - 1, col);
    glm::vec3 n3 = getPosition(row - 1, col - 1);
    glm::vec3 n4 = getPosition(row, col - 1);
    glm::vec3 n5 = getPosition(row + 1, col - 1);
    glm::vec3 n6 = getPosition(row + 1, col);
    glm::vec3 n7 = getPosition(row + 1, col + 1);

    glm::vec3 normal0 = glm::normalize(glm::cross(n1 - p, n0 - p));
    glm::vec3 normal1 = glm::normalize(glm::cross(n2 - p, n1 - p));
    glm::vec3 normal2 = glm::normalize(glm::cross(n3 - p, n2 - p));
    glm::vec3 normal3 = glm::normalize(glm::cross(n4 - p, n3 - p));
    glm::vec3 normal4 = glm::normalize(glm::cross(n5 - p, n4 - p));
    glm::vec3 normal5 = glm::normalize(glm::cross(n6 - p, n5 - p));
    glm::vec3 normal6 = glm::normalize(glm::cross(n7 - p, n6 - p));
    glm::vec3 normal7 = glm::normalize(glm::cross(n0 - p, n7 - p));

    return glm::normalize(normal0 + normal1 + normal2 + normal3 + normal4 + normal5 + normal6 + normal7);
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
    std::vector<glm::vec3> data(2 * numVertices);
    int index = 0;
    for (int row = 0; row < m_numRows - 1; row++) {
        for (int col = m_numCols - 1; col >= 0; col--) {
            data[index++] = getPosition(row, col);
            data[index++] = getNormal  (row, col);
            data[index++] = getPosition(row + 1, col);
            data[index++] = getNormal  (row + 1, col);
        }
        data[index++] = getPosition(row + 1, 0);
        data[index++] = getNormal  (row + 1, 0);
        data[index++] = getPosition(row + 1, m_numCols - 1);
        data[index++] = getNormal  (row + 1, m_numCols - 1);
    }

    return data;
}


/**
 * Draws the terrain.
 */
void Terrain::draw()
{
    openGLShape->draw();
}
