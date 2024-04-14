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
const uint RoundBox = 5u;
const uint Octohedron = 6u;
const uint HexPrism = 7u;
const uint GridPlane = 8u;

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
    float diffuse;
    float specular;

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
const float eps = 0.01;
const float max_steps = 128;

const float shadow_eps = 0.01;
const float shadow_max_steps = 64;
const float shadow_max_dist = 50.0;

// Distance functions https://iquilezles.org/articles/distfunctions/
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

float sdRoundBox(vec3 p, vec3 b, float r)
{
    vec3 q = abs(p) - b + r;
    return length(max(q, 0.0)) + min(max(q.x, max(q.y, q.z)), 0.0) - r;
}

float sdOctahedron(vec3 p, float s)
{
    p = abs(p);
    float m = p.x+p.y+p.z-s;
    vec3 q;
    if (3.0*p.x < m) q = p.xyz;
    else if (3.0*p.y < m) q = p.yzx;
    else if (3.0*p.z < m) q = p.zxy;
    else return m*0.57735027;

    float k = clamp(0.5*(q.z-q.y+s), 0.0, s);
    return length(vec3(q.x, q.y-s+k, q.z-k));
}

float sdHexPrism(vec3 p, vec2 h)
{
    const vec3 k = vec3(-0.8660254, 0.5, 0.57735);
    p = abs(p);
    p.xy -= 2.0*min(dot(k.xy, p.xy), 0.0)*k.xy;
    vec2 d = vec2(
    length(p.xy-vec2(clamp(p.x, -k.z*h.x, k.z*h.x), h.x))*sign(p.y-h.x),
    p.z-h.y);
    return min(max(d.x, d.y), 0.0) + length(max(d, 0.0));
}

