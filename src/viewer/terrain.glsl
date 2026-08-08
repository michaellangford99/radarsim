#version 330 core
out vec4 FragColor;
 
in vec4 Position;
in vec4 WorldPosition;
in vec3 Normal;
in vec2 TexCoord;
in vec3 Color;

uniform sampler2D normal_map;
uniform sampler2D height_map;

uniform mat4 inv_view_projection;
uniform vec3 camera_position;

uniform vec3 terrain_color;
uniform float terrain_intensity;
uniform vec3 light_dir;
uniform vec3 sunlight_dir;
uniform vec3 sun_color;

uniform float h_slice;

uniform float a;
uniform float k;
uniform float mu;
uniform vec3 rayleigh_color;

//---------
uniform vec3 ant_pos_m;
uniform vec3 ant_vel_mps;

uniform float bump_width;
uniform vec4 range_iso_color;

float compute_range(vec3 a, vec3 b)
{
	return length(a-b);
}

vec3 compute_grad_range(vec3 a, vec3 b)
{
	float dh = 1.0;
	vec3  vhx = vec3(dh,0,0);
	vec3  vhy = vec3(0,dh,0);
	vec3  vhz = vec3(0,0,dh);

	float dx = (compute_range(a,b + vhx) - compute_range(a,b - vhx))/(2*dh);
	float dy = (compute_range(a,b + vhy) - compute_range(a,b - vhy))/(2*dh);
	float dz = (compute_range(a,b + vhz) - compute_range(a,b - vhz))/(2*dh);

	return vec3(dx, dy, dz);
}

float rationalBump( float x, float k )
{
    return 1.0/(1.0+k*x*x);
}

// vec2 compute_grad_screen_space()
// {
// 	return vec2(dFdx(world_pos), dFdy(world_pos));
// }

struct antenna
{
	vec3 pos_m;
	vec3 vel_mps;
};

// use struct definitions later
float bistatic_doppler(antenna tx_ant, antenna rx_ant, vec3 tgt_pos_m, vec3 tgt_vel_mps)//only value that changes with uv is world_pos.
{
	vec3 tx_dir_vec = normalize(tgt_pos_m - tx_ant.pos_m);
    float tx_range_rate = dot(tgt_vel_mps - tx_ant.vel_mps, tx_dir_vec); // clean up these names. here, pos. range rate means outbound. (i think thats good since range rate is der of range)

    vec3 rx_dir_vec = normalize(tgt_pos_m - rx_ant.pos_m);
    float rx_range_rate = dot(tgt_vel_mps - rx_ant.vel_mps, rx_dir_vec); // clean up these names. here, pos. range rate means outbound. (i think thats good since range rate is der of range)

	return tx_range_rate + rx_range_rate;
}

vec3 compute_grad_wrt_pos_bistatic_doppler(antenna tx_ant, antenna rx_ant, vec3 tgt_pos_m, vec3 tgt_vel_mps)
{
	float dh = 1.0;
	vec3  vhx = vec3(dh,0,0);
	vec3  vhy = vec3(0,dh,0);
	vec3  vhz = vec3(0,0,dh);

	float dx = (bistatic_doppler(tx_ant, rx_ant, tgt_pos_m + vhx, tgt_vel_mps) - bistatic_doppler(tx_ant, rx_ant, tgt_pos_m - vhx, tgt_vel_mps))/(2*dh);
	float dy = (bistatic_doppler(tx_ant, rx_ant, tgt_pos_m + vhy, tgt_vel_mps) - bistatic_doppler(tx_ant, rx_ant, tgt_pos_m - vhy, tgt_vel_mps))/(2*dh);
	float dz = (bistatic_doppler(tx_ant, rx_ant, tgt_pos_m + vhz, tgt_vel_mps) - bistatic_doppler(tx_ant, rx_ant, tgt_pos_m - vhz, tgt_vel_mps))/(2*dh);

	return vec3(dx, dy, dz);
}

