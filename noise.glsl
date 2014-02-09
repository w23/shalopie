uniform float uf_time;
uniform vec2 uv2_resolution;
float t = uf_time;
vec2 p = 2. * gl_FragCoord.xy / uv2_resolution - vec2(1.);

float hash(float v) { return fract(sin(v*.1)*35827.3218); }
float hash(vec2 v) { return hash(dot(v,vec2(71.,313.))); }
float noise(float v) {
	float F=floor(v), f=fract(v);
	f *= f * (3. - 2. * f);
	return mix(hash(F), hash(F+1.), f);
}
float noise(vec2 v) {
	vec2 F=floor(v), f=fract(v);
	f *= f * (3. - 2. * f);
	return mix(
		mix(hash(F), hash(F+vec2(1.,0.)), f.x),
		mix(hash(F+vec2(0.,1.)), hash(F+vec2(1.,1.)), f.x),
		f.y);
}
float fnoise(vec2 v) {
	return 0.
	+ .5 * noise(v)
	+ .25 * noise(v*1.97)
	+ .125 * noise(v*4.07)
	+ .0625 * noise(v*8.17);
}

float world(vec3 at) {
	return length(at) - 1.;
}

void main() {
	p.x *= uv2_resolution.x / uv2_resolution.y;
	p *= 7.;
	float a = 3.1416 * (fnoise(p) + 4.*noise(t));
	p += .6 * vec2(cos(a), sin(a));
	gl_FragColor = vec4(fnoise(p));
}
