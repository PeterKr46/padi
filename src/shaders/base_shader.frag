uniform sampler2D texture;

// Under WTFPL: https://stackoverflow.com/a/17897228

vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main() {
    vec2 fragCoord = gl_TexCoord[0].xy;

    vec4 fragColor = texture2D(texture, fragCoord);
    vec3 colorShift = gl_Color.rgb;

    float csum = colorShift.r + colorShift.g + colorShift.b;
    float dark = (step(csum, 0.5));
    //colorShift.rgb = (1.0-step(fragColor.rrr, vec3(0.95))*(1.0-colorShift.rgb));
    // colorShift = rgb2hsv(gl_Color.rgb);
    // colorShift.r += fragColor.g;
    // colorShift = hsv2rgb(colorShift);
    //gl_FragColor.rgb = dark * (1-step(fragColor.r, 0.9)) + (1-(dark*step(fragColor.r, 0.9))) * (fragColor.rrr * colorShift.rgb);
                     //+ (step(fragColor.rrr, vec3(0.98))*(fragColor.rrr * colorShift.rgb));

    gl_FragColor.rgb = colorShift * fragColor.rrr;
    gl_FragColor.a = (1.0 - step(fragColor.a, 0.1)) * gl_Color.a;
}

