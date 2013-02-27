#version 330 core

layout(location = 0) in vec3 verPosition;
layout(location = 1) in vec2 uvCoord;

uniform mat4 PVW;
uniform int  winWidth;
uniform int  winHeight;
uniform int  imageWidth;
uniform int  imageHeight;
// Переменные используемые для анимации.
uniform float zOffset;
uniform float yOffset;

out vec2 UV;

void main(){
	vec3 posW = verPosition;
	float r = float(winWidth)/winHeight;
	float r1 = float(imageWidth)/imageHeight;
	// При изменении размеров окна изменяется и размер отображаемой области.
	posW.z *= r;

	if( (winWidth < imageWidth) || (winHeight < imageHeight) ){
		if( r1 <= r ) 
			posW.z *= r1/r;
		else
			posW.y *= r/r1;
	}
	else{
		posW.z *= imageWidth/float(winWidth);
		posW.y *= imageHeight/float(winHeight);
	}
	posW.y += yOffset;
	posW.z += zOffset;
	
	gl_Position = PVW*vec4(posW, 1.0);
	UV = uvCoord;
}
