// Source nonseparable f32 blend translation from tiny-skia 0.12 highp.rs:506-671.
// Copyright 2018 Google Inc.
// Copyright 2020 Yevhenii Reizner
// BSD-3-Clause notice: ../../../LICENSE-tiny-skia-blend (from this file's directory).
// SOURCE_BLEND_KERNEL is fixed at runtime compilation: 0 Hue, 1 Saturation,
// 2 Color, 3 Luminosity. No mutable effect properties are read by this shader.
Texture2D<float4> backgroundTexture : register(t0);
SamplerState backgroundSampler : register(s0);
Texture2D<float4> foregroundTexture : register(t1);
SamplerState foregroundSampler : register(s1);

float sat3(float3 c)
{
    return max(c.r, max(c.g, c.b)) - min(c.r, min(c.g, c.b));
}

float lum3(float3 c)
{
    return c.r * 0.30f + c.g * 0.59f + c.b * 0.11f;
}

void set_sat(inout float3 c, float s)
{
    float mn = min(c.r, min(c.g, c.b));
    float mx = max(c.r, max(c.g, c.b));
    float span = mx - mn;
    if (span == 0.0f)
    {
        c.r = 0.0f;
        c.g = 0.0f;
        c.b = 0.0f;
    }
    else
    {
        c.r = (c.r - mn) * s / span;
        c.g = (c.g - mn) * s / span;
        c.b = (c.b - mn) * s / span;
    }
}

void set_lum(inout float3 c, float target)
{
    float diff = target - lum3(c);
    c.r = c.r + diff;
    c.g = c.g + diff;
    c.b = c.b + diff;
}

float clip_one(float c, float mn, float mx, float l, float a)
{
    // Source mask.blend(t, f) chooses t on true. Capture mx before changes.
    if (!(mx >= 0.0f))
        c = l + (c - l) * l / (l - mn);
    if (mx > a)
        c = l + (c - l) * (a - l) / (mx - l);
    return max(c, 0.0f);
}

void clip_color(inout float3 c, float a)
{
    float mn = min(c.r, min(c.g, c.b));
    float mx = max(c.r, max(c.g, c.b));
    float l = lum3(c);
    c.r = clip_one(c.r, mn, mx, l, a);
    c.g = clip_one(c.g, mn, mx, l, a);
    c.b = clip_one(c.b, mn, mx, l, a);
}

float4 main(float4 position : SV_POSITION, float4 scenePosition : SCENE_POSITION,
    float4 backgroundCoord : TEXCOORD0,
    float4 foregroundCoord : TEXCOORD1) : SV_TARGET
{
    float4 d = backgroundTexture.Sample(backgroundSampler, backgroundCoord.xy);
    float4 s = foregroundTexture.Sample(foregroundSampler, foregroundCoord.xy);
    float3 adjusted;
#if SOURCE_BLEND_KERNEL == 0
    adjusted = s.rgb * s.a;
    set_sat(adjusted, sat3(d.rgb) * s.a);
    set_lum(adjusted, lum3(d.rgb) * s.a);
#elif SOURCE_BLEND_KERNEL == 1
    adjusted = d.rgb * s.a;
    set_sat(adjusted, sat3(s.rgb) * d.a);
    set_lum(adjusted, lum3(d.rgb) * s.a);
#elif SOURCE_BLEND_KERNEL == 2
    adjusted = s.rgb * d.a;
    set_lum(adjusted, lum3(d.rgb) * s.a);
#elif SOURCE_BLEND_KERNEL == 3
    adjusted = d.rgb * s.a;
    set_lum(adjusted, lum3(s.rgb) * d.a);
#else
#error Invalid immutable SOURCE_BLEND_KERNEL
#endif
    clip_color(adjusted, s.a * d.a);
    float3 color = s.rgb * (1.0f - d.a) + d.rgb * (1.0f - s.a) + adjusted;
    float alpha = s.a + d.a - s.a * d.a;
    return float4(color, alpha);
}
