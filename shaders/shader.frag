#version 330 core

in vec3 WS_position; // world-space position
in vec3 WS_normal;   // world-space normal

//in vec2 uv;

out vec3 fragColor;

//uniform sampler2D rock_tex;

void main(){
    vec3 WS_toLight = normalize(vec3(10.0) - WS_position);
    vec3 color = vec3(0.43, 0.28, 0.2);
    fragColor = color * vec3(0.3 + 0.7 * max(0.0, dot(normalize(WS_normal), WS_toLight)));

//    fragColor = vec3(texture(rock_tex, uv));
}
