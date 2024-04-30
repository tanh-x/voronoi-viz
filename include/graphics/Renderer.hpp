#ifndef VORONOI_VIZ_RENDERER_HPP
#define VORONOI_VIZ_RENDERER_HPP

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "geometry/DCEL.hpp"

class Renderer {
public:
    Renderer(int resolutionX, int resolutionY);

    void initVertexObjects(DCEL* geometry);

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
    std::vector<float> edgeEndpoints {};
    bool vertexObjsInitialized = false;

    // Other elements of the view port
    GLuint gridVAO {};
    GLuint gridVBO {};
    int lineCount = 0;

    // Methods
    void createGrid(std::vector<float> &vertices, float gridSize, int numLines);

    void renderGrid() const;


};

#endif //VORONOI_VIZ_RENDERER_HPP
