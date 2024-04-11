#version 460
layout(local_size_x = 32, local_size_y = 32) in;

// Output image	
layout(rgba32f, binding = 0) uniform image2D img_output;

// Object types
const uint Placeholder = 0u;
const uint Sphere = 1u;
const uint Box  = 2u;
const uint Torus  = 3u;
const uint InfiniteSpheres = 4u;

// Link types
const uint Default = 0u;
const uint SoftUnion = 1u;
const uint Subtraction = 2u;
const uint Intersection = 3u;

// Buffer of objects
struct Object {
    uint type;
    float x, y, z;
    float sx, sy, sz;
    float r, g, b;

    uint link_type;
    uint num_children;
};

layout(std430, binding = 1) buffer ObjectBuffer
{
    Object[] objects;
} object_buffer;


// Uniforms
uniform mat4x4 view;
uniform mat4x4 inv_proj;

uniform uint image_width;
uniform uint image_height;
uniform uint num_objects;

uniform vec3 sky_bottom_color;
uniform vec3 sky_top_color;
uniform float fog_dist;
uniform float shadow_intensity;

uniform bool visualize_distances;

// temp directional light
uniform vec3 light_direction;
uniform vec3 light_pos;
uniform vec3 light_color;


// Constants
// todo make these configurable
const float MAX = 1234567890123456789024.0f;
const float max_dist = 100.0;
const float eps = 0.05;
const float max_steps = 128;

const float shadow_eps = 0.1;
const float shadow_max_steps = 64;
const float shadow_max_dist = 50.0;

// Distance functions
float sdSphere(vec3 p, float s)
{
    return length(p)-s;
}

float sdBox(vec3 p, vec3 b)
{
    vec3 q = abs(p) - b;
    return length(max(q, 0.0)) + min(max(q.x, max(q.y, q.z)), 0.0);
}

float sdTorus(vec3 p, vec2 t)
{
    vec2 q = vec2(length(p.xz)-t.x, p.y);
    return length(q)-t.y;
}

float sdInfiniteSpheres(vec3 p, vec3 s)
{
    vec3 q = p - s*round(p/s);
    return sdSphere(q, 1);
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
vec4 smooth_min(vec4 a, vec4 b, float k)
{
    float h =  max(k-abs(a.w-b.w), 0.0)/k;
    float m = h*h*0.5;
    float s = m*k*(1.0/2.0);

    vec2 mix = (a.w<b.w) ? vec2(a.w-s, m) : vec2(b.w-s, 1.0-m);

    return (1-mix.y) * a + mix.y * b;
}

vec4 subtraction(vec4 a, vec4 b) {
    return vec4(a.xyz, max(-a.w, b.w));
}

vec4 intersection(vec4 a, vec4 b) {
    return vec4(a.xyz, max(a.w, b.w));
}

vec3 get_ray_origin() {
    return (view * vec4(0, 0, 0, 1.0)).xyz;
}

vec3 get_ray_direction(in vec2 uv) {
    vec3 dir = (inv_proj * vec4(uv, 0, 1.0)).xyz;
    dir = (view * vec4(dir, 0)).xyz;
    dir = normalize(dir);
    return dir;
}

void query_scene(in vec3 pos, out vec3 color, out float min_dist) {
    color = vec3(0, 0, 0);
    min_dist = MAX;

    for (int i = 0; i < num_objects; i++) {
        Object curr = object_buffer.objects[i];

        float curr_dist = find_distance_to_object(curr, pos);
        if (curr_dist < min_dist) {
            min_dist = curr_dist;
            color = vec3(curr.r, curr.g, curr.b);
        }
    }
}

float query_scene_dist(in vec3 pos) {
    vec3 surface_color;
    float dist;
    query_scene(pos, surface_color, dist);
    return dist;
}

vec3 estimate_surface_normal(in vec3 p) {
    float x = query_scene_dist(vec3(p.x+eps, p.y, p.z)) - query_scene_dist(vec3(p.x-eps, p.y, p.z));
    float y = query_scene_dist(vec3(p.x, p.y+eps, p.z)) - query_scene_dist(vec3(p.x, p.y-eps, p.z));
    float z = query_scene_dist(vec3(p.x, p.y, p.z+eps)) - query_scene_dist(vec3(p.x, p.y, p.z-eps));

    return normalize(vec3(x, y, z));
}

float compute_shadow(vec3 origin, vec3 direction, float dst_to_light) {
    int num_steps = 0;
    float total_dist = 0;

    // For calculating soft shadows
    const float soft_shadow_factor = 8;
    float result = 1.0f;

    while (total_dist < shadow_max_dist && num_steps < shadow_max_steps) {
        vec3 surface_color;
        float dist;
        query_scene(origin, surface_color, dist);

        if (dist < shadow_eps) {
            return shadow_intensity;
        }

        result = min(result, shadow_intensity + soft_shadow_factor * dist / total_dist);

        origin = origin + direction * dist;
        total_dist += dist;
        num_steps++;
    }

    return result;
}


void main() {
    const ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
    vec4 out_pixel = vec4(0.0, 0.0, 0.0, 1.0);

    // Get current UV coordinates
    const vec2 uv = vec2(gl_GlobalInvocationID.xy) / vec2(image_width, image_height) * 2 - 1;

    // Get current ray
    vec3 origin = get_ray_origin();
    const vec3 direction = get_ray_direction(uv);

    // Raymarching
    int num_steps = 0;
    float total_dist = 0;
    bool hit_obj = false;

    while (total_dist < max_dist && num_steps < max_steps) {
        vec3 surface_color;
        float dist;
        query_scene(origin, surface_color, dist);

        // Hit object
        if (dist < eps) {
            hit_obj = true;

            vec3 hit_point = origin + dist * direction;
            vec3 surface_normal = estimate_surface_normal(hit_point - eps * direction);
            vec3 light_dir = normalize(light_pos - hit_point);

            // Compute lighting
            float NdotL = min(1.0, max(0.0, dot(surface_normal, light_dir)));
            vec3 half_lambert = pow(NdotL * 0.5 + 0.5, 2.0) * surface_color;
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
    float fog_value = clamp((hit_obj ? total_dist : MAX)/fog_dist, 0.0, 1.0);
    vec3 fog_out_color = mix(sky_bottom_color, sky_top_color, clamp(direction.y, 0.0, 1.0));

    out_pixel = vec4(mix(out_pixel.xyz, fog_out_color, fog_value), 1);

    if (visualize_distances) {
        float val = 1 - 5 * float(num_steps) / max_steps;
        out_pixel = vec4(val, val, val, 1.0f);
    }

    imageStore(img_output, pixel_coords, out_pixel);
}