#version 330 core

layout (location = 0) in vec2 aPos;
uniform mat4 projection;  // Orthographic projection matrix

void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
//    gl_Position = projection * vec4(aPos, 0.0, 1.0);
//    vertexColor = vec3(aPos.x * 2, aPos.y * 2, 1);
}