#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture1;

void main()
{
    vec3 black = vec3(0.0, 0.0, 0.0);
    vec4 texColor = texture(texture1, TexCoords);
    if(texColor.rgb == black){
        texColor.a = 0.0;
        discard;
    }
    FragColor = texColor;
} 