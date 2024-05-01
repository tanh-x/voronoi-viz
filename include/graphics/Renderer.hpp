#ifndef VORONOI_VIZ_RENDERER_HPP
#define VORONOI_VIZ_RENDERER_HPP

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "geometry/DCEL.hpp"

class Renderer {
public:
    Renderer(int resolutionX, int resolutionY);

    void initVertexObjects(DCEL* geometry);

    void initSiteVertexObjects(const std::vector<Vec2> &sites);

    void startRender();

    void terminate();

private:
    DCEL* dcel = nullptr;

    // Window and context stuff
    GLFWwindow* window;
    int resolutionX;
    int resolutionY;

    // Shader stuff
    const char* vertSrc;
    int vertShader;
    const char* fragSrc;
    int fragShader;
    int shaderProgram;

    // Vertex data stuff
    // Main geometry
    GLuint VAO {};
    GLuint VBO {};
    GLuint pointsVAO {};
    GLuint pointsVBO {};
    GLuint sitesVAO {};
    GLuint sitesVBO {};
    std::vector<float> edgeEndpoints {};
    std::vector<float> pointVertices {};
    std::vector<float> siteVertices {};
    bool vertexObjsInitialized = false;

    // Sites vertices

    // Other elements of the view port
    GLuint gridVAO {};
    GLuint gridVBO {};
    int lineCount = 0;

    // Methods
    void createGrid(std::vector<float> &vertices, float gridSize, int numLines);

    void renderGrid() const;


};

#endif //VORONOI_VIZ_RENDERER_HPP
