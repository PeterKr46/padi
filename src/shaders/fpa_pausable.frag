uniform sampler2D   texture;
uniform float       time;
uniform bool        paused;


// BEGIN NOISE via https://gist.github.com/patriciogonzalezvivo/670c22f3966e662d2f83
float rand(float n) {
    return fract(sin(n) * 43758.5453123);
}

float rand(vec2 n) {
    return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453);
}
float noise(float p){
    float fl = floor(p);
    float fc = fract(p);
    return mix(rand(fl), rand(fl + 1.0), fc);
}

float noise(vec2 n) {
    const vec2 d = vec2(0.0, 1.0);
    vec2 b = floor(n), f = smoothstep(vec2(0.0), vec2(1.0), fract(n));
    return mix(mix(rand(b), rand(b + d.yx), f.x), mix(rand(b + d.xy), rand(b + d.yy), f.x), f.y);
}
// END NOISE

//
// PUBLIC DOMAIN CRT STYLED SCAN-LINE SHADER
//
//   by Timothy Lottes
//
// This is more along the style of a really good CGA arcade monitor.
// With RGB inputs instead of NTSC.
// The shadow mask example has the mask rotated 90 degrees for less chromatic aberration.
//
// Left it unoptimized to show the theory behind the algorithm.
//
// It is an example what I personally would want as a display option for pixel art games.
// Please take and use, change, or whatever.
//

#define res (vec2(453, 255))
#define halfpx (vec2(0.5) / res)
#define curvature (vec2(4.8, 3.5))

// Hardness of scanline.
//  -8.0 = soft
// -16.0 = medium
float hardScan=-6.0;

// Hardness of pixels in scanline.
// -2.0 = soft
// -4.0 = hard
float hardPix=-4.0;

// Amount of shadow mask.
float maskDark=0.5;
float maskLight=1.5;

//------------------------------------------------------------------------

// Nearest emulated sample given floating point position and texel offset.
// Also zero's off screen.
vec3 Fetch(vec2 pos, vec2 off){
    pos= halfpx + floor(pos*res+off)/res;
    return texture2D(texture, pos.xy, -16.0).rgb * step(pos.x,1.0) * (1.0 - step(pos.x,0.0)) * step(pos.y,1.0) * (1.0 - step(pos.y,0.0));
}

// Distance in emulated pixels to nearest texel.
vec2 Dist(vec2 pos){ pos=pos*res;return -((pos-floor(pos))-vec2(0.5)); }

// 1D Gaussian.
float Gaus(float pos, float scale){ return exp2(scale*pos*pos); }

// 3-tap Gaussian filter along horz line.
vec3 Horz3(vec2 pos, float off){
    vec3 b=Fetch(pos, vec2(-1.0, off));
    vec3 c=Fetch(pos, vec2(0.0, off));
    vec3 d=Fetch(pos, vec2(1.0, off));
    float dst=Dist(pos).x;
    // Convert distance to weight.
    float scale=hardPix;
    float wb=Gaus(dst-1.0, scale);
    float wc=Gaus(dst+0.0, scale);
    float wd=Gaus(dst+1.0, scale);
    // Return filtered sample.
    return (b*wb+c*wc+d*wd)/(wb+wc+wd);
}

// 5-tap Gaussian filter along horz line.
vec3 Horz5(vec2 pos, float off){
    vec3 a=Fetch(pos, vec2(-2.0, off));
    vec3 b=Fetch(pos, vec2(-1.0, off));
    vec3 c=Fetch(pos, vec2(0.0, off));
    vec3 d=Fetch(pos, vec2(1.0, off));
    vec3 e=Fetch(pos, vec2(2.0, off));
    float dst=Dist(pos).x;
    // Convert distance to weight.
    float scale=hardPix;
    float wa=Gaus(dst-2.0, scale);
    float wb=Gaus(dst-1.0, scale);
    float wc=Gaus(dst+0.0, scale);
    float wd=Gaus(dst+1.0, scale);
    float we=Gaus(dst+2.0, scale);
    // Return filtered sample.
    return (a*wa+b*wb+c*wc+d*wd+e*we)/(wa+wb+wc+wd+we);
}

// Return scanline weight.
float Scan(vec2 pos, float off){
    float dst=Dist(pos).y;
    return Gaus(dst+off, hardScan);
}

// Allow nearest three lines to effect pixel.
vec3 Tri(vec2 pos){
    vec3 a=Horz3(pos, -1.0);
    vec3 b=Horz5(pos, 0.0);
    vec3 c=Horz3(pos, 1.0);
    float wa=Scan(pos, -1.0);
    float wb=Scan(pos, 0.0);
    float wc=Scan(pos, 1.0);
    return a*wa+b*wb+c*wc;
}


vec2 curveRemapUV(vec2 uv)
{
    // as we near the edge of our screen apply greater distortion using a cubic function
    uv = uv * vec2(2.0) - vec2(1.0);
    vec2 offset = abs(uv.yx) / curvature;
    uv = uv + uv * offset * offset;
    uv = uv * 0.5 + 0.5;
    return uv;
}

// Entry.
void main(){
    //out vec4 fragColor, in vec2 fragCoord
    vec2 fragCoord = gl_TexCoord[0].xy;
    vec3 multiplicativeNoise = vec3(1.0);
    fragCoord = curveRemapUV(fragCoord);
    if(paused) {
        fragCoord.x += noise(floor(fragCoord.y * 128.0) + floor(time * 14.0) * 32.0) * (0.25 / 455.0) - (0.125 / 455.0);
        multiplicativeNoise.rgb = vec3(1.0) + vec3(noise(fragCoord.y) * 0.5 - 0.25) * step(noise(floor(fragCoord.y * 128.0) + noise(time * 24.0) * 24.0), 0.9);
    }
    vec4 fragColor = texture2D(texture, fragCoord);

    fragColor.rgb = Tri(fragCoord) * multiplicativeNoise;
    if(paused) {
        //fragColor.r = fragColor.g = fragColor.b = max(fragColor.r, max(fragColor.b, fragColor.g));
    }

    // Bloom a little?
    // vec3 add = Horz5(fragCoord, 0.0);
    // fragColor.rgb += ((add.r + add.g + add.b) / 6.0) - 0.0675;

    fragColor.a=1.0;

    gl_FragColor = fragColor;
}

