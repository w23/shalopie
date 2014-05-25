uniform float uf_time;
uniform vec2 uv2_resolution;
uniform sampler2D us2_noise;
//uniform sampler2D us2_heightmap;
float t = uf_time*.2;

vec4 noise4(vec2 p) {
	vec2 F = floor(p), f = fract(p);
	f *= f * (3. - 2. * f);
	return texture2D(us2_noise, (F+f+.5)/256., -100.);
}
float noise(vec2 p) { return noise4(p).x; }
float noise(float p) { return noise4(vec2(p,.5)).y; }

float fnoise(vec2 p) {
	return noise(p)*.5+noise(p*2.)*.25+noise(p*4.+4.*noise(3.*t))*.125+noise(p*8.)*.0625+noise(p*16.)*.03125;
}

float gnd(vec2 p) {
	p.x += t*2.;
	float k =fnoise(p*.7);
	return -.03+2.*k*k*k + noise(p * 170.)*.01;;
}

float world(vec3 p) {
	float d = .3*(p.y - max(.08 + noise((p.xz+vec2(t*2.,0.)) * 170.)*.002,gnd(p.xz)));
	return d;
}

vec4 albedo(vec3 p) {
	p.x += t*2.;
	vec4 g1 = vec4(.3,.9,.4,1e7);
	vec4 g2 = vec4(1.,1.,1.,1e7);//.6,.7,.8);
	vec4 w = vec4(.3, .5, .9, 2e2);
	vec4 color =  mix(g1, g2, clamp(0.,1.,.5*p.y+pow(1.5*noise(p.xz*4.),4.)));
	if (p.y <= .11)
		color = w;
	color = color + .2*(noise4(p.xz*38.).yzwx-vec4(.5,.5,.5,0.));
	return color;
}

vec3 normal(vec3 p) {
	vec2 e = vec2(1e-2, .0);
	return normalize(vec3(
		world(p + e.xyy) - world(p - e.xyy),
		world(p + e.yxy) - world(p - e.yxy),
		world(p + e.yyx) - world(p - e.yyx)
		));
}

float trace(vec3 o, vec3 d, float lmin, float lmax) {
	float l = lmin;
	float s;
	vec3 p;
	for (int i = 0; i < 128; ++i) {
		p = o + d * l;
		s = world(p);
		l += s;
		if (s < .001 * l || l > lmax) break;
	}
	if (s < .002) {
		for (int i = 0; i < 8; ++i) {
		}
	}
	return l;
}

const float R0 = 6360e3;
const float Ra = 6380e3;
const int steps = 32;
const int stepss = 8;
const float g = .76;
const float g2 = g * g;
const float Hr = 8e3;
const float Hm = 1.2e3;
const float I = 5.;
vec3 C = vec3(0., -R0, 0.);
vec3 bM = vec3(21e-6);
vec3 bR = vec3(5.8e-6, 13.5e-6, 33.1e-6);
vec3 Ds = normalize(vec3(sin(t), .5+.51*sin(.8*t), cos(t)-1.));

void densities(in vec3 pos, out float rayleigh, out float mie) {
	float h = length(pos - C) - R0;
	rayleigh =  exp(-h/Hr);
	mie = exp(-h/Hm);
}

float escape(in vec3 p, in vec3 d, in float R) {
	vec3 v = p - C;
	float b = dot(v, d);
	float c = dot(v, v) - R*R;
	float det2 = b * b - c;
	if (det2 < 0.) return -1.;
	float det = sqrt(det2);
	float t1 = -b - det, t2 = -b + det;
	return (t1 >= 0.) ? t1 : t2;
}