void main()
{
	vec3 norm = texture(normal_map, TexCoord).xyz*2 - 1;
	float height = texture(height_map, TexCoord).x*65535.0;

	vec3 light_dir_norm = normalize(light_dir);
	float light = 0.25 + 0.75*dot(light_dir_norm, norm);//modified lambertian

	float sunlight = (dot(normalize(sunlight_dir), norm));//modified lambertian
	if (sunlight < 0)
		sunlight = 0;

	// vec2 pos = TexCoord;
	// int step_count = 100;
	// for (int i = 0; i < step_count; i++)
	// {
	// 	vec3 march_dir = mat3(1, 0, 0,
	// 					      0, 1, 0,
	// 						  0, 0, 0) * light_dir_norm;

	// 	pos += light_dir_norm.xy*0.0002;
	// 	float s_height = texture(height_map, pos).x*65535.0;
	// 	if (s_height > height+35)
	// 		light = light*0.989;
	// }


	//float light_avg = 0.25 + 0.75*dot(light_dir_norm, vec3(0,0,1));

	vec3 terrain_color = terrain_color*light + sun_color*sunlight;

	vec4 unproj_pos = inv_view_projection * vec4(Position);
	unproj_pos  = unproj_pos / unproj_pos.w;
	vec3 world_pos = unproj_pos.xyz;
	float dist = length(camera_position-world_pos);

	//FragColor =  vec4(dist/100, 0, 0, 1.0);

	float add_color = 0.0;

	float radius = 0;
	int steps = 100;
	float intensity = 0.0;
	for (int i = 0; i < steps; i++)
	{
		radius += dist/steps;

		float volume = (dist/steps);
		intensity += 0.001*(0.5+a)*exp(-mu*radius)*volume * 1 / pow(radius, 2*k);
	}

	terrain_color += rayleigh_color*intensity;

	vec3 line_color = vec3(0,0,0.4);
	float spacing = 1000.0;
	vec2 xy_mod = mod(TexCoord*3601.0, spacing) - spacing/2.0;

	float k=5.0;
	float line_intensity = 0;//max(exp(-k*(xy_mod.x*xy_mod.x)),exp(-k*(xy_mod.y*xy_mod.y)));

	FragColor = vec4(terrain_color + line_color*line_intensity, 1.0);

	// add isorange lines
	//----------------------------------------------------------------------------
	float range = length(ant_pos_m - world_pos);

	vec3 grad_range = compute_grad_range(ant_pos_m, world_pos);

	vec3 grad_ss_u = dFdx(world_pos);
	vec3 grad_ss_v = dFdy(world_pos);

	vec2 grad_ss_range = vec2(dot(grad_range, grad_ss_u), dot(grad_range, grad_ss_v));

	// closed form gradient available for doppler and for range.

	float eval = (mod(range, 10.0)-0.5) / length(grad_ss_range);

	// if (eval < 0.7)
	// 	FragColor += vec4(0.4, 0.4, 0.4, 0);

	FragColor += range_iso_color * rationalBump(eval, bump_width) / exp(10*length(grad_ss_range));
	FragColor += vec4(0,0.1,0,0) * length(grad_ss_range);

	//uniform vec3 ant_pos_m;
	//uniform vec3 ant_vel_mps;


	//--------------------------------------

	antenna ant = antenna(vec3(10,10,3), vec3(10, 10, 0));

	float doppler_vel_mps = bistatic_doppler(ant, ant, world_pos, vec3(0,0,0));
	vec3  grad_doppler_vel_mps = compute_grad_wrt_pos_bistatic_doppler(ant, ant, world_pos, vec3(0,0,0));

	vec2 grad_ss_doppler_vel = vec2(dot(grad_doppler_vel_mps, grad_ss_u), dot(grad_doppler_vel_mps, grad_ss_v));

	float eval_doppler_vel = (mod(doppler_vel_mps, 1.0)-0.5) / length(grad_ss_doppler_vel);

	FragColor += vec4(0,0.7,0,0) * rationalBump(eval_doppler_vel, bump_width) / exp(10*length(grad_ss_doppler_vel));
	//FragColor += vec4(0,0.1,0,0) * length(grad_ss_range);

	//FragColor += vec4(1.0, 0.0, 0.88, 0.0) * doppler_vel_mps*0.01;

	//--------------------------------------

	if (height > h_slice+5000)
		FragColor *= 1+(height-(h_slice+5000))/200;
}