#version 440 core
layout (depth_any) out float gl_FragDepth;

in GEOM_OUT
{
    vec2 tc;
    float depth;
    mat3 tbn;
    vec3 normal;
    vec4 position;
} fsIn;

out highp vec4 fColor;
uniform mat4 modelToWorld;
uniform mat4 worldToCamera;
uniform mat4 cameraToView;

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
uniform sampler2D specularMap;

uniform vec3 cameraPos;

struct Light
{
    vec3 Color;
    float AmbientIntensity;
    float DiffuseIntensity;
    vec3 Direction;
    float SpecPower;
    float MatShines;
};

uniform Light light;

vec4 lightFactorNormal()
{
    vec3 bumpMapNormal = texture(normalMap, fsIn.tc).xyz;
    bumpMapNormal = 2.0 * bumpMapNormal - vec3(1.0);
    bumpMapNormal = fsIn.tbn * bumpMapNormal;
    float factor = dot(bumpMapNormal, -light.Direction);
    if(factor > 0.0)
    {
        return vec4(light.Color, 1.0) *
        light.DiffuseIntensity *
        factor;
    }
    else
        return vec4(0.0, 0.0, 0.0, 0.0);
}

vec4 lightFactorSpecular()
{
    vec3 surfaceToCamera = normalize(cameraPos - fsIn.position.xyz);
    float specularFactor = pow(max(0.0, normalize(dot(surfaceToCamera, reflect(-light.Direction, fsIn.normal)))), light.MatShines);
    vec4 specular = vec4(specularFactor * texture(specularMap, fsIn.tc).xyz * light.SpecPower, 1.0);
    return specular;
}


void main()
{
    gl_FragDepth = fsIn.depth;
    vec4 diffuseLight = lightFactorNormal();
    vec4 specularLight = lightFactorSpecular();
    fColor = texture(diffuseMap, fsIn.tc) *
            (diffuseLight +
             light.AmbientIntensity);
    //fColor = specularLight;
}
