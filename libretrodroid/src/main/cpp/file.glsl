#ifdef GL_FRAGMENT_PRECISION_HIGH
#define HIGHP highp
#else
#define HIGHP mediump
precision mediump float;
#endif
#define EPSILON 0.01

precision mediump float;
uniform lowp sampler2D texture;
uniform HIGHP vec2 textureSize;
uniform lowp sampler2D previousPass;
uniform mediump float screenDensity;
uniform mediump float vFlipY;

varying HIGHP vec2 screenCoords;
varying HIGHP vec2 coords;
varying HIGHP vec2 passCoords;
varying HIGHP vec2 c5;
varying HIGHP vec2 c6;
varying HIGHP vec2 c9;
varying HIGHP vec2 c10;
varying lowp float deltaSharpness;

lowp float fastLuma(lowp vec3 v) {
  return v.g;
}
#if USE_FAST_LUMA
lowp float luma(lowp vec3 v) {
  return fastLuma(v);
}
#else
lowp float luma(lowp vec3 v) {
  return dot(v, vec3(0.299, 0.587, 0.114));
}
#endif

lowp float linearStep(lowp float edge0, lowp float edge1, lowp float t) {
  return clamp((t - edge0) / (edge1 - edge0 + EPSILON), 0.0, 1.0);
}
lowp float sharpSmooth(lowp float t, lowp float sharpness) {
  return linearStep(sharpness, 1.0 - sharpness, t);
}
lowp float sharpness(lowp float l1, lowp float l2) {
  lowp float lumaDiff = abs(l1 - l2);
  lowp float sharpness = 0.5 * SHARPNESS_MIN + deltaSharpness * min(lumaDiff * SHARPNESS_BIAS, 1.0);
  return sharpness;
}
lowp vec4 blend(lowp vec4 a, lowp vec4 b, lowp float t) {
  return mix(a, b, sharpSmooth(t, sharpness(a.a, b.a)));
}
lowp vec3 unpack(lowp float values) {
  return vec3(floor(mod(values / 4.0, 4.0)), floor(mod(values / 16.0, 4.0)), floor(mod(values / 64.0, 4.0)));
}
lowp vec3 triangle(lowp vec2 pxCoords) {
  lowp vec3 ws = vec3(0.0);
  ws.x = pxCoords.y - pxCoords.x;
  ws.y = 1.0 - ws.x;
  ws.z = (pxCoords.y - ws.x) / (ws.y + EPSILON);
  return ws;
}
lowp vec3 quad(lowp vec2 pxCoords) {
  return vec3(pxCoords.x, pxCoords.x, pxCoords.y);
}
lowp vec4 choose(lowp vec4 reference, lowp vec4 choiceA, lowp vec4 choiceB) {
  lowp float diffa = abs(reference.a - choiceA.a);
  lowp float diffb = abs(reference.a - choiceB.a);
  return diffa < diffb ? choiceA : choiceB;
}
lowp vec3 pattern0(inout lowp mat4 cs, lowp vec2 pxCoords) {
  cs = mat4(cs[0], cs[1], cs[2], cs[3]);
  return vec3(0.0, pxCoords.x, pxCoords.y);
}
lowp vec3 pattern1(inout lowp mat4 cs, lowp vec2 pxCoords) {
  lowp vec3 result;
  if (pxCoords.y > pxCoords.x) {
    cs = mat4(cs[0], cs[2], cs[2], cs[3]);
    result = vec3(1.0, pxCoords.x, pxCoords.y);
  } else {
    cs = mat4(cs[0], cs[1], cs[1], cs[3]);
    result = vec3(1.0, pxCoords.y, pxCoords.x);
  }
  return result;
}
lowp vec3 pattern2(inout lowp mat4 cs, lowp vec2 pxCoords) {
  lowp vec3 result;
  lowp vec4 cd = choose(cs[0], cs[2], cs[3]);
  if (pxCoords.y > 2.0 * pxCoords.x) {
    cs = mat4(cs[0], cs[2], cs[2], cd);
    result = vec3(1.0, pxCoords.x * 2.0, pxCoords.y);
  } else {
    cs = mat4(cs[0], cs[1], cd, cs[3]);
    result = vec3(0.0, (pxCoords.x - 0.5 * pxCoords.y) / (1.0 - 0.5 * pxCoords.y + EPSILON), pxCoords.y);
  }
  return result;
}
lowp vec3 pattern3(inout lowp mat4 cs, lowp vec2 pxCoords) {
  lowp vec3 result;
  lowp vec4 ab = choose(cs[3], cs[0], cs[1]);
  lowp vec4 cd = choose(cs[0], cs[2], cs[3]);

  if (pxCoords.y > 2.0 * pxCoords.x) {
    cs = mat4(cs[0], cs[2], cs[2], cd);
    result = vec3(1.0, pxCoords.x * 2.0, pxCoords.y);
  } else if (pxCoords.y < 2.0 * pxCoords.x - 1.0) {
    cs = mat4(cs[3], cs[1], cs[1], ab);
    result = vec3(1.0, (1.0 - pxCoords.x) * 2.0, 1.0 - pxCoords.y);
  } else {
    cs = mat4(cs[0], ab, cd, cs[3]);
    result = vec3(0.0, 2.0 * (pxCoords.x - 0.5 * pxCoords.y), pxCoords.y);
  }
  return result;
}
lowp vec3 pattern4(inout lowp mat4 cs, lowp vec2 pxCoords) {
  lowp vec3 result;
  lowp vec4 ab = choose(cs[2], cs[0], cs[1]);
  lowp vec4 cd = choose(cs[0], cs[2], cs[3]);

  if (pxCoords.x < 0.5) {
    cs = mat4(cs[0], ab, cs[2], cd);
    result = vec3(0.0, pxCoords.x * 2.0, pxCoords.y);
  } else {
    cs = mat4(ab, cs[1], cd, cs[3]);
    result = vec3(0.0, 2.0 * (pxCoords.x - 0.5), pxCoords.y);
  }
  return result;
}
lowp vec3 pattern5(inout lowp mat4 cs, lowp vec2 pxCoords) {
  lowp vec3 result;
  if (pxCoords.y > pxCoords.x + 0.5) {
    cs = mat4(cs[3], cs[2], cs[2], cs[0]);
    result = vec3(1.0, 2.0 * pxCoords.x, 2.0 * (pxCoords.y - 0.5));
  } else if (pxCoords.y > pxCoords.x) {
    cs = mat4(cs[0], cs[3], cs[0], cs[3]);
    result = vec3(1.0, pxCoords.x, pxCoords.y);
  } else {
    cs = mat4(cs[0], cs[1], cs[1], cs[3]);
    result = vec3(1.0, pxCoords.y, pxCoords.x);
  }
  return result;
}
void main() {
  lowp vec3 t5 = texture2D(texture, c5).rgb;
  lowp vec3 t6 = texture2D(texture, c6).rgb;
  lowp vec3 t9 = texture2D(texture, c9).rgb;
  lowp vec3 t10 = texture2D(texture, c10).rgb;

  lowp mat4 colors = mat4(
    vec4(t5, luma(t5)),
    vec4(t6, luma(t6)),
    vec4(t9, luma(t9)),
    vec4(t10, luma(t10))
  );

  lowp vec2 flagsTexture = texture2D(previousPass, passCoords).rg;

  int pattern = int(flagsTexture.x * 10.0);
  lowp vec3 transform = unpack(floor(flagsTexture.y * 255.0 + 0.5));

  lowp vec2 pxCoords = fract(screenCoords);

  if (transform.x > 0.5) {
    colors = mat4(colors[1], colors[0], colors[3], colors[2]);
    pxCoords.x = 1.0 - pxCoords.x;
  }

  if (transform.y > 0.5) {
    colors = mat4(colors[2], colors[3], colors[0], colors[1]);
    pxCoords.y = 1.0 - pxCoords.y;
  }

  if (transform.z > 0.5) {
    colors = mat4(colors[0], colors[2], colors[1], colors[3]);
    pxCoords = pxCoords.yx;
  }

  lowp vec3 result;

  if (pattern == 0) {
    result = pattern0(colors, pxCoords);
  } else if (pattern == 1) {
    result = pattern1(colors, pxCoords);
  } else if (pattern == 2) {
    result = pattern2(colors, pxCoords);
  } else if (pattern == 3) {
    result = pattern3(colors, pxCoords);
  } else if (pattern == 4) {
    result = pattern4(colors, pxCoords);
  } else {
    result = pattern5(colors, pxCoords);
  }

  lowp vec3 weights = result.x > 0.5 ? triangle(result.yz) : quad(result.yz);

  lowp vec4 final = blend(
    blend(colors[0], colors[1], weights.x),
    blend(colors[2], colors[3], weights.y),
    weights.z
  );

  gl_FragColor = vec4(final.rgb, 1.0);
}