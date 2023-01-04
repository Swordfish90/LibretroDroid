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

// Interpolation
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
lowp vec3 barycentric(lowp vec2 a, lowp vec2 b, lowp vec2 c, lowp vec2 p, lowp float sharpness) {
  lowp float denom = 1.0 / ((b.y - c.y) * (a.x - c.x) + (c.x - b.x) * (a.y - c.y));
  float l0 = denom * ((b.y - c.y) * (p.x - c.x) + (c.x - b.x) * (p.y - c.y));
  float l1 = denom * ((c.y - a.y) * (p.x - c.x) + (a.x - c.x) * (p.y - c.y));
  l0 = sharpSmooth(l0, sharpness);
  return vec3(l0, l1, 1.0 - l0 - l1);
}

lowp vec3 quadBilinear(lowp vec3 a, lowp vec3 b, lowp vec3 c, lowp vec3 d, lowp vec2 p, lowp float sharpness) {
  lowp float x = sharpSmooth(p.x, sharpness);
  lowp float y = sharpSmooth(p.y, sharpness);
  return mix(mix(a, b, x), mix(c, d, x), y);
}

lowp vec3 triangleInterpolate(lowp vec3 t5, lowp vec3 t6, lowp vec3 t9, lowp vec3 t10, lowp vec3 flags[3], lowp vec2 pxCoords, lowp float sharpness) {
  lowp vec2 pCoords[3];
  lowp vec3 pColors[3];

  lowp vec3 a1 = t5;
  lowp vec3 b1 = t6;
  lowp vec3 c1 = t9;
  lowp vec3 d1 = t10;

  lowp bool du = flags[1].x > 0.0;
  lowp bool dd = flags[1].y > 0.0;
  lowp bool dl = flags[2].x > 0.0;
  lowp bool dr = flags[2].y > 0.0;

  if (flags[0].y > 0.0) {
    pxCoords = vec2(1.0 - pxCoords.x, pxCoords.y);
    a1 = t6; b1 = t5; c1 = t10; d1 = t9;
    bool tmp = dl; dl = dr; dr = tmp; // TODO FILIPPO Maybe move the vertex shader
  }

  lowp vec3 a = a1;
  lowp vec3 b = b1;
  lowp vec3 c = c1;
  lowp vec3 d = d1;

  if (pxCoords.x > pxCoords.y) {
    pxCoords = vec2(pxCoords.y, pxCoords.x);
    b = c1; c = b1;
    dd = dr; dl = du;
  }

  if (dd && dl) {
    if (pxCoords.y > 0.5 + pxCoords.x) {
      pCoords[0] = vec2(0.0, 0.5); pCoords[1] = vec2(0.0, 1.0); pCoords[2] = vec2(0.5, 1.0);
      pColors[0] = d; pColors[1] = c; pColors[2] = a;
    } else {
      pCoords[0] = vec2(0.0, 0.0); pCoords[1] = vec2(0.0, 1.0); pCoords[2] = vec2(1.0, 1.0);
      pColors[0] = a; pColors[1] = a; pColors[2] = d; // TODO FILIPPO... La a e' una porcata
    }
  } else if (dd) {
    if (pxCoords.y > 2.0 * pxCoords.x) {
      pCoords[0] = vec2(0.0, 1.0); pCoords[1] = vec2(0.0, 0.0); pCoords[2] = vec2(0.5, 1.0);
      pColors[0] = c; pColors[1] = a; pColors[2] = a;
    } else {
      pCoords[0] = vec2(1.0, 1.0); pCoords[1] = vec2(0.0, 0.0); pCoords[2] = vec2(0.5, 1.0);
      pColors[0] = d; pColors[1] = a; pColors[2] = a;
    }
  } else if (dl) {
    if (pxCoords.y > 0.5 * pxCoords.x + 0.5) {
      pCoords[0] = vec2(0.0, 1.0); pCoords[1] = vec2(0.0, 0.5); pCoords[2] = vec2(1.0, 1.0);
      pColors[0] = c; pColors[1] = d; pColors[2] = d;
    } else {
      pCoords[0] = vec2(0.0, 0.0); pCoords[1] = vec2(0.0, 0.5); pCoords[2] = vec2(1.0, 1.0);
      pColors[0] = a; pColors[1] = d; pColors[2] = d;
    }
  } else {
    pCoords[0] = vec2(0.0, 1.0); pCoords[1] = vec2(0.0, 0.0); pCoords[2] = vec2(1.0, 1.0);
    pColors[0] = c; pColors[1] = a; pColors[2] = d;
  }

  lowp vec3 weights = barycentric(pCoords[0], pCoords[1], pCoords[2], pxCoords, sharpness);
  return weights.x * pColors[0] + weights.y * pColors[1] + weights.z * pColors[2];
}