// this can be explained: http://www.scratchapixel.com/lessons/3d-advanced-lessons/simulating-the-colors-of-the-sky/atmospheric-scattering/
vec3 scatter(vec3 o, vec3 d) {
	float L = escape(o, d, Ra);	
	float mu = dot(d, Ds);
	float opmu2 = 1. + mu*mu;
	float phaseR = .0596831 * opmu2;
	float phaseM = .1193662 * (1. - g2) * opmu2 / ((2. + g2) * pow(1. + g2 - 2.*g*mu, 1.5));
	
	float depthR = 0., depthM = 0.;
	vec3 R = vec3(0.), M = vec3(0.);
	
	float dl = L / float(steps);
	for (int i = 0; i < steps; ++i) {
		float l = float(i) * dl;
		vec3 p = o + d * l;

		float dR, dM;
		densities(p, dR, dM);
		dR *= dl; dM *= dl;
		depthR += dR;
		depthM += dM;

		float Ls = escape(p, Ds, Ra);
		if (Ls > 0.) {
			float dls = Ls / float(stepss);
			float depthRs = 0., depthMs = 0.;
			for (int j = 0; j < stepss; ++j) {
				float ls = float(j) * dls;
				vec3 ps = p + Ds * ls;
				float dRs, dMs;
				densities(ps, dRs, dMs);
				depthRs += dRs * dls;
				depthMs += dMs * dls;
			}
			
			vec3 A = exp(-(bR * (depthRs + depthR) + bM * (depthMs + depthM)));
			R += A * dR;
			M += A * dM;
		} else {
			return vec3(0.);
		}
	}
	
	return I * (R * bR * phaseR + M * bM * phaseM);
}

vec3 illuminate_dir(vec4 alb, vec3 p, vec3 n, vec3 inc, vec3 ldir, vec3 lcolor, float lmax) {
	float env = max(0., dot(n,ldir));
	vec3 h2 = normalize(ldir - inc);
	float shadow = smoothstep(.9*lmax, lmax, trace(p, ldir, .01, lmax));
	float k = (alb.w + 2.) * (alb.w + 4.) / (24. * (pow(2., -alb.w / 2.) + alb.w));
	return alb.xyz * lcolor * env * shadow + lcolor * pow(dot(n,h2),alb.w) * k;
}

vec3 illuminate_point(vec4 alb, vec3 p, vec3 n, vec3 inc, vec3 lpos, vec3 lcolor) {
	vec3 ldir = lpos - p;
	float d2 = dot(ldir, ldir);
	float d = sqrt(d2);
	return illuminate_dir(alb, p, n, inc, ldir / d, lcolor, d) / d2;
}

/*
float occlusion(vec3 p, vec3 d) {
	for (int i = 0; i < 8; ++i) {

	}
}*/

void main() {
	vec2 p = gl_FragCoord.xy / uv2_resolution * 2. - 1.;
	p.x *= uv2_resolution.x / uv2_resolution.y;

	vec3 eye = vec3(0., 1.5, 2.);
	//eye.y += gnd(eye.xz);
	vec3 dir = normalize(vec3(p, -1.));

	float lmax = 20.;
	float l = trace(eye, dir, 0., lmax);
	vec3 pos = eye + dir * l;
	vec3 nor = normal(pos);

	vec3 lightdir = normalize(vec3(1.));

	vec3 color = vec3(0.);
	vec3 amb = vec3(0.);

	for (int i = 0; i < 4; ++i) {
		amb += scatter(pos,
			normalize(nor+vec3(2.,1.,2.)*(noise4(floor(14.*(pos.xz+t))).wyz - vec3(.5, 0.,.5))));
	}

	vec3 cscatter = scatter(eye, dir);

	if (l < lmax && world(pos) < .2*l) {
		vec4 albedo = albedo(pos);
		color += albedo.xyz * amb * .15 +
			/*illuminate_point(albedo, pos, nor, dir,
				vec3(cos(t),3.,sin(t)),
				.3*vec3(.3,.4,.1)) +
			illuminate_point(pos, nor, dir,
				vec3(sin(t*3.),3.+2.*sin(t*.3),cos(t)),
				vec3(1.)) +
			illuminate_point(pos, nor, dir,
				vec3(sin(t*.8),3.,cos(t*2.)),
				vec3(.4,.9,.2)));*/
			.75*illuminate_dir(albedo, pos, nor, dir, Ds, scatter(eye, Ds), 100.);

		color = mix(color, cscatter, min(1.,1.5*l/lmax));
	} else {
		color = cscatter;
	}

	gl_FragColor = vec4(pow(color, vec3(.45)), 1.);
}