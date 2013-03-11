#ifndef SHADERS_H
#define SHADERS_H

char ver_shader[] = 
"#version 330 core\n"

"layout(location = 0) in vec3 verPosition;\n"
"layout(location = 1) in vec2 uvCoord;\n"

"uniform mat4 PVW;\n"
"uniform int  winWidth;\n"
"uniform int  winHeight;\n"
"uniform int  imageWidth;\n"
"uniform int  imageHeight;\n"

"// Переменные используемые для анимации.\n"
"uniform float zOffset;\n"
"uniform float yOffset;\n"

"out vec2 UV;\n"

"void main(){\n"
	"vec3 posW = verPosition;\n"
	"float r = float(winWidth)/winHeight;\n"
	"float r1 = float(imageWidth)/imageHeight;\n"
	"// При изменении размеров окна изменяется и размер отображаемой области.\n"
	"posW.z *= r;\n"

	"if( (winWidth < imageWidth) || (winHeight < imageHeight) ){\n"
		"if( r1 <= r )\n"
			"posW.z *= r1/r;\n"
		"else\n"
			"posW.y *= r/r1;\n"
	"}\n"
	"else{\n"
		"posW.z *= imageWidth/float(winWidth);\n"
		"posW.y *= imageHeight/float(winHeight);\n"
	"}\n"
	"posW.y += yOffset;\n"
	"posW.z += zOffset;\n"

	"gl_Position = PVW*vec4(posW, 1.0);\n"
	"UV = uvCoord;\n"
"}\n";

char frag_shader[] = 
"#version 330 core\n"

"out vec3 color;\n"
"in  vec2 UV;\n"
"uniform sampler2D Image;\n"

"void main(){\n"
	"color = texture(Image, UV).rgb;\n"
"}\n";

#endif // SHADERS_H
