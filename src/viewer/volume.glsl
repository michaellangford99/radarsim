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

float m = 0.467;
float n = 7.5;

//uniform vec3 volume_color;

void main() {
    vec4 unproj_pos = inv_view_projection * vec4(Position);
	unproj_pos  = unproj_pos / unproj_pos.w;
	vec3 world_pos = unproj_pos.xyz;

	vec3 ray = normalize(world_pos - camera_position);

    float radius = 0.5;

    vec3 sphere_pos = (model * vec4(0,0,0,1)).xyz;

    vec3 C = sphere_pos;
    vec3 O = camera_position;
    vec3 D = ray;

    float a = dot(D, D);
    float b = dot(2*D, (O-C));
    float c = pow(length(O-C),2) - pow(radius,2);

    float delta = b*b - 4*a*c;

    //if (delta < 0) discard;
    //else FragColor = vec4(1, 1, 1, 1);

    float t0 = (-b+sqrt(delta))/(2*a);
    float t1 = (-b-sqrt(delta))/(2*a);

    vec3 p0 = camera_position+ray*t0;
    vec3 p1 = camera_position+ray*t1;

    float dist = length(p0-p1);

    //float atten = 

    vec3 volume_color = normalize(world_pos);

    float alpha = dist*exp(m*dist)/n;

    FragColor = vec4(volume_color*alpha, alpha);

    //FragColor = vec4(WorldPosition.xyz/WorldPosition.w - world_pos, 1);

    //FragColor = vec4(world_pos.xy, 0, 1);

    //FragColor = vec4(1, 1, 1, 1);
}