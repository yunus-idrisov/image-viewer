#version 330 core

out vec3 color;
in  vec2 UV;
uniform sampler2D Image;

void main(){
	color = texture(Image, UV).rgb;
}
