// w23
uniform float uf_time;
uniform vec2 uv2_resolution;
float t = uf_time*.4;
vec2 p = (2. * gl_FragCoord.xy / uv2_resolution - vec2(1.)) * vec2(uv2_resolution.x / uv2_resolution.y, 1.);
//float t = iGlobalTime;

struct ray_t {
	vec3 o, d;
};

struct mat_t {
	vec3 diff;
	float spec;
	float refl;
};

struct xs_t {
	vec3 pos, nor, inc;
	mat_t m;
};

struct light_t {
	vec3 col, pos;
};

float hash(float v) { return fract(sin(v) * 26357.74232); }
float hash(vec3 v) { return hash(dot(v, vec3(17., 171., 313.))); }
float hash(vec2 v) { return hash(dot(v, vec2(17., 171.))); }
float noise(float v) {
	float f = fract(v), F = floor(v);
	f *= f * (3. - 2. * f);
	return mix(hash(F), hash(F+1.), f);
}
float noise(vec2 v) {
	vec2 f = fract(v), F = floor(v);
	f = f * f * (3. - 2. * f);
	return mix(
		mix(hash(F), hash(F+vec2(1.,0.)), f.x),
		mix(hash(F+vec2(0.,1.)), hash(F+vec2(1.,1.)), f.x), f.y);
}

float noise(vec3 v) {
	vec3 f = fract(v), F = floor(v);
	f = f * f * (3. - 2. * f);
	return mix(
		mix(
		mix(hash(F), hash(F+vec3(1.,0.,0.)), f.x),
		mix(hash(F+vec3(0.,1.,0.)), hash(F+vec3(1.,1.,0.)), f.x), f.y),
		mix(
		mix(hash(F+vec3(0.,0.,1.)), hash(F+vec3(1.,0.,1.)), f.x),
		mix(hash(F+vec3(0.,1.,1.)), hash(F+vec3(1.,1.,1.)), f.x), f.y), f.z);
}

float fnoise(vec2 v) {
	return .5 * noise(v) + .25 * noise(v*2.) + .125 * noise(v*4.01) + .0625 * noise(v*7.92);
}

float fnoise(vec3 v) {
	return .5 * noise(v) + .25 * noise(v*2.) + .125 * noise(v*4.01) + .0625 * noise(v*7.92);
}

ray_t  lookat(vec3 o, vec3 a, vec3 d) {
	vec3 f = normalize(a - o), r = cross(f, vec3(0.,1.,0.)), u = cross(r, f);
	return ray_t(o, mat3(r, u, -f) * d);
}

mat3 rY(float a) {
	float c=cos(a), s=sin(a);
	return mat3(c,0.,-s,0.,1.,0.,s,0.,c);
}

/*float noise(vec3 v) {
	vec3 f = fract(v), F = floor(v);
	return mix(
		mix(
			mix(hash(F), hash(F+vec3(1.,0.,0.), f.x),
			mix(hash(F+vec3(), hash(F+vec3(1.,0.,0.), f.x),));
}*/

float fft(float p) { return 0.; } // texture2D(gFFTTexture, vec2(p,0.)).b; }
float pad(vec2 p) { return 0.; } //texture2D(gPadTexture, vec2(p)/4.).r; }

float vmax(vec3 v) { return max(v.x, max(v.y, v.z)); }

float s_sphere(vec3 at) {
	return length(at-vec3(0.,2.,0.)) - 1.3;
}

float s_ball2(vec3 at) {
	return length(at-vec3(3.,4.,0.)+vec3(3.)*pad(vec2(0.))) - 2.;
}

float s_floor(vec3 at) {
	return at.y + 2. + .2*sin(3.*(pad(vec2(1.))*4. + noise(t)*4.+fnoise(at.xz*.2)));//(1. + pad(vec2(1.)));//texture2D(gFFTTexture, vec2(length(at.xz),0.));
}

float s_spiral(vec3 at) {
	at = at * rY(at.y*.26 - t);
	return .5*min(length(at.xz-vec2(8.,0.))-1.,length(at.xz-vec2(-8.,0.))-1.);
}

