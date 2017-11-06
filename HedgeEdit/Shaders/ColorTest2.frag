#version 330 core
out vec4 FragColor;
in vec4 pos;

void main()
{
    float x = 0.01f * pos.x * (pos.z * 120f);
    float y = 0.01f * pos.y * (pos.z * 100f);
    float z = 0.01f * pos.z * (pos.z * 80f);
    FragColor = vec4(x, y, z, 1f);
}
