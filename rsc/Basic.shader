#shader vertex
#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;

out vec2 TexCoord;

uniform mat4 transform;

void main() {
	gl_Position = transform * vec4(position.x, position.y, position.z, 1.0);
	TexCoord = texCoord;
};


#shader fragment
#version 330 core

//layout (location = 0) in vec3 position;
//layout (location = 1) in vec2 texCoord;

uniform float carColor;
uniform sampler2D carTexture;
uniform sampler2D bgTexture;

out vec4 color;
in vec2 TexCoord;

void main() {
	vec4 simpleMix;
	if (carColor == 3) {
		simpleMix = texture(bgTexture, TexCoord);
	}
	else {
		simpleMix = texture(carTexture, TexCoord);
		if (carColor > 1.0f) {
			simpleMix = simpleMix * vec4(0.0f, 0.0f, (carColor - 2.0f), 1.0f);
		}
		else {
			simpleMix = simpleMix * vec4((1.0f - carColor), (carColor), 0.0f, 1.0f);
		}
	}
	if (simpleMix.a < 0.1f) {
		discard;
	}
	color = simpleMix;
};