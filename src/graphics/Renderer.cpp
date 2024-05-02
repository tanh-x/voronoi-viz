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
    createGrid(gridVertices, 0.1f, 20);

    glBufferData(GL_ARRAY_BUFFER, gridVertices.size() * sizeof(float), gridVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

}

void Renderer::initVertexObjects(DCEL* geometry) {
    this->dcel = geometry;

    // Initialize vertex points data

    for (auto &e: this->dcel->halfEdges) {
        pointVertices.push_back(float(dcel->getCenteredX(e->origin->pos.x)));
        pointVertices.push_back(float(dcel->getCenteredY(e->origin->pos.y)));
        pointVertices.push_back(float(dcel->getCenteredX(e->dest->pos.x)));
        pointVertices.push_back(float(dcel->getCenteredY(e->dest->pos.y)));
    }

    glGenVertexArrays(1, &pointsVAO);
    glGenBuffers(1, &pointsVBO);

    glBindVertexArray(pointsVAO);
    glBindBuffer(GL_ARRAY_BUFFER, pointsVBO);
    glBufferData(
        GL_ARRAY_BUFFER,
        pointVertices.size() * sizeof(float),
        pointVertices.data(),
        GL_STATIC_DRAW
    );
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Initialize outer data
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


void Renderer::initSiteVertexObjects(const std::vector<Vec2> &sites) {
    if (!vertexObjsInitialized) throw std::runtime_error("DCEL not initialized");

    for (auto &s: sites) {
        siteVertices.push_back(float(dcel->getCenteredX(s.x)));
        siteVertices.push_back(float(dcel->getCenteredY(s.y)));
    }

    glGenVertexArrays(1, &sitesVAO);
    glGenBuffers(1, &sitesVBO);

    glBindVertexArray(sitesVBO);
    glBindBuffer(GL_ARRAY_BUFFER, sitesVBO);
    glBufferData(
        GL_ARRAY_BUFFER,
        siteVertices.size() * sizeof(float),
        siteVertices.data(),
        GL_STATIC_DRAW
    );
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Renderer::startRender() {
    if (!vertexObjsInitialized) throw std::runtime_error("Vertex objects not initialized (no vertex data)");

    // Set up the orthographic projection matrix
//    float near = -10.0f;
//    float far = 10.0f;
//    std::array<float, 16> projectionMatrix = orthographicProjection(
//        dcel->bottomLeftBounds,
//        dcel->topRightBounds,
//        near, far
//    );
//    int projLoc = glGetUniformLocation(shaderProgram, "projection");
//    glUniformMatrix4fv(projLoc, 1, GL_FALSE, projectionMatrix.data());

    // Grid element
    int colorLocation = glGetUniformLocation(shaderProgram, "gridColor");
    glUniform4f(colorLocation, 0.5f, 0.5f, 0.9f, 1.0f);

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shaderProgram);

        // Render grid
//        glEnable(GL_BLEND);
//        glLineWidth(0.5f);
//        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//        renderGrid();
//        glDisable(GL_BLEND);

        // Render edges
        glBindVertexArray(VAO);
        glLineWidth(2.0f);
        glDrawArrays(GL_LINES, 0, edgeEndpoints.size() / 2);

        // Render points
        glBindVertexArray(pointsVAO);
        glPointSize(6.0f); // Set the size of the points
        glDrawArrays(GL_POINTS, 0, pointVertices.size() / 2);

        glBindVertexArray(sitesVAO);
        glPointSize(8.0f); // Set the size of the points
        glDrawArrays(GL_POINTS, 0, siteVertices.size() / 2);

        glBindVertexArray(0);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}


void Renderer::terminate() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &pointsVAO);
    glDeleteBuffers(1, &pointsVBO);
    glDeleteVertexArrays(1, &gridVAO);
    glDeleteBuffers(1, &gridVBO);
    glDeleteVertexArrays(1, &sitesVAO);
    glDeleteBuffers(1, &sitesVBO);
    glDeleteProgram(shaderProgram);
    glfwTerminate();
}

void Renderer::createGrid(std::vector<float> &vertices, float gridSize, int numLines) {
    assert(lineCount == 0);

    std::vector<float> gridVertices;
    float gapSize = gridSize * static_cast<float>(numLines);

    for (int i = 0; i <= numLines; i++) {
        float pos = static_cast<float>(i) * gridSize;

        // Vertical lines
        vertices.push_back(pos);
        vertices.push_back(0);
        vertices.push_back(pos);
        vertices.push_back(gapSize);

        // Horizontal lines
        vertices.push_back(0);
        vertices.push_back(pos);
        vertices.push_back(gapSize);
        vertices.push_back(pos);

        lineCount += 2;
    }
}

void Renderer::renderGrid() const {
    glBindVertexArray(gridVAO);
    glDrawArrays(GL_LINES, 0, lineCount);
    glBindVertexArray(0);
}
