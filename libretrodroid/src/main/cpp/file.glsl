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
  return clamp((t - edge0) / (edge1 - edge0), 0.0, 1.0);
}
lowp float sharpSmooth(lowp float t, lowp float sharpness) {
  return linearStep(sharpness, 1.0 - sharpness, t);
}
lowp float sharpness(lowp float l1, lowp float l2) {
  lowp float lumaDiff = abs(l1 - l2);
  lowp float sharpness = 0.5 * SHARPNESS_MIN + deltaSharpness * min(lumaDiff * SHARPNESS_BIAS, 1.0);
  return sharpness;
}
lowp float sharpness(lowp vec3 c1, lowp vec3 c2) {
  return sharpness(fastLuma(c1), fastLuma(c2));
}
lowp vec3 blend(lowp vec3 a, lowp vec3 b, lowp float t) {
  return mix(a, b, sharpSmooth(t, sharpness(a, b)));
}
lowp vec3 unpack(lowp float values) {
  return vec3(floor(mod(values / 4.0, 4.0)), floor(mod(values / 16.0, 4.0)), floor(mod(values / 64.0, 4.0)));
}
lowp vec3 triangle(lowp vec2 pxCoords) {
  lowp vec3 ws = vec3(0.0);
  ws.x = pxCoords.y - pxCoords.x;
  ws.y = 1.0 - ws.x;
  ws.z = (pxCoords.y - ws.x) / ws.y;
  return ws;
}
lowp vec3 quad(lowp vec2 pxCoords) {
  return vec3(pxCoords.x, pxCoords.x, pxCoords.y);
}
lowp mat4 pattern0(lowp mat4 cs, lowp vec2 pxCoords) {
  lowp mat4 result = mat4(cs[0], cs[1], cs[2], cs[3]);
  result[0][3] = 0.0; result[1][3] = pxCoords.x; result[2][3] = pxCoords.y;
  return result;
}
lowp mat4 pattern1(lowp mat4 cs, lowp vec2 pxCoords) {
  lowp mat4 result;
  if (pxCoords.y > pxCoords.x) {
    result = mat4(cs[0], cs[2], cs[2], cs[3]);
    result[0][3] = 1.0; result[1][3] = pxCoords.x; result[2][3] = pxCoords.y;
  } else {
    result = mat4(cs[0], cs[1], cs[1], cs[3]);
    result[0][3] = 1.0; result[1][3] = pxCoords.y; result[2][3] = pxCoords.x;
  }
  return result;
}
lowp mat4 pattern2(lowp mat4 cs, lowp vec2 pxCoords) {
  lowp mat4 result;
  if (pxCoords.y > 2.0 * pxCoords.x) {
    result = mat4(cs[0], cs[2], cs[2], cs[0]);
    result[0][3] = 1.0; result[1][3] = pxCoords.x * 2.0; result[2][3] = pxCoords.y;
  } else {
    result = mat4(cs[0], cs[1], cs[0], cs[3]);
    result[0][3] = 0.0; result[1][3] = (pxCoords.x - 0.5 * pxCoords.y) / (1.0 - 0.5 * pxCoords.y); result[2][3] = pxCoords.y;
  }
  return result;
}
lowp mat4 pattern3(lowp mat4 cs, lowp vec2 pxCoords) {
  lowp mat4 result;
  if (pxCoords.y > 2.0 * pxCoords.x) {
    result = mat4(cs[0], cs[2], cs[2], cs[0]);
    result[0][3] = 1.0; result[1][3] = pxCoords.x * 2.0; result[2][3] = pxCoords.y;
  } else if (pxCoords.y < 2.0 * pxCoords.x - 1.0) {
    result = mat4(cs[3], cs[1], cs[1], cs[3]);
    result[0][3] = 1.0; result[1][3] = (1.0 - pxCoords.x) * 2.0; result[2][3] = 1.0 - pxCoords.y;
  } else {
    result = mat4(cs[0], cs[3], cs[0], cs[3]);
    result[0][3] = 0.0; result[1][3] = 2.0 * (pxCoords.x - 0.5 * pxCoords.y); result[2][3] = pxCoords.y;
  }
  return result;
}
lowp mat4 pattern4(lowp mat4 cs, lowp vec2 pxCoords) {
  lowp mat4 result;
  if (pxCoords.x < 0.5) {
    result = mat4(cs[0], cs[2], cs[2], cs[0]);
    result[0][3] = 0.0; result[1][3] = pxCoords.x * 2.0; result[2][3] = pxCoords.y;
  } else {
    result = mat4(cs[2], cs[1], cs[0], cs[3]);
    result[0][3] = 0.0; result[1][3] = 2.0 * (pxCoords.x - 0.5); result[2][3] = pxCoords.y;
  }
  return result;
}
lowp mat4 pattern5(lowp mat4 cs, lowp vec2 pxCoords) {
  lowp mat4 result;
  if (pxCoords.y > pxCoords.x + 0.5) {
    result = mat4(cs[3], cs[2], cs[2], cs[0]);
    result[0][3] = 1.0; result[1][3] = 2.0 * pxCoords.x; result[2][3] = 2.0 * (pxCoords.y - 0.5);
  } else if (pxCoords.y > pxCoords.x) {
    result = mat4(cs[0], cs[3], cs[0], cs[3]);
    result[0][3] = 1.0; result[1][3] = pxCoords.x; result[2][3] = pxCoords.y;
  } else {
    result = mat4(cs[0], cs[1], cs[1], cs[3]);
    result[0][3] = 1.0; result[1][3] = pxCoords.y; result[2][3] = pxCoords.x;
  }
  return result;
}
void main() {
  lowp mat4 colors = mat4(
    vec4(texture2D(texture, c5).rgb, 0.0),
    vec4(texture2D(texture, c6).rgb, 0.0),
    vec4(texture2D(texture, c9).rgb, 0.0),
    vec4(texture2D(texture, c10).rgb, 0.0)
  );

  lowp vec2 flagsTexture = texture2D(previousPass, passCoords).rg;

  int pattern = int(flagsTexture.x * 10.0);
  lowp vec3 transform = unpack(floor(flagsTexture.y * 255.0 + 0.5));

  lowp vec2 pxCoords = fract(screenCoords);

  if (transform.x > 0.0) {
    colors = mat4(colors[1], colors[0], colors[3], colors[2]);
    pxCoords.x = 1.0 - pxCoords.x;
  }

  if (transform.y > 0.0) {
    colors = mat4(colors[2], colors[3], colors[0], colors[1]);
    pxCoords.y = 1.0 - pxCoords.y;
  }

  if (transform.z > 0.0) {
    colors = mat4(colors[0], colors[2], colors[1], colors[3]);
    pxCoords = pxCoords.yx;
  }

  lowp mat4 result;

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

  lowp vec2 coords = vec2(result[1][3], result[2][3]);

  lowp vec3 weights = result[0][3] > 0.5 ? triangle(coords) : quad(coords);

  lowp vec3 final = blend(
    blend(result[0].rgb, result[1].rgb, weights.x),
    blend(result[2].rgb, result[3].rgb, weights.y),
    weights.z
  );

  gl_FragColor = vec4(final, 1.0);
}