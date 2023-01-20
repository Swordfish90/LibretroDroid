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
  ws.y = pxCoords.x + (1.0 - pxCoords.y);
  ws.z = (pxCoords.y - ws.x) / (1.0 - ws.x);
  return ws;
}
lowp vec3 quad(lowp vec2 pxCoords) {
  return vec3(pxCoords.x, pxCoords.x, pxCoords.y);
}
void swap(inout lowp vec3 a, inout lowp vec3 b) {
  lowp vec3 tmp = a;
  a = b;
  b = tmp;
}
lowp vec3 pattern0(lowp vec3 a, lowp vec3 b, lowp vec3 c, lowp vec3 d, out vec3[4] cs, lowp vec2 pxCoords) {
  cs[0] = a; cs[1] = b; cs[2] = c; cs[3] = d;
  return quad(pxCoords);
}
lowp vec3 pattern1(lowp vec3 a, lowp vec3 b, lowp vec3 c, lowp vec3 d, out vec3[4] cs, lowp vec2 pxCoords) {
  lowp vec3 result = vec3(0.0);
  if (pxCoords.y > pxCoords.x) {
    cs[0] = a; cs[1] = c; cs[2] = c; cs[3] = d;
    result = triangle(pxCoords);
  } else {
    cs[0] = a; cs[1] = b; cs[2] = b; cs[3] = d;
    result = triangle(pxCoords.yx);
  }
  return result;
}
lowp vec3 pattern2(lowp vec3 a, lowp vec3 b, lowp vec3 c, lowp vec3 d, out vec3[4] cs, lowp vec2 pxCoords) {
  lowp vec3 result = vec3(0.0);
  if (pxCoords.y > 2.0 * pxCoords.x) {
    cs[0] = a; cs[1] = c; cs[2] = c; cs[3] = a;
    result = triangle(vec2(pxCoords.x * 2.0, pxCoords.y));
  } else {
    cs[0] = a; cs[1] = b; cs[2] = a; cs[3] = d;
    result = quad(vec2((pxCoords.x - 0.5 * pxCoords.y) / (1.0 - 0.5 * pxCoords.y), pxCoords.y));
  }
  return result;
}
lowp vec3 pattern3(lowp vec3 a, lowp vec3 b, lowp vec3 c, lowp vec3 d, out vec3[4] cs, lowp vec2 pxCoords) {
  lowp vec3 result = vec3(0.0);
  if (pxCoords.y > 2.0 * pxCoords.x) {
    cs[0] = a; cs[1] = c; cs[2] = c; cs[3] = a;
    result = triangle(vec2(pxCoords.x * 2.0, pxCoords.y));
  } else if (pxCoords.y < 2.0 * pxCoords.x - 1.0) {
    cs[0] = d; cs[1] = b; cs[2] = b; cs[3] = d;
    result = triangle(vec2((1.0 - pxCoords.x) * 2.0, 1.0 - pxCoords.y));
  } else {
    cs[0] = a; cs[1] = d; cs[2] = a; cs[3] = d;
    result = quad(vec2(2.0, 1.0) * (pxCoords - vec2(0.5 * pxCoords.y, 0.0)));
  }
  return result;
}
lowp vec3 pattern4(lowp vec3 a, lowp vec3 b, lowp vec3 c, lowp vec3 d, out vec3[4] cs, lowp vec2 pxCoords) {
  lowp vec3 result = vec3(0.0);
  if (pxCoords.x < 0.5) {
    cs[0] = a; cs[1] = c; cs[2] = c; cs[3] = a;
    result = quad(vec2(2.0 * pxCoords.x, pxCoords.y));
  } else {
    cs[0] = c; cs[1] = b; cs[2] = a; cs[3] = d;
    result = quad(vec2(2.0 * (pxCoords.x - 0.5), pxCoords.y));
  }
  return result;
}
lowp vec3 pattern5(lowp vec3 a, lowp vec3 b, lowp vec3 c, lowp vec3 d, out vec3[4] cs, lowp vec2 pxCoords) {
  lowp vec3 result = vec3(0.0);
  if (pxCoords.y > pxCoords.x + 0.5) {
    cs[0] = d; cs[1] = c; cs[2] = c; cs[3] = a;
    result = triangle(vec2(2.0 * pxCoords.x, 2.0 * (pxCoords.y - 0.5)));
  } else if (pxCoords.y > pxCoords.x) {
    cs[0] = a; cs[1] = d; cs[2] = a; cs[3] = d;
    result = triangle(pxCoords);
  } else {
    cs[0] = a; cs[1] = b; cs[2] = b; cs[3] = d;
    result = triangle(pxCoords.yx);
  }
  return result;
}
void main() {
  lowp vec3 a = texture2D(texture, c5).rgb;
  lowp vec3 b = texture2D(texture, c6).rgb;
  lowp vec3 c = texture2D(texture, c9).rgb;
  lowp vec3 d = texture2D(texture, c10).rgb;

  lowp vec2 flagsTexture = texture2D(previousPass, passCoords).rg;

  int pattern = int(flagsTexture.x * 10.0);
  lowp vec3 transform = unpack(floor(flagsTexture.y * 255.0 + 0.5));

  lowp vec2 pxCoords = fract(screenCoords);

  if (transform.x > 0.0) {
    swap(a, b); swap(c, d);
    pxCoords.x = 1.0 - pxCoords.x;
  }

  if (transform.y > 0.0) {
    swap(a, c); swap(b, d);
    pxCoords.y = 1.0 - pxCoords.y;
  }

  if (transform.z > 0.0) {
    swap(b, c);
    pxCoords = pxCoords.yx;
  }

  lowp vec3 ws = vec3(0.0);
  lowp vec3 cs[4];

  if (pattern == 5) {
    ws = pattern5(a, b, c, d, cs, pxCoords);
  } else if (pattern == 4) {
    ws = pattern4(a, b, c, d, cs, pxCoords);
  } else if (pattern == 3) {
    ws = pattern3(a, b, c, d, cs, pxCoords);
  } else if (pattern == 2) {
    ws = pattern2(a, b, c, d, cs, pxCoords);
  } else if (pattern == 1) {
    ws = pattern1(a, b, c, d, cs, pxCoords);
  } else {
    ws = pattern0(a, b, c, d, cs, pxCoords);
  }

  lowp vec3 final = blend(blend(cs[0], cs[1], ws.x), blend(cs[2], cs[3], ws.y), ws.z);

  gl_FragColor = vec4(final, 1.0);
}