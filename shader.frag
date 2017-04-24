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

uniform bool applyingDiffuse;
uniform bool applyingSpecular;
uniform bool applyingNormal;

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


vec3 normal()
{
    vec3 normal;
    if(applyingNormal)
    {
        normal = texture(normalMap, fsIn.tc).xyz;
        normal = 2.0 * normal - vec3(1.0);
        normal = fsIn.tbn * normal;
    }
    else
        normal = -fsIn.normal;
    return normal;
}

vec4 lightFactorNormal(vec3 normal)
{
    float factor = dot(normal, light.Direction);
    if(factor > 0.0)
    {
        return vec4(light.Color, 1.0) *
        light.DiffuseIntensity *
        factor;
    }
    else
        return vec4(0.0, 0.0, 0.0, 0.0);
}

vec4 lightFactorSpecular(vec3 normal)
{
    if(applyingSpecular)
    {
        vec3 lightPosition = 100.0 * (-light.Direction);
        vec3 incidenceVector = normalize(fsIn.position.xyz - lightPosition);
        vec3 reflectionVector = reflect(incidenceVector, normal);
        vec3 surfaceToCamera = normalize(cameraPos - fsIn.position.xyz);
        float cosAngle = max(0.0, dot(surfaceToCamera, reflectionVector));
        float specularCoefficient = pow(cosAngle, light.MatShines);
        vec4 specularComponent = specularCoefficient *
                                 texture(specularMap, fsIn.tc) *
                                 light.SpecPower;
        return specularComponent;
    }
    else
        return vec4(0.0, 0.0, 0.0, 0.0);
}


void main()
{
    gl_FragDepth = fsIn.depth;

    vec3 normal = normal();
    vec4 diffuseLight = lightFactorNormal(normal);
    vec4 specularLight = lightFactorSpecular(normal);
    vec4 textureColor;
    if(applyingDiffuse)
        textureColor = texture(diffuseMap, fsIn.tc);
    else
        textureColor = vec4(0.7, 0.7, 0.7, 1.0);
    fColor = textureColor *
            (diffuseLight +
             specularLight +
             light.AmbientIntensity);
}
