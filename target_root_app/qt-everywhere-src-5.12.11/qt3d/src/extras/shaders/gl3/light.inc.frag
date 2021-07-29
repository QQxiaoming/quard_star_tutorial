const int MAX_LIGHTS = 8;
const int TYPE_POINT = 0;
const int TYPE_DIRECTIONAL = 1;
const int TYPE_SPOT = 2;
struct Light {
    int type;
    vec3 position;
    vec3 color;
    float intensity;
    vec3 direction;
    float constantAttenuation;
    float linearAttenuation;
    float quadraticAttenuation;
    float cutOffAngle;
};
uniform Light lights[MAX_LIGHTS];
uniform int lightCount;

// Pre-convolved environment maps
struct EnvironmentLight {
    samplerCube irradiance; // For diffuse contribution
    samplerCube specular; // For specular contribution
};
uniform EnvironmentLight envLight;
uniform int envLightCount = 0;
