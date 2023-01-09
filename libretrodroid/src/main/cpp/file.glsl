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
lowp float intersection(lowp vec2 a, lowp vec2 b, lowp vec2 c, lowp vec2 d) {
  lowp float ydc = d.y - c.y;
  lowp float xdc = d.x - c.x;
  lowp float num = (a.y - c.y) * xdc - (a.x - c.x) * ydc;
  lowp float denom = (b.x - a.x) * ydc - (b.y - a.y) * xdc;
  return num / denom;
}
lowp vec3 barycentric(lowp vec2 ps[3], lowp vec2 p) {
  lowp float ri1 = intersection(ps[0], ps[1], p + ps[1] - ps[2], p + ps[2] - ps[1]);
  lowp vec2 ai1 = mix(ps[0], ps[1], ri1);

  lowp float ri2 = intersection(ps[0], ps[2], p + ps[1] - ps[2], p + ps[2] - ps[1]);
  lowp vec2 ai2 = mix(ps[0], ps[2], ri2);

  return vec3(ri1, ri2, distance(p, ai1) / (distance(p, ai1) + distance(p, ai2)));
}

void triangleInterpolate(lowp vec3 t5, lowp vec3 t6, lowp vec3 t9, lowp vec3 t10, lowp vec3 flags[3], lowp vec2 pxCoords, out lowp vec3 outColors[4], out lowp vec3 weights) {
  lowp vec2 pCoords[3];
  lowp vec3 pColors[3];

  lowp vec3 a1 = t5;
  lowp vec3 b1 = t6;
  lowp vec3 c1 = t9;
  lowp vec3 d1 = t10;

  bool du = flags[1].x > 0.0;
  bool dd = flags[1].y > 0.0;
  bool dl = flags[2].x > 0.0;
  bool dr = flags[2].y > 0.0;

  bool negative = flags[0].y > 0.0;

  if (negative) {
    pxCoords = vec2(1.0 - pxCoords.x, pxCoords.y);
    a1 = t6; b1 = t5; c1 = t10; d1 = t9;
    bool tmp = dl; dl = dr; dr = tmp; // TODO FILIPPO Maybe move the vertex shader
  }

  lowp vec3 a = a1;
  lowp vec3 b = b1;
  lowp vec3 c = c1;
  lowp vec3 d = d1;

  bool inverted = pxCoords.x > pxCoords.y;

  if (inverted) {
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
    if (flags[1].z > 0.0) { // This interpolation only works with triangles which where originally half a square
      pCoords[0] = vec2(0.0, 1.0); pCoords[1] = vec2(0.0, 0.0); pCoords[2] = vec2(1.0, 1.0);
      pColors[0] = c; pColors[1] = a; pColors[2] = d;
    } else if (flags[1].x > 0.0) {
      pCoords[0] = vec2(1.0, 1.0); pCoords[1] = vec2(0.0, 1.0); pCoords[2] = vec2(0.0, 0.0);
      pColors[0] = d; pColors[1] = c; pColors[2] = a;
    } else if (flags[1].y > 0.0) {
      pCoords[0] = vec2(0.0, 0.0); pCoords[1] = vec2(1.0, 1.0); pCoords[2] = vec2(0.0, 1.0); 
      pColors[0] = a; pColors[1] = d; pColors[2] = c;
    } else if (flags[2].z > 0.0) {
      pCoords[0] = vec2(0.0, 0.0); pCoords[1] = vec2(1.0, 1.0); pCoords[2] = vec2(0.0, 1.0); 
      pColors[0] = a; pColors[1] = d; pColors[2] = c;
    } else {
      pCoords[0] = vec2(1.0, 1.0); pCoords[1] = vec2(0.0, 1.0); pCoords[2] = vec2(0.0, 0.0);
      pColors[0] = d; pColors[1] = c; pColors[2] = a;
    }
  }

  outColors[0] = pColors[0]; outColors[1] = pColors[1]; outColors[2] = pColors[0]; outColors[3] = pColors[2];
  weights = barycentric(pCoords, pxCoords);
}
void swap(inout lowp vec3 a, inout lowp vec3 b) {
  lowp vec3 tmp = a;
  a = b;
  b = tmp;
}
void quadInterpolate(lowp vec3 t5, lowp vec3 t6, lowp vec3 t9, lowp vec3 t10, lowp vec3 flags[3], lowp vec2 pxCoords, out lowp vec3 outColors[4], out lowp vec3 outWeights) {
  lowp vec2 finalCoords;

  bool h0 = flags[1].x > 0.0;
  bool h1 = flags[1].y > 0.0;

  bool v0 = flags[2].x > 0.0;
  bool v1 = flags[2].y > 0.0;

  lowp vec3 a = t5;
  lowp vec3 b = t6;
  lowp vec3 c = t9;
  lowp vec3 d = t10;

  if (v0 || v1) {
    h0 = v0;
    h1 = v1;
    a = t9; b = t5; c = t10; d = t6;
    pxCoords = vec2(1.0 - pxCoords.y, pxCoords.x);
  }

  if (h0) {
    if (pxCoords.y > 0.5) {
      outColors[0] = d; outColors[1] = c; outColors[2] = c; outColors[3] = d;
      finalCoords = vec2(pxCoords.x, 2.0 * (pxCoords.y - 0.5));
    } else {
      outColors[0] = a; outColors[1] = b; outColors[2] = d; outColors[3] = c;
      finalCoords = vec2(pxCoords.x, 2.0 * pxCoords.y);
    }
  } else if (h1) {
    if (pxCoords.y > 0.5) {
      outColors[0] = b; outColors[1] = a; outColors[2] = c; outColors[3] = d;
      finalCoords = vec2(pxCoords.x, 2.0 * (pxCoords.y - 0.5));
    } else {
      outColors[0] = a; outColors[1] = b; outColors[2] = b; outColors[3] = a;
      finalCoords = vec2(pxCoords.x, 2.0 * pxCoords.y);
    }
  } else {
    outColors[0] = a; outColors[1] = b, outColors[2] = c, outColors[3] = d;
    finalCoords = pxCoords;
  }

  outWeights = vec3(finalCoords.x, finalCoords.x, finalCoords.y);
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

  lowp vec3 colors[4];
  lowp vec3 weights;

  bool triangulate = flags[0].x > 0.0 || flags[0].y > 0.0;
  if (triangulate) {
    triangleInterpolate(t5, t6, t9, t10, flags, pxCoords, colors, weights);
  } else {
    quadInterpolate(t5, t6, t9, t10, flags, pxCoords, colors, weights);
  }
  lowp vec3 final = blend(blend(colors[0], colors[1], weights.x), blend(colors[2], colors[3], weights.y), weights.z);

  gl_FragColor = vec4(final, 1.0);
}