float sdPlane(vec3 p, vec3 n, float h)
{
    // n must be normalized
    return dot(p, n) + h;
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

    if (curr.type == RoundBox) {
        vec3 box_size = vec3(curr.sx, curr.sy, curr.sz);
        return sdRoundBox(obj_pos - pos, box_size, 0.1);
    }

    if (curr.type == Octohedron) {
        return sdOctahedron(obj_pos - pos, curr.sx);
    }

    if (curr.type == HexPrism) {
        return sdHexPrism(obj_pos - pos, vec2(curr.sx, curr.sy));
    }

    if (curr.type == GridPlane) {
        return sdPlane(pos, vec3(0, 1, 0), -curr.y);
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
    return vec4(a.xyz, max(-b.w, a.w));
}

vec4 intersection(vec4 a, vec4 b) {
    return vec4(a.xyz, max(a.w, b.w));
}

vec3 get_object_color(in Object obj, in vec3 pos) {
    if (obj.type == GridPlane) {
        const bool x = mod(int(pos.x), int(obj.sx * 2)) < obj.sx;
        const bool z = mod(int(pos.z), int(obj.sz * 2)) < obj.sz;

        if (x) {
            return z ? vec3(1) : vec3(0.5);
        }

        return z ? vec3(0.5) : vec3(1.0);
    }

    return vec3(obj.r, obj.g, obj.b);
}

vec4 combined_query(in vec4 curr_data, in Object other, in vec3 pos) {
    const uint link_type = other.link_type;
    float other_dist = find_distance_to_object(other, pos);
    vec4 other_data = vec4(get_object_color(other, pos), other_dist);

    if (link_type == Default) {
        if (curr_data.w < other_data.w) {
            return curr_data;
        }
        return other_data;
    }

    if (link_type == SoftUnion) {
        return smooth_min(curr_data, other_data, 10);
    }

    if (link_type == Subtraction) {
        return subtraction(curr_data, other_data);
    }

    if (link_type == Intersection) {
        return intersection(curr_data, other_data);
    }

    return curr_data;
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

vec4 query_object(in uint idx, in vec3 pos) {
    Object curr = object_buffer.objects[idx];
    const bool linked =  curr.type != Placeholder;

    vec4 curr_data = vec4(get_object_color(curr, pos), find_distance_to_object(curr, pos));

    if (linked) {
        for (uint c = 1; c <= curr.num_children; c++) {
            curr_data = combined_query(curr_data, object_buffer.objects[idx+c], pos);
        }
    }

    return curr_data;
}

void query_scene(in vec3 pos, out vec3 color, out float min_dist, out uint hit_idx) {
    color = vec3(0, 0, 0);
    min_dist = MAX;
    hit_idx = 0;

    for (uint i = 0; i < num_objects; i++) {
        Object curr = object_buffer.objects[i];
        const bool linked =  curr.type != Placeholder;
        vec4 curr_data = query_object(i, pos);

        if (curr_data.w < min_dist) {
            min_dist = curr_data.w;
            color = curr_data.rgb;
            hit_idx = i;
        }

        if (linked) {
            i += curr.num_children;
        }
    }
}

float query_scene_dist(in vec3 pos) {
    vec3 surface_color;
    float dist;
    uint hit_idx;
    query_scene(pos, surface_color, dist, hit_idx);
    return dist;
}

float query_obj_dist(in vec3 pos, in uint idx) {
    return query_object(idx, pos).w;
}

vec3 estimate_surface_normal(in vec3 p, uint obj_idx) {
    float x = query_obj_dist(vec3(p.x+eps, p.y, p.z), obj_idx) - query_obj_dist(vec3(p.x-eps, p.y, p.z), obj_idx);
    float y = query_obj_dist(vec3(p.x, p.y+eps, p.z), obj_idx) - query_obj_dist(vec3(p.x, p.y-eps, p.z), obj_idx);
    float z = query_obj_dist(vec3(p.x, p.y, p.z+eps), obj_idx) - query_obj_dist(vec3(p.x, p.y, p.z-eps), obj_idx);

    return normalize(vec3(x, y, z));
}

float compute_shadow(vec3 origin, vec3 direction, float dst_to_light) {
    const float dist_limit = min(shadow_max_dist, dst_to_light);

    int num_steps = 0;
    float total_dist = 0;

    // For calculating soft shadows
    const float soft_shadow_factor = 32;
    float result = 1.0f;

    while (total_dist < dist_limit && num_steps < shadow_max_steps) {
        vec3 surface_color;
        float dist;
        uint hit_idx;
        query_scene(origin, surface_color, dist, hit_idx);

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
        uint hit_idx;
        query_scene(origin, surface_color, dist, hit_idx);

        // Hit object
        if (dist < eps) {
            hit_obj = true;
            Object curr = object_buffer.objects[hit_idx];

            vec3 hit_point = origin + dist * direction;
            vec3 surface_normal = estimate_surface_normal(hit_point - eps * direction, hit_idx);

            // Compute shadows
            const vec3 shadow_offset_pos = hit_point + surface_normal * eps*3;
            vec3 light_dir = light_pos - shadow_offset_pos;
            const float dist_to_light = length(light_dir);
            light_dir = normalize(light_dir);
            const float shadow_value = compute_shadow(shadow_offset_pos, light_dir, dist_to_light);

            // Compute light (Blinn-Phong)
            light_dir = normalize(light_pos - hit_point);
            const vec3 view_dir = normalize(origin - hit_point);
            const vec3 halfway_dir = normalize(view_dir + light_dir);

            const float shininess = curr.specular;
            const float diffuse = curr.diffuse;

            const float specular = pow(max(dot(halfway_dir, surface_normal), 0.0), shininess);
            const float lambertian = diffuse * clamp(dot(surface_normal, light_dir), 0.0, 1.0);

            vec3 lit_color = (lambertian + specular) * surface_color * light_color * shadow_value;

            // Gamma correction todo make it configurable
            const float gamma = 2.2;
            lit_color = pow(lit_color.rgb, vec3(1.0/gamma));

            out_pixel = vec4(lit_color, 1.0);
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