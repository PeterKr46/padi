uniform sampler2D texture;

vec4 scanLineIntensity(float uv, float resolution, float opacity)
{
    float intensity = sin(uv * resolution * 3.141 * 2.0);
    intensity = ((0.5 * intensity) + 0.5) * 0.9 + 0.1;
    return vec4(vec3(pow(intensity, opacity)), 1.0);
}

vec2 curvature = vec2(3.5, 3.5);

vec2 curveRemapUV(vec2 uv)
{
    // as we near the edge of our screen apply greater distortion using a cubic function
    uv = uv * vec2(2.0) - vec2(1.0);
    vec2 offset = abs(uv.yx) / vec2(curvature.x, curvature.y);
    uv = uv + uv * offset * offset;
    uv = uv * 0.5 + 0.5;
    return uv;
}

void main()
{
    // lookup the pixel in the texture
    vec2 uv = gl_TexCoord[0].xy;//curveRemapUV(gl_TexCoord[0].xy);
    vec4 pixel = vec4(0);
    if (uv.x > 0 && uv.x < 1 && uv.y > 0 && uv.y < 1) {
        pixel = texture2D(texture, uv);
        //pixel *= scanLineIntensity(uv.x, 512, 0.5);
        //pixel *= scanLineIntensity(uv.y, 288, 0.5);// multiply it by the color
    }
    gl_FragColor = gl_Color * pixel;
}