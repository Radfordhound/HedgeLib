#version 330 core
out vec4 FragColor;
in vec4 pos;

void main()
{
    float x = 0.01f * pos.x * (pos.z * 12f);
    float y = 0.01f * pos.y * (pos.z * 10f);
    float z = 0.01f * pos.z * (pos.z * 8f);
    FragColor = vec4(x, y, z, 1f);
}
