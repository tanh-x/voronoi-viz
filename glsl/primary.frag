#version 330 core

in vec3 vertexColor;
out vec4 FragColor;

void main() {
    FragColor = vec4(1.0f, 0.5f, 0.5f, 1.0f);
//    FragColor = vec4(vertexColor.xy, 0.2f, 1.0f);
}