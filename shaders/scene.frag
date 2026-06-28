#version 330 core
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
out vec4 FragColor;
uniform vec3 objectColor;
uniform float shininess;
uniform vec3 lightDirection;
uniform vec3 lightAmbient;
uniform vec3 lightDiffuse;
uniform vec3 lightSpecular;
uniform vec3 viewPos;
uniform sampler2D texture_diffuse;
uniform bool useTexture;
uniform vec3 emission; 
uniform float specularStrength; // Control intensity of highlights
uniform bool ambientOn;
uniform bool diffuseOn;
uniform bool specularOn;
uniform bool dirLightOn;
uniform bool spotLightOn;
uniform bool isGround;

struct PointLight {
    vec3 position;
    vec3 color;
    float constant;
    float linear;
    float quadratic;
    bool isEnabled;
};

#define MAX_POINT_LIGHTS 48
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform float pointLightIntensity;

void main() {
    vec4 tex = texture(texture_diffuse, TexCoords);
    vec3 color = useTexture ? tex.rgb * objectColor : objectColor;
    float alpha = useTexture ? tex.a : 1.0;
    
    if (useTexture && length(tex.rgb) < 0.05) {
        color = objectColor; 
        alpha = 1.0;
    }
    
    if(alpha < 0.1) discard;
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    // 1. Ambient
    vec3 ambient = (ambientOn ? lightAmbient : vec3(0.0)) * color;

    // 2. Directional (if enabled)
    vec3 diffuse = vec3(0.0);
    vec3 specular = vec3(0.0);
    if (dirLightOn) {
        vec3 lightDir = normalize(-lightDirection);
        float diff = max(dot(norm, lightDir), 0.0);
        if (diffuseOn) diffuse += lightDiffuse * diff * color;

        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(norm, halfwayDir), 0.0), shininess);
        if (specularOn) specular += lightSpecular * spec * specularStrength;
    }

    // 3. Simple Spot Light (Camera Mounted)
    if (spotLightOn) {
        vec3 spotDir = normalize(FragPos - viewPos);
        float theta = dot(spotDir, -viewDir);
        if (theta > 0.97) {
            diffuse += (diffuseOn ? vec3(0.22) : vec3(0.0)) * color;
        }
    }

    vec3 result = ambient + diffuse + specular + emission;

    for(int i = 0; i < MAX_POINT_LIGHTS; i++) {
        if(pointLights[i].isEnabled) {
            vec3 pLightDir = normalize(pointLights[i].position - FragPos);
            float pDiff = max(dot(norm, pLightDir), 0.0);
            vec3 pHalfwayDir = normalize(pLightDir + viewDir);
            float pSpec = pow(max(dot(norm, pHalfwayDir), 0.0), shininess);
            float distance = length(pointLights[i].position - FragPos);
            float attenuation = 1.0 / (pointLights[i].constant + pointLights[i].linear * distance + pointLights[i].quadratic * (distance * distance));
            
            if (diffuseOn) result += (pDiff * pointLights[i].color * color) * attenuation * pointLightIntensity;
            if (specularOn) result += (pSpec * pointLights[i].color * specularStrength) * attenuation * pointLightIntensity;
        }
    }
    
    // Smoothly fade out the ground into the skybox at a distance
    // This removes the harsh geometric line and seamlessly merges 3D grass with 2D panorama grass
    if (isGround) {
        float dist = length(FragPos.xz);
        float fadeStart = 60.0;
        float fadeEnd = 200.0;
        if (dist > fadeStart) {
            float fadeFactor = clamp((dist - fadeStart) / (fadeEnd - fadeStart), 0.0, 1.0);
            fadeFactor = smoothstep(0.0, 1.0, fadeFactor); // cubic smoothing
            alpha = mix(alpha, 0.0, fadeFactor);
        }
    }
    
    FragColor = vec4(result, alpha);
}