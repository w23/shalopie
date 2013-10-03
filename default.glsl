uniform float uf_time;
uniform vec2 uv2_resolution;
float t = uf_time;
vec2 p = 2. * gl_FragCoord.xy / uv2_resolution - vec2(1.);

void main() {
	p.x *= uv2_resolution.x / uv2_resolution.y;
	gl_FragColor = vec4(p, 0., 0.);
}