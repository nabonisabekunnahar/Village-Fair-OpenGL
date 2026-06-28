#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D leafTexture;
uniform vec3 lightDirection;
uniform vec3 lightAmbient;
uniform vec3 lightDiffuse;
uniform vec3 viewPos;
uniform bool ambientOn;
uniform bool diffuseOn;
uniform bool dirLightOn;

void main()
{
    vec4 texColor = texture(leafTexture, TexCoords);
    
    // Fallback if texture is fully transparent or missing (debugging)
    // We assume a healthy texture has some alpha. 
    // If it's pure transparent, we might be seeing a failed load.
    if(texColor.a < 0.05)
        discard;

    // Ambient
    vec3 ambient = (ambientOn ? lightAmbient : vec3(0.0)) * texColor.rgb;
 
    // Diffuse (Simple double-sided)
    vec3 diffuse = vec3(0.0);
    if (dirLightOn) {
        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(-lightDirection);
        float diff = abs(dot(norm, lightDir)); // Simple abs for double-sided
        if (diffuseOn) diffuse = lightDiffuse * diff * texColor.rgb;
    }
 
    // Output with alpha from texture
    FragColor = vec4(ambient + diffuse, texColor.a);
}
