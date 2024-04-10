#version 460
layout(local_size_x = 32, local_size_y = 32) in;

// Output image	
layout(rgba32f, binding = 0) uniform image2D img_output;

// Buffer of objects

const uint Sphere = 0x00000000u;
const uint Box  = 0x00000001u;
const uint Torus  = 0x00000002u;
const uint InfiniteSpheres = 0x00000003u;

struct Object {
	int type;
	float x, y, z;
	float sx, sy, sz;
	float r, g, b;
};

layout(std430, binding = 1) buffer ObjectBuffer
{
    Object[] objects;
} object_buffer;

const uint SoftUnion = 0x00000000u;
const uint Subtraction  = 0x00000001u;
const uint Intersection  = 0x00000002u;

// Uniforms
uniform mat4x4 view;
uniform mat4x4 inv_proj;

uniform uint image_width;
uniform uint image_height;
uniform uint num_objects;

uniform vec3 fog_color;

uniform float fog_dist;
uniform float shadow_intensity;

uniform bool visualize_distances;

// temp directional light
uniform vec3 light_direction;
uniform vec3 light_pos;
uniform vec3 light_color;


// Constants
const float MAX = 1234567890123456789024.0f;
const float max_dist = 100.0;
const float eps = 0.05;
const float max_steps = 128;

const float shadow_eps = 0.1;
const float shadow_max_steps = 64;
const float shadow_max_dist = 50.0;

// Distance functions
float sdSphere( vec3 p, float s )
{
  return length(p)-s;
}

float sdBox( vec3 p, vec3 b )
{
  vec3 q = abs(p) - b;
  return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0);
}

float sdTorus( vec3 p, vec2 t )
{
  vec2 q = vec2(length(p.xz)-t.x,p.y);
  return length(q)-t.y;
}

float sdInfiniteSpheres(vec3 p, vec3 s)
{
    vec3 q = p - s*round(p/s);
    return sdSphere( q , 1);
}

// Select distance function based on object type

float find_distance_to_object(Object curr, vec3 pos) { 
	vec3 obj_pos = vec3(curr.x, curr.y, curr.z);

	if (curr.type == Sphere) {
		return sdSphere(obj_pos - pos, curr.sx);
	}

	if (curr.type == Box) {
		vec3 box_size = vec3(curr.sx, curr.sy, curr.sz);
		return sdBox(obj_pos - pos, box_size);
	}

	if (curr.type == Torus) {
		vec2 scale = vec2(curr.sx, curr.sy);
		return sdTorus(obj_pos - pos, scale);
	}

	if (curr.type == InfiniteSpheres) {
		vec3 scale = vec3(curr.sx, curr.sy, curr.sz);
		return sdInfiniteSpheres(obj_pos - pos, scale);
	}

	return MAX;
}

// combination functions
vec4 smooth_min( vec4 a, vec4 b, float k )
{
	float h =  max( k-abs(a.w-b.w), 0.0 )/k;
    float m = h*h*0.5;
    float s = m*k*(1.0/2.0);

	vec2 mix = (a.w<b.w) ? vec2(a.w-s,m) : vec2(b.w-s,1.0-m);

	return (1-mix.y) * a + mix.y * b;
}

vec4 subtraction(vec4 a, vec4 b) {
	return vec4(a.xyz, max(-a.w, b.w));
}

vec4 intersection(vec4 a, vec4 b) {
	return vec4(a.xyz, max(a.w, b.w));
}

// returns color in xyz, and distance in w
vec4 find_combined_values(Object o1, Object o2, int link_type, vec3 pos) {
	float o1_dist = find_distance_to_object(o1, pos);
	float o2_dist = find_distance_to_object(o2, pos);

	vec4 o1_data = vec4(o1.r, o1.g, o1.b, o1_dist);
	vec4 o2_data = vec4(o2.r, o2.g, o2.b, o2_dist);

	if (link_type == SoftUnion) {
		return smooth_min(o1_data, o2_data, 10);
	}

	if (link_type == Subtraction) {
		return subtraction(o1_data, o2_data);
	}

	if (link_type == Intersection) {
		return intersection(o1_data, o2_data);
	}

	
	// default
	if (o1_dist < o2_dist) {
		return o1_data;
	}
	else {
		return o2_data;
	}
}

vec3 get_ray_origin() {
	return (view * vec4(0,0,0,1.0)).xyz;
}

