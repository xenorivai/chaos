#version 430 core
out vec4 fragColor;
in vec4 gl_FragCoord;

//uniforms
uniform vec2 iResolution;
uniform float iTime;
uniform float iTimeDelta;

uniform vec3 camPos;
uniform vec3 camFront;
uniform vec3 camUp;
uniform float camFOV;

uniform int MAX_MARCHING_STEPS;
uniform float MIN_DIST;
uniform float MAX_DIST;
uniform int ITERATIONS;

uniform float sphere_fold_min_radius;
uniform float sphere_fold_fixed_radius;

struct LIGHT {
	vec3 pos;
	vec3 intensity;
};
uniform int nLights;
uniform LIGHT lights[2];

const float EPSILON = 0.0001;
#define M_PI 3.14159265359

mat4 viewMatrix(vec3 eye, vec3 center, vec3 up);

//utility functions
float smin(float a, float b, float k);
float boxSDF(vec3 p, vec3 dim);
float sphereSDF(vec3 p, float r);
float cylinderSDF(vec3 p, vec3 dim);
float torusSDF(vec3 p);
float gyroidTorusSDF(vec3 p);
float planeSDF(vec3 p);
float rep(vec3 p, vec3 modulus);

float intersectSDF(float A, float B);
float unionSDF(float A, float B, float k);
float differenceSDF(float A, float B);

vec3 rotate(vec3 p, vec3 axis, float theta);
vec3 boxFold(vec3 p, float value, float Scale);
vec4 sphereFold(vec4 p);

//ray generation stuff
vec3 rayDirection(float fieldOfView, vec2 res, vec2 fragCoord);
float rayMarch(vec3 eye, vec3 marchingDirection, float start, float end);
vec3 estimateNormal(vec3 p);

//Phong Illumination
vec3 phongContribForLight(vec3 k_d, vec3 k_s, float alpha, vec3 p, vec3 lightPos, vec3 lightIntensity);
vec3 phongIllumination(vec3 k_a, vec3 k_d, vec3 k_s, float alpha, vec3 p, LIGHT lights, int nLights);

//fractal test bed
float de(vec3 p) {
	float Scale = 2.0;
	vec4 z = vec4(p, 1.0);
	vec4 c = z;

	for (int i = 0; i < 25; i++) {
		z.xyz = boxFold(z.xyz, 1.0, Scale);
		z = sphereFold(z);

		z = Scale * z + c;
	}

	return length(z.xyz) / z.w;
}

float sceneSDF(vec3 p) {
	//float sphereDist = sphereSDF((p - vec3(5.5)) / 1.2, 0.5) * 1.2;
	//float boxDist = boxSDF(p - vec3(0, 1.5, 0), vec3(.75, 0.75, 1.0));
	//float cylinderDist = cylinderSDF(p, vec3(0.5, .6, .1));
	float planeDist = planeSDF(p);
	//float torusDist = torusSDF(p + vec3(cos(2 * iTime), 0.0, sin(2 * iTime)));

	//return unionSDF(unionSDF(boxDist, planeDist, 0.5), torusDist, 0.5);
	//return unionSDF(unionSDF(boxDist, planeDist), sphereDist);
	//return gyroidTorusSDF(p);


	float sphereDist = sphereSDF(p, .95);
	float dist = boxSDF(p, vec3(.75, .75, .75));
	dist = differenceSDF(dist, sphereDist);
	return min(dist, planeDist);

	//return de(p);

	//return rep(p, vec3(1, 0, 1));
	//return unionSDF(rep(p, vec3(0.25, 0, 0.25)), planeDist, 0.1);

	//vec3 q = vec3(4.0 * sin(iTime), 2.0, 4.0 * cos(iTime));
	//return unionSDF(unionSDF(boxDist, planeDist, 0.1), sphereSDF(p - q, 0.1), 0.9);

	//vec3 q = vec3(0, 0, 15);
	//return min(boxSDF(p - q, vec3(0.5)), boxSDF(p + q, vec3(0.5)));

}

//returns true if shadow ray from 'ro' towards(along 'rd') light hits anything
bool shadow(vec3 ro, vec3 rd, float mint, float maxt) {
	for (float t = mint; t < maxt; )
	{
		float d = sceneSDF(ro + rd * t);
		if (d < 0.001) {
			return true; //shadow ray hit something
		}
		t += d;
	}
	return false; //shadow ray did'nt hit any thing
}

