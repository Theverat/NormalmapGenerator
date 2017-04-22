#version 440 core
layout (triangles) in;
layout (triangle_strip) out;
layout (max_vertices = 3) out;

uniform mat4 modelToWorld;
uniform mat4 worldToCamera;
uniform mat4 cameraToView;

in TSE_OUT
{
    vec2 tc;
} geomIn[];

out GEOM_OUT
{
    vec2 tc;
    float depth;
    mat3 tbn;
    vec3 normal;
    vec4 position;
} geomOut;


mat3 calculateTBN()
{
    vec3 a = (modelToWorld * gl_in[0].gl_Position).xyz;
    vec3 b = (modelToWorld * gl_in[1].gl_Position).xyz;
    vec3 c = (modelToWorld * gl_in[2].gl_Position).xyz;
    // texture coordinates difference
    vec2 uv1 = geomIn[1].tc - geomIn[0].tc;
    vec2 uv2 = geomIn[2].tc - geomIn[0].tc;
    // edges
    vec3 e1 = b - a;
    vec3 e2 = c - a;

    float r = 1 / ((uv1.x * uv2.y) - (uv1.y * uv2.x));

    // get normal
    vec3 normal = normalize(cross(e1, e2));

    // get tangent
    vec3 tangent = (e1 * uv2.y - e2 * uv1.y) * r;
    tangent = normalize(tangent);
    tangent = normalize(tangent - dot(tangent, normal) * normal);

    //get bitangent
    vec3 bitangent = cross(tangent, normal);
    if(dot(cross(normal, tangent), bitangent) < 0.0)
        tangent = tangent * (-1.0);
    mat3 tbn = mat3(tangent, bitangent, normal);
    return tbn;
}

void main(void)
{
    vec3 a = (modelToWorld * gl_in[0].gl_Position).xyz;
    vec3 b = (modelToWorld * gl_in[1].gl_Position).xyz;
    vec3 c = (modelToWorld * gl_in[2].gl_Position).xyz;
    geomOut.normal = normalize(cross(b - a, c - a));
    int i;
    for(i = 0; i<gl_in.length(); i++)
    {
        geomOut.tbn = calculateTBN();
        geomOut.position = gl_in[i].gl_Position;
        gl_Position = cameraToView * worldToCamera * modelToWorld * gl_in[i].gl_Position;
        geomOut.tc = geomIn[i].tc;
        geomOut.depth = (gl_Position.z) / 5.0;
        geomOut.normal = normalize(cross(c - a, b - a));
        EmitVertex();
    }
    EndPrimitive();
}
