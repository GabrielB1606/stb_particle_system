#version 460 core

layout (location = 0) in vec3 a_Pos;

uniform mat4 u_ProjView;
uniform mat4 u_Transform;

void main() {
    gl_Position = u_ProjView * u_Transform * vec4(a_Pos.x , a_Pos.y, a_Pos.z, 1.0);
}