#include <iostream>
#include <array>
#include <cassert>
#include "graphics/Renderer.hpp"
#include "utils/files.hpp"

#define WINDOW_TITLE "Voronoi Visualization"

Renderer::Renderer(int resolutionX, int resolutionY) {
    this->resolutionX = resolutionX;
    this->resolutionY = resolutionY;

    // Boilerplate stuff
    glfwInit();
    this->window = glfwCreateWindow(this->resolutionX, this->resolutionY, WINDOW_TITLE, nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    glfwMakeContextCurrent(window);
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        throw std::runtime_error("Failed to initialize GLEW");
    }

    // Load shader source files
    vertSrc = readFile("glsl/primary.vert");
    fragSrc = readFile("glsl/primary.frag");

    // Let OpenGL compile and initialize the shaders
    vertShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertShader, 1, &vertSrc, nullptr);
    glCompileShader(vertShader);
    fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShader, 1, &fragSrc, nullptr);
    glCompileShader(fragShader);
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertShader);
    glAttachShader(shaderProgram, fragShader);
    glLinkProgram(shaderProgram);
    glDeleteShader(vertShader);
    glDeleteShader(fragShader);

    // Initialize grid elements
    glGenVertexArrays(1, &gridVAO);
    glGenBuffers(1, &gridVBO);
    glBindVertexArray(gridVAO);
    glBindBuffer(GL_ARRAY_BUFFER, gridVBO);

    std::vector<float> gridVertices;
    createGrid(gridVertices, 0.05f, 40);

    glBufferData(GL_ARRAY_BUFFER, gridVertices.size() * sizeof(float), gridVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

}

void Renderer::initVertexObjects(DCEL* geometry) {
    this->dcel = geometry;

    for (auto &e: geometry->halfEdges) {
        edgeEndpoints.push_back(float(dcel->getCenteredX(e->origin->pos.x)));
        edgeEndpoints.push_back(float(dcel->getCenteredY(e->origin->pos.y)));
        edgeEndpoints.push_back(float(dcel->getCenteredX(e->dest->pos.x)));
        edgeEndpoints.push_back(float(dcel->getCenteredY(e->dest->pos.y)));
    }

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(
        GL_ARRAY_BUFFER,
        edgeEndpoints.size() * sizeof(float),
        edgeEndpoints.data(),
        GL_STATIC_DRAW
    );
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    vertexObjsInitialized = true;
}

void Renderer::startRender() {
    if (!vertexObjsInitialized) throw std::runtime_error("Vertex objects not initialized (no vertex data)");

    // Set up the orthographic projection matrix
    float near = -10.0f;
    float far = 10.0f;
    std::array<float, 16> projectionMatrix = orthographicProjection(
        dcel->bottomLeftBounds,
        dcel->topRightBounds,
        near, far
    );
    int projLoc = glGetUniformLocation(shaderProgram, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, projectionMatrix.data());

    // Grid element
    int colorLocation = glGetUniformLocation(shaderProgram, "gridColor");
    glUniform4f(colorLocation, 0.5f, 0.5f, 0.9f, 1.0f);

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shaderProgram);

//        glEnable(GL_BLEND);
//        glLineWidth(0.5f);
//        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//        renderGrid();
//        glDisable(GL_BLEND);

        glBindVertexArray(VAO);
        glLineWidth(2.0f);
        glDrawArrays(GL_LINES, 0, edgeEndpoints.size() / 2);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}


void Renderer::terminate() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
    glfwTerminate();
}

void Renderer::createGrid(std::vector<float> &vertices, float gridSize, int numLines) {
    assert(lineCount == 0);

    std::vector<float> gridVertices;
    float gapSize = gridSize * static_cast<float>(numLines);

    for (int i = 0; i <= numLines; i++) {
        float pos = i * gridSize;

        // Vertical lines
        vertices.push_back(pos);
        vertices.push_back(0);
        vertices.push_back(pos);
        vertices.push_back(gapSize + (i * 0.005f));

        // Horizontal lines
        vertices.push_back(0);
        vertices.push_back(pos);
        vertices.push_back(gapSize + i * 0.005f);
        vertices.push_back(pos);

        lineCount += 2;
    }
}

void Renderer::renderGrid() const {
    glBindVertexArray(gridVAO);
    glDrawArrays(GL_LINES, 0, lineCount);
    glBindVertexArray(0);
}
