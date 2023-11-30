#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture1;

void main()
{
    vec3 black = vec3(0.1, 0.1, 0.1);
    // FragColor = vec4(col, 0.0);
    vec4 texColor = texture(texture1, TexCoords);
    // texColor.a = 1.0;
    if(texColor.rgb == black){
        texColor.a = 0.0;
        discard;
    }
    // if(texColor.a > 0){
    //     texColor.rgb /= texColor.a;

    // }
    FragColor = texColor;
} 