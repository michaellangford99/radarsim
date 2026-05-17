#version 450 core
//TODO: need to redownload GLAD to get 4.6
layout (location = 0) in vec3 aPos;

out vec4 Position;
out vec4 WorldPosition;//is this needed?

uniform mat4 world;
uniform mat4 view;
uniform mat4 projection;

layout(binding=0, std430) readonly buffer ssbo_point_cloud {
    vec4 points[];
};

void main()
{
    vec3 point_pos = points[gl_InstanceID].xyz;

    point_pos.x = 10*point_pos.x/point_pos.y;

	Position = projection * view * world * vec4(point_pos, 1.0);
	WorldPosition = world * vec4(point_pos, 1.0);
    gl_Position = Position;
}