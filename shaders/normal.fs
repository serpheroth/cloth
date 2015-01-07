#version 410 core

uniform vec3 color;
uniform vec3 lightdir;
uniform vec3 viewdir;
out vec4 fs_color;

void main(void){
	fs_color=vec4(1.0,0.0,0.0,1.0);
}