float s_xplane(vec3 at) {
	return at.x + 28.;
}

float world(vec3 at) {
	return /* win/angle fix by iq: fnoise(at)*.8*noise(t*9.) + fft(0.) + */
			min(s_ball2(at),min(s_xplane(at),
			min(s_spiral(at),
				min(s_floor(at),s_sphere(at)))));
}

vec3 wnormal(vec3 at) {
	vec2 e = vec2(.001, 0.);
	float W = world(at);
	return normalize(-vec3(W-world(at+e.xyy), W-world(at+e.yxy), W-world(at+e.yyx)));
}

mat_t wmaterial(vec3 at) {
	mat_t m = mat_t(vec3(1.), 10000., 1.);
	float c = s_floor(at), d;
	d=s_sphere(at);if(d<c){c=d; m = mat_t(vec3(1.,0.,0.), 100., 1.0);}
	d=s_ball2(at);if(d<c){c=d; m = mat_t(vec3(.2,.6,.9), 1000., 1.0);}
	d=s_spiral(at);if(d<c){c=d; m = mat_t(vec3(.9,.8,.3), 1000., 0.);}
	return m;
}

xs_t xs_mk(ray_t r, float p) {
	vec3 pos = r.o + r.d * p;
	return xs_t(pos, wnormal(pos), r.d, wmaterial(pos));
}

float trace(ray_t r, float minl) {
	float L = minl;
	for (int i = 0; i < 64; ++i) {
		vec3 p = r.o + r.d * L;
		float d = world(p);
		L += d;
		if (d < .01) break; 
	}
	return L;
}

vec3 enlight(xs_t x, light_t l) {
	vec3 ldir = l.pos - x.pos;
	float ldist = length(ldir);
	float sh = 1.;
	for(int i = 1; i < 16; ++i) sh = min(world(x.pos + ldir * float(i)/16.), sh);
	sh = smoothstep(0., 1., sh * 4.);
	if (sh < 0.) return vec3(0.);
	vec3 col = max(0.,dot(x.nor,ldir)) * x.m.diff * l.col;
	if (x.m.spec > 0.) {
		vec3 h = normalize(ldir - x.inc);
		col += l.col * (x.m.spec + 8.) * pow(max(0.,dot(x.nor,h)), x.m.spec) / 25.;
	}
	return col * sh / dot(ldir,ldir);
}

void main(void)
{
	//float aspect = iResolution.x / iResolution.y;
	//vec2 p = (gl_FragCoord.xy / iResolution.xy - .5) * vec2(aspect, 1.); 

	//float low = 0.;
	//for (int i = 0; i < 32; ++i) low += fft(float(i) / 255.);

	vec3 eye_pos = vec3(
		(noise(t*1.3) - .5) * 36.,
		2. + 12.*noise(t+184.),
		(noise(t*.7+1275.) - .5) * 36.);
	
	//ray_t r = lookat(vec3(0., 8., 22.)*rY(noise(t*3.)*2./* + low*/), vec3(0.), normalize(vec3(p, -2.)));
	ray_t r = lookat(eye_pos, vec3(0.), normalize(vec3(p, -2.)));

	vec3 color = vec3(0.);
	float refl = 1.;
	for (int i = 0; i < 3; ++i) {
		float path = trace(r, 1.);
		xs_t x = xs_mk(r, path);
		if (path > 100.) break;
		vec3 c = vec3(0.);
		c += enlight(x, light_t(vec3(3.), vec3(4.)));
		c += enlight(x, light_t(vec3(3.,4.,2.), vec3(-4.,4.,-6.)));
		c += enlight(x, light_t(vec3(1.,3.,4.), vec3(-4.,4.,6.)));

		color += c * refl;
		r.o = x.pos;
		r.d = normalize(reflect(x.inc,x.nor));
		refl = x.m.refl;
		if (refl <= 0.) break;
	}

	gl_FragColor = vec4(pow(color,vec3(1.2)), 1.);
}