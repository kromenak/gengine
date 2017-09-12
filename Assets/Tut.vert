#version 150

in vec3 vp;
void main(void) {
    gl_Position = vec4(vp, 1.0);
}
