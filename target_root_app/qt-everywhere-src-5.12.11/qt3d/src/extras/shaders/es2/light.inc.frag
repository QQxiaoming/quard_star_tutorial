#define FP highp

const int MAX_LIGHTS = 8;
const int TYPE_POINT = 0;
const int TYPE_DIRECTIONAL = 1;
const int TYPE_SPOT = 2;
struct Light {
    int type;
    FP vec3 position;
    FP vec3 color;
    FP float intensity;
    FP vec3 direction;
    FP float constantAttenuation;
    FP float linearAttenuation;
    FP float quadraticAttenuation;
    FP float cutOffAngle;
};
uniform Light lights[MAX_LIGHTS];
uniform int lightCount;
