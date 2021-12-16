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
    float val = -1.0 + 2.0 * glm::fract(sin(row * 127.1f + col * 311.7f) * 43758.5453123f);
    if (row == 10 && col == 10) {
        return -1;
    }
    if ((row == 9 && col == 10) || (row == 10 && col == 9) || (row == 11 && col == 10) || (row == 10 && col == 11)) {
        row = 9;
        col = 9;
    }

    float r = float(row)-10.0;
    float c = float(col)-10.0;
    return (r*r+c*c+25)/(r*r+c*c+5)+val*0.3-1.8; // hyperbola

    // return -sqrt(0.05*(r*r+c*c))+val*0.15+1.8f;

    return val;
}


/**
 * Returns the object-space position for the terrain vertex at the given row
 * and column.
 */
float Terrain::getPositionHelper(int row, int col, float n, float amp)
{
    int newRow = glm::floor(row / n);
    int newCol = glm::floor(col / n);

    float A = randValue(newRow, newCol);
    float B = randValue(newRow, newCol + 1);
    float x = col / n - newCol;

    float C = randValue(newRow + 1, newCol);
    float D = randValue(newRow + 1, newCol + 1);

    float AB = glm::mix(A, B, x*x*(3 - 2*x));
    float CD = glm::mix(C, D, x*x*(3 - 2*x));

    float y = row / n - newRow;
    return amp * glm::mix(AB, CD, y*y*(3 - 2*y));
}

glm::vec3 Terrain::getPosition(int row, int col)
{
    glm::vec3 position;
    position.x = 10 * row/m_numRows - 5;
    position.z = 10 * col/m_numCols - 5;

    // TODO: Adjust position.y using value noise.
//    position.y = getPositionHelper(row, col, 20.0, 1)
//            + getPositionHelper(row, col, 10.0, 0.5)
//            + getPositionHelper(row, col, 5.0, 0.25);

    position.y = getPositionHelper(row, col, 5.0, 1);
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

    int ind[9][2] = {{0, 1}, {-1, 1}, {-1, 0}, {-1, -1}, {0, -1}, {1, -1}, {1, 0}, {1, 1}, {0, 1}};
    glm::vec3 n0, n1;
    glm::vec3 p = getPosition(row, col);

    glm::vec3 aNorm = glm::vec3(0.0, 0.0, 0.0);
    for (int i = 0; i < 8; i ++) {
        n0 = getPosition(row + ind[i][0], col + ind[i][1]);
        n1 = getPosition(row + ind[i + 1][0], col + ind[i + 1][1]);
        aNorm += glm::normalize(glm::cross(n1 - p, n0 - p));
    }

    return glm::normalize(aNorm/8.0f);

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
