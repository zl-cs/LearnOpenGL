#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture1;

void main()
{             
    vec4 texColor = texture(texture1, TexCoords);
    if(texColor.a == 1.0)
        texColor.a = 0.0;
        discard;
    FragColor = texColor;
}