void main()
{
	vec4 fragCoord = gl_FragCoord;

	vec3 viewDir = rayDirection(camFOV, iResolution, fragCoord.xy);

	mat4 viewToWorld = viewMatrix(camPos, camPos + camFront, camUp);

	vec3 worldDir = (viewToWorld * vec4(viewDir, 0.0)).xyz;

	//shoot rays per pixel, get dist to closest object in 'dist'
	float dist = rayMarch(camPos, worldDir, MIN_DIST, MAX_DIST);

	//very far, color as background
	if (dist > MAX_DIST - EPSILON) {
		float t = dist;
		fragColor = vec4(vec3(0.2, 0.2, 0.2), 1.0);
		return;
	}

	//go to the closest point
	vec3 p = camPos + dist * worldDir;

	//color the point
	vec3 K_a = vec3(0.2);
	vec3 K_d = vec3(0.1, 0.81, 1);
	vec3 K_s = vec3(1.0, 1.0, 1.0);
	float shininess = 32.0;

	//LIGHT lights[2];
	//lights[0].pos = vec3(4.0 * sin(iTime), 2.0, 4.0 * cos(iTime));
	//lights[0].intensity = vec3(0.8);
	//
	////lights[1].pos = vec3(2.0 * sin(0.37 * iTime), 2.0 * cos(0.37 * iTime), 2.0);
	//lights[1].pos = vec3(0.1 * sin(0.5 * iTime), 0.2 * (sin(0.5 * iTime) + 1) + 0.95, 0.1 * cos(0.5 * iTime));
	//lights[1].intensity = vec3(1.2, 0.2, 0.2);

	const vec3 ambientLight = 0.5 * vec3(1.0, 1.0, 1.0);
	vec3 color = ambientLight * K_a;

	//check for shadows? -- raymarch from point to lightPos
	for (int i = 0; i < nLights; i++) {
		if (shadow(p, normalize(lights[i].pos - p), 0.01, 3.0)) {
			color *= 0.1;
		}
	}
	for (int i = 0; i < nLights; i++) {
		color += phongContribForLight(K_d, K_s, shininess, p, lights[i].pos, lights[i].intensity);
	}




	fragColor = vec4(color, 1.0);
}

/* TO-FIX!! Attempt to pass array of struct of lights DOES NOT WORK

	vec3 phongIllumination(vec3 k_a, vec3 k_d, vec3 k_s, float alpha, vec3 p, LIGHT[] lights, int nLights) {

		const vec3 ambientLight = 0.5 * vec3(1.0, 1.0, 1.0);
		vec3 color = ambientLight * k_a;

		for (int i = 0; i < nLights; i++) {
			color += phongContribForLight(k_d, k_s, alpha, p, lights[i].pos, lights[i].intensity);
			if (shadow(p, normalize(lights[i].pos - p), 0.01, 3.0)) {
				color = vec3(0.15, 0.15, 0.15);
			}
		}
		return color;
	}
*/

vec3 phongContribForLight(vec3 k_d, vec3 k_s, float alpha, vec3 p, vec3 lightPos, vec3 lightIntensity) {
	//https://en.wikipedia.org/wiki/Phong_reflection_model
	vec3 N = estimateNormal(p);
	vec3 L = normalize(lightPos - p);
	vec3 V = normalize(camPos - p);
	vec3 R = normalize(reflect(-L, N));

	float dotLN = dot(L, N);
	float dotRV = dot(R, V);

	if (dotLN < 0.0) {
		// Light not visible from this point on the surface
		return vec3(0.0, 0.0, 0.0);
	}

	if (dotRV < 0.0) {
		// Light reflection in opposite direction as viewer, apply only diffuse
		// component
		return lightIntensity * (k_d * dotLN);
	}
	return lightIntensity * (k_d * dotLN + k_s * pow(dotRV, alpha));
}

float smin(float a, float b, float k) {
	k = 0.2;
	float h = clamp(0.5 + 0.5 * (b - a) / k, 0, 1);
	return mix(b, a, h) - k * h * (1 - h);
}

