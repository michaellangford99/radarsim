#version 330 core
out vec4 FragColor;
 
in vec4 Position;
in vec4 WorldPosition;
in vec3 Normal;
in vec2 TexCoord;
in vec3 Color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 inv_view_projection;
uniform vec3 camera_position;

float m = -75.0;
uniform float n;

uniform float radius;
uniform sampler2D beam_map;

#define PI 3.141592

uniform vec3 volume_color;

void main() {
    vec4 unproj_pos = inv_view_projection * vec4(Position);
	unproj_pos  = unproj_pos / unproj_pos.w;
	vec3 world_pos = unproj_pos.xyz;

	vec3 ray = normalize(world_pos - camera_position);

    //float radius = 0.5;

    vec3 sphere_pos = (model * vec4(0,0,0,1)).xyz;

    vec3 C = sphere_pos;
    vec3 O = camera_position;
    vec3 D = ray;

    float a = dot(D, D);
    float b = dot(2*D, (O-C));
    float c = pow(length(O-C),2) - pow(radius,2);

    float delta = b*b - 4*a*c;

    float t0 = (-b+sqrt(delta))/(2*a);
    float t1 = (-b-sqrt(delta))/(2*a);

    vec3 p0 = camera_position+ray*t0;
    vec3 p1 = camera_position+ray*t1;

    //soo we have our initial integration points
    //255 means goes to the radius
    float dist = 0;

    int steps = 200;
    float dx = length(p0-p1)/steps;

    vec3 p = p1;

    for (int i = 0; i < steps; i++)
    {
        vec3 xyz = p-sphere_pos;
        float rho = length(xyz);
        float theta = atan(xyz.y, xyz.x);
        float phi = acos(xyz.z/rho);
        vec2 theta_phi_sampler_coords = vec2((theta+PI/2.0)/PI, phi/PI);

        // this is what we'd want to replace
        float beam_intensity = texture(beam_map, theta_phi_sampler_coords).x;

        if (xyz.x > 0)
        {
            if (rho < beam_intensity*radius)
                dist += dx*beam_intensity;
        }
        p += dx*ray;
    }

    float alpha = 1.0 - exp(m*dist/radius);
    FragColor = vec4(volume_color*alpha, alpha);
}