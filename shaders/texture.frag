#version 330 core

in vec2 uv;

uniform sampler2D tex;

out vec4 fragColor;

void main(){
    fragColor = vec4(1.0, 0.0, 0.0, 1.0);

    // TODO: [Task 8] Sample the texture "tex" at the given UV-coordinates.
    fragColor = texture(tex, uv);
}

//#version 330 core

//in vec3 WS_position; // world-space position
//in vec3 WS_normal;   // world-space normal

//uniform vec3 color;
//out vec3 fragColor;

//void main(){
//    vec3 WS_toLight = normalize(vec3(10.0) - WS_position);
//    fragColor = color * vec3(0.3 + 0.7 * max(0.0, dot(normalize(WS_normal), WS_toLight)));
//}
