#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D Texture1;

void main()
{
    vec4 col = texture(screenTexture, TexCoords).rgba;
    FragColor = col;
} 