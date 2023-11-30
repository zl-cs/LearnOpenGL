#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture1;

void main()
{
    // vec3 block = vec3(1.0, 1.0, 1.0);
    // FragColor = vec4(col, 0.0);
    vec4 texColor = texture(texture1, TexCoords);
    if(texColor.a == 1.0){
        texColor.a = 0.0;
        discard;
    }
    FragColor = texColor;
} 