vec3 get_ray_direction(vec2 uv) {
	vec3 dir = (inv_proj * vec4(uv, 0, 1.0)).xyz;
	dir = (view * vec4(dir, 0)).xyz;
	dir = normalize(dir);
	return dir;
}

vec4 query_scene(vec3 pos) {
    vec3 color = vec3(0,0,0);
    float min_dist = MAX;

	for (int i = 0; i < num_objects; i++) {
		Object curr = object_buffer.objects[i];
		float curr_dist = find_distance_to_object(curr, pos);

		if (curr_dist < min_dist) {
			min_dist = curr_dist;
			color = vec3(curr.r, curr.g, curr.b);
		}
	}

	return vec4(color, min_dist);
}

vec3 estimate_surface_normal(vec3 p) {
	float x = query_scene(vec3(p.x+eps, p.y, p.z)).w - query_scene(vec3(p.x-eps, p.y, p.z)).w;
	float y = query_scene(vec3(p.x, p.y+eps, p.z)).w - query_scene(vec3(p.x, p.y-eps, p.z)).w;
	float z = query_scene(vec3(p.x, p.y, p.z+eps)).w - query_scene(vec3(p.x, p.y, p.z-eps)).w;

	return normalize(vec3(x,y,z));
}

float compute_shadow(vec3 origin, vec3 direction, float dst_to_light) {
	int num_steps = 0;
	float total_dist = 0;

	while (total_dist < shadow_max_dist && num_steps < shadow_max_steps) {
		vec4 result = query_scene(origin);
	
		vec3 surface_color = result.xyz;
		float dist = result.w;

		if (dist < shadow_eps) {
			return shadow_intensity;
		}

		origin = origin + direction * dist;
		total_dist += dist;
		num_steps++;
	}

	return 1;
}


void main() {
  
  ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
  vec4 out_pixel = vec4( 0.0, 0.0, 0.0, 1.0);

  // Get current UV coordinates
  vec2 uv = vec2(gl_GlobalInvocationID.xy) / vec2(image_width, image_height) * 2 - 1;

  // Get current ray 
  vec3 origin = get_ray_origin();
  vec3 direction = get_ray_direction(uv);
  
  // Raymarching
  int num_steps = 0;
  float total_dist = 0;
  bool hit_obj = false;

  while (total_dist < max_dist && num_steps < max_steps) {
	vec4 result = query_scene(origin);
	
	vec3 surface_color = result.xyz;
	float dist = result.w;

	// Hit object
	if (dist < eps) {
		hit_obj = true;

		vec3 hit_point = origin + dist * direction;
		vec3 surface_normal = estimate_surface_normal(hit_point - eps * direction);
		vec3 light_dir = normalize(light_pos - hit_point);

		// Compute lighting
		float NdotL = min(1.0, max(0.0, dot(surface_normal, light_dir)));
		vec3 half_lambert = pow(NdotL * 0.5 + 0.5,2.0) * surface_color;
		vec3 lit_color = half_lambert * 1.5 * light_color;

		bool use_shadows = true;
		float shadow_value = 1;

		if (use_shadows) {
			// Compute shadows
			vec3 shadow_offset_pos = hit_point + surface_normal * eps*3;
			vec3 dir_to_light = light_pos - shadow_offset_pos;
			float dist_to_light = length(dir_to_light);
			dir_to_light = normalize(dir_to_light);
			shadow_value = compute_shadow(shadow_offset_pos, dir_to_light, dist_to_light);
		}

		
		vec3 final_color = lit_color * shadow_value;
		out_pixel = vec4(final_color, 1.0);
		break;
	}

	origin = origin + direction * dist;
	total_dist += dist;
	num_steps++;
  }

  // Apply fog to output pixel
  float fog_value = max(0.0, min(1.0, (hit_obj ? total_dist : MAX)/fog_dist));
  float fog_color_scale = max(0.0, min(1.0, (direction.y)));
  vec3 fog_out_color = fog_color_scale * vec3(1,1,1) + (1-fog_color_scale) * fog_color;

  out_pixel = vec4((fog_value * fog_out_color) + (1-fog_value) * out_pixel.xyz, 1);

  if (visualize_distances) { 
	float val = 1 - 5 * float(num_steps) / max_steps;
	out_pixel = vec4(val, val, val, 1.0f);
  }

  imageStore(img_output, pixel_coords, out_pixel);
}