float boxSDF(vec3 p, vec3 dim) {
	vec3 d = abs(p) - dim;
	vec3 md = min(d, vec3(0));
	return length(max(d, vec3(0))) + max(max(md.x, md.y), md.z);
}

float sphereSDF(vec3 p, float r) {
	return length(p) - r;
}

float cylinderSDF(vec3 p, vec3 dim)
{
	return length(p.xz - dim.xy) - dim.z;
}

float torusSDF(vec3 p) {
	float smallRadius = .75; // minor radius
	float largeRadius = 1.; // major radius

	return length(vec2(length(p.xz) - largeRadius, p.y)) - smallRadius;
}

float gyroidTorusSDF(vec3 p) {
	float rt = 15.;
	float rg = 4.;
	float ws = 0.3;

	p.xz = vec2(rt * atan(p.z, -p.x), length(p.xz) - rt);
	p.yz = vec2(rg * atan(p.z, -p.y), length(p.yz) - rg);
	return .6 * max(abs(dot(sin(p), cos(p).yzx)) - ws, abs(p.z) - .5 * M_PI);
}

float planeSDF(vec3 p) {
	vec3 n = vec3(0., 1., 0.);       // plane's normal vector
	float distanceFromOrigin = 0.; // position along normal

	return dot(p, n) + distanceFromOrigin;
}

float intersectSDF(float A, float B) {
	return max(A, B);
}

float unionSDF(float A, float B, float k) {
	return smin(A, B, k);
}

float differenceSDF(float A, float B) {
	return max(A, -B);
}

mat4 viewMatrix(vec3 eye, vec3 center, vec3 up) {
	// Based on gluLookAt man page
	vec3 f = normalize(center - eye);
	vec3 s = normalize(cross(f, up));
	vec3 u = cross(s, f);
	return mat4(
		vec4(s, 0.0),
		vec4(u, 0.0),
		vec4(-f, 0.0),
		vec4(0.0, 0.0, 0.0, 1)
	);
}

float rep(vec3 p, vec3 modulus) {
	return sphereSDF(mod(p, modulus) - 0.5 * modulus, 0.3);
}

vec3 rayDirection(float fieldOfView, vec2 res, vec2 fragCoord) {
	vec2 xy = fragCoord - res / 2.0;
	float z = res.y / tan(radians(fieldOfView) / 2.0);
	return normalize(vec3(xy, -z));
}

float rayMarch(vec3 eye, vec3 marchingDirection, float start, float end) {
	float depth = start;	//starting dist infront of the camera
	for (int i = 0; i < MAX_MARCHING_STEPS; i++) {
		float dist = sceneSDF(eye + depth * marchingDirection);
		if (dist < EPSILON) {
			//surface hit!
			return depth;
		}

		depth += dist;

		if (depth >= end) {
			//very far
			return end;
		}
	}
	//out of steps
	return end;
}

vec3 estimateNormal(vec3 p) {
	//gradient of SDF
	return normalize(vec3(
		sceneSDF(vec3(p.x + EPSILON, p.y, p.z)) - sceneSDF(vec3(p.x - EPSILON, p.y, p.z)),
		sceneSDF(vec3(p.x, p.y + EPSILON, p.z)) - sceneSDF(vec3(p.x, p.y - EPSILON, p.z)),
		sceneSDF(vec3(p.x, p.y, p.z + EPSILON)) - sceneSDF(vec3(p.x, p.y, p.z - EPSILON))
	));
}

vec3 rotate(vec3 p, vec3 axis, float theta) {
	vec3 v = cross(axis, p), u = cross(v, axis);
	return u * cos(theta) + v * sin(theta) + axis * dot(p, axis);
}

vec3 boxFold(vec3 p, float value, float Scale) {
	return clamp(p, -value, value) * Scale - p;
}

vec4 sphereFold(vec4 p) {
	float LINEAR_SCALE = sphere_fold_fixed_radius / sphere_fold_min_radius;
	float pDot = dot(p.xyz, p.xyz);
	if (pDot < sphere_fold_min_radius) p *= LINEAR_SCALE;
	else if (pDot < sphere_fold_fixed_radius) p *= sphere_fold_fixed_radius / pDot;

	return p;
}