#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in mat4 instanceMatrix;

out vec2 TexCoords;
out vec3 FragPos;
out vec3 Normal;

uniform mat4 view;
uniform mat4 projection;

void main()
{
    TexCoords = aTexCoords;
    FragPos = vec3(instanceMatrix * vec4(aPos, 1.0));
    
    // For a flat leaf, the normal is always "up" relative to the leaf plane
    // We transform it by the rotation part of the instance matrix
    Normal = normalize(mat3(instanceMatrix) * vec3(0.0, 0.0, 1.0));

    gl_Position = projection * view * vec4(FragPos, 1.0);
}