lowp vec3 quadInterpolate(lowp vec3 t5, lowp vec3 t6, lowp vec3 t9, lowp vec3 t10, lowp vec3 flags[3], lowp vec2 pxCoords, lowp float sharpness) {
  lowp vec3 pColors[4];
  lowp vec2 finalCoords;

  lowp bool h0 = flags[1].x > 0.0;
  lowp bool h1 = flags[1].y > 0.0;
  lowp bool v0 = flags[2].x > 0.0;
  lowp bool v1 = flags[2].y > 0.0;

  if (flags[0].z > 0.0) {
    if (h0) {
      if (pxCoords.y > 0.5) {
        pColors[0] = t10; pColors[1] = t9, pColors[2] = t9, pColors[3] = t10;
        finalCoords = vec2(pxCoords.x, 2.0 * (pxCoords.y - 0.5));
      } else {
        pColors[0] = t5; pColors[1] = t6, pColors[2] = t10, pColors[3] = t9;
        finalCoords = vec2(pxCoords.x, 2.0 * pxCoords.y);
      }
    } else if (h1) {
      if (pxCoords.y > 0.5) {
        pColors[0] = t6; pColors[1] = t5, pColors[2] = t9, pColors[3] = t10;
        finalCoords = vec2(pxCoords.x, 2.0 * (pxCoords.y - 0.5));
      } else {
        pColors[0] = t5; pColors[1] = t6, pColors[2] = t6, pColors[3] = t5;
        finalCoords = vec2(pxCoords.x, 2.0 * pxCoords.y);
      }
    } else if (v0) {
      if (pxCoords.x > 0.5) {
        pColors[0] = t10; pColors[1] = t6, pColors[2] = t6, pColors[3] = t10;
        finalCoords = vec2(2.0 * (pxCoords.x - 0.5), pxCoords.y);
      } else {
        pColors[0] = t5; pColors[1] = t10, pColors[2] = t9, pColors[3] = t6;
        finalCoords = vec2(2.0 * pxCoords.x, pxCoords.y);
      }
    } else if (v1) {
      if (pxCoords.x > 0.5) {
        pColors[0] = t9; pColors[1] = t6, pColors[2] = t5, pColors[3] = t10;
        finalCoords = vec2(2.0 * (pxCoords.x - 0.5), pxCoords.y);
      } else {
        pColors[0] = t5; pColors[1] = t9, pColors[2] = t9, pColors[3] = t5;
        finalCoords = vec2(2.0 * pxCoords.x, pxCoords.y);
      }
    }
  } else {
    pColors[0] = t5; pColors[1] = t6, pColors[2] = t9, pColors[3] = t10;
    finalCoords = pxCoords;
  }

  return quadBilinear(pColors[0], pColors[1], pColors[2], pColors[3], finalCoords, sharpness);
}

void main() {
  lowp vec3 t5 = texture2D(texture, c5).rgb;
  lowp vec3 t6 = texture2D(texture, c6).rgb;
  lowp vec3 t9 = texture2D(texture, c9).rgb;
  lowp vec3 t10 = texture2D(texture, c10).rgb;

  lowp vec4 flagsTexture = floor(texture2D(previousPass, passCoords) * 255.0 + 0.5);
  lowp vec3 flags[3];
  flags[0] = unpack(flagsTexture.x);
  flags[1] = unpack(flagsTexture.y);
  flags[2] = unpack(flagsTexture.z);

  lowp vec2 pxCoords = fract(screenCoords);

  lowp vec3 final = vec3(0.0);

  bool triangulate = flags[0].x > 0.0 || flags[0].y > 0.0;
  if (triangulate) {
    final = triangleInterpolate(t5, t6, t9, t10, flags, pxCoords, 0.5);
  } else {
    final = quadInterpolate(t5, t6, t9, t10, flags, pxCoords, 0.5);
  }

  gl_FragColor = vec4(final, 1.0);
}