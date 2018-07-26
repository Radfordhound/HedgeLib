#version 330 core
out vec4 FragColor;
  
in vec4 Color;
in vec2 TexCoord;

uniform sampler2D ourTexture;

void main()
{
	vec4 tex = texture(ourTexture, TexCoord);
	if(tex.w < 0.1f)
	{
		discard;
	}
    FragColor = tex * Color;
}