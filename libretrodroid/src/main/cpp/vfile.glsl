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
uniform mediump float screenDensity;

#define MIN_SADDLE 0.10
#define MIN_EDGE_1 0.02
#define EDGE_1_THRESHOLD 2.0
#define MIN_EDGE_2 0.02
#define EDGE_2_THRESHOLD 3.0

varying HIGHP vec2 screenCoords;
varying HIGHP vec2 coords;
varying HIGHP vec2 c1;
varying HIGHP vec2 c2;
varying HIGHP vec2 c4;
varying HIGHP vec2 c5;
varying HIGHP vec2 c6;
varying HIGHP vec2 c7;
varying HIGHP vec2 c8;
varying HIGHP vec2 c9;
varying HIGHP vec2 c10;
varying HIGHP vec2 c11;
varying HIGHP vec2 c13;
varying HIGHP vec2 c14;

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

// Edge detection
lowp float hasDiagonal(lowp float d1, lowp float d2, lowp float d3, lowp float d4) {
  lowp float diff1 = abs(d1 - d2);
  lowp float diff2 = abs(d3 - d4);
  return step(EDGE_1_THRESHOLD * diff1 + MIN_EDGE_1, diff2);
}
lowp vec2 minMax(lowp vec2 v) {
  return vec2(min(v.x, v.y), max(v.x, v.y));
}
lowp float maxDelta(lowp vec2 mm) {
  return mm.y - mm.x;
}
lowp float maxDelta(lowp vec2 mm1, lowp vec2 mm2) {
  return max(mm1.y, mm2.y) - min(mm1.x, mm2.x);
}
lowp float minDelta(lowp vec2 mm1, lowp vec2 mm2) {
  return abs(min(mm1.y, mm2.y) - max(mm1.x, mm2.x));
}
lowp float hasDiagonal(lowp vec2 left, lowp vec2 main, lowp vec2 right) {
  lowp vec2 mmLeft = minMax(left);
  lowp vec2 mmMain = minMax(main);
  lowp vec2 mmRight = minMax(right);
  lowp float diff1 = min(maxDelta(mmMain, mmLeft), maxDelta(mmMain, mmRight));
  lowp float diff2 = minDelta(mmLeft, mmRight);
  return step(EDGE_2_THRESHOLD * diff1 + MIN_EDGE_2, diff2);
}
bool isSaddle(lowp float d1, lowp float d2, lowp float d3, lowp float d4) {
  return abs(d1 - d2) < MIN_EDGE_1 && abs(d3 - d4) < MIN_EDGE_1;
}
// TODO FILIPPO... It would be very cool to pack 4 values here instead of three
lowp float pack(lowp vec3 values) {
  return dot(values, vec3(4.0, 16.0, 64.0));
}

void main() {
  lowp vec3 t1 = texture2D(texture, c1).rgb;
  lowp vec3 t2 = texture2D(texture, c2).rgb;
  lowp vec3 t4 = texture2D(texture, c4).rgb;
  lowp vec3 t5 = texture2D(texture, c5).rgb;
  lowp vec3 t6 = texture2D(texture, c6).rgb;
  lowp vec3 t7 = texture2D(texture, c7).rgb;
  lowp vec3 t8 = texture2D(texture, c8).rgb;
  lowp vec3 t9 = texture2D(texture, c9).rgb;
  lowp vec3 t10 = texture2D(texture, c10).rgb;
  lowp vec3 t11 = texture2D(texture, c11).rgb;
  lowp vec3 t13 = texture2D(texture, c13).rgb;
  lowp vec3 t14 = texture2D(texture, c14).rgb;

  lowp float l1 = luma(t1);
  lowp float l2 = luma(t2);
  lowp float l4 = luma(t4);
  lowp float l5 = luma(t5);
  lowp float l6 = luma(t6);
  lowp float l7 = luma(t7);
  lowp float l8 = luma(t8);
  lowp float l9 = luma(t9);
  lowp float l10 = luma(t10);
  lowp float l11 = luma(t11);
  lowp float l13 = luma(t13);
  lowp float l14 = luma(t14);

  lowp float d05_10 = hasDiagonal(l5, l10, l6, l9);
  lowp float d06_09 = hasDiagonal(l6, l9, l5, l10);

  // Saddle fix
  if (isSaddle(l5, l10, l6, l9)) {
    lowp float average = (l1 + l2 + l4 + l7 + l8 + l11 + l13 + l14) / 8.0;
    lowp float diff1 = abs(mix(l5, l10, 0.5) - average);
    lowp float diff2 = abs(mix(l6, l9, 0.5) - average);
    d05_10 = step(diff2 + MIN_SADDLE, diff1);
    d06_09 = step(diff1 + MIN_SADDLE, diff2);
  }

  // Vertical diagonals
  lowp float d01_10 = hasDiagonal(vec2(l5, l9), vec2(l1, l10), vec2(l2, l6));
  lowp float d02_09 = hasDiagonal(vec2(l1, l5), vec2(l2, l9), vec2(l6, l10));
  lowp float d05_14 = hasDiagonal(vec2(l9, l13), vec2(l5, l14), vec2(l6, l10));
  lowp float d06_13 = hasDiagonal(vec2(l5, l9), vec2(l6, l13), vec2(l10, l14));

  // Horizontal diagonals
  lowp float d04_10 = hasDiagonal(vec2(l8, l9), vec2(l4, l10), vec2(l5, l6));
  lowp float d06_08 = hasDiagonal(vec2(l9, l10), vec2(l6, l8), vec2(l4, l5));
  lowp float d05_11 = hasDiagonal(vec2(l9, l10), vec2(l5, l11), vec2(l6, l7));
  lowp float d07_09 = hasDiagonal(vec2(l10, l11), vec2(l7, l9), vec2(l5, l6));

  lowp vec4 final = vec4(0.0);

  if (d02_09 > 0.0 && d06_08 > 0.0) {
    final.x = 0.55;
    final.y = pack(vec3(0.0, 1.0, 0.0));
  } else if (d01_10 > 0.0 && d05_11 > 0.0) {
    final.x = 0.55;
    final.y = pack(vec3(0.0, 0.0, 1.0));
  } else if (d06_13 > 0.0 && d07_09 > 0.0) {
    final.x = 0.55;
    final.y = pack(vec3(1.0, 0.0, 0.0));
  } else if (d05_14 > 0.0 && d04_10 > 0.0) {
    final.x = 0.55;
    final.y = pack(vec3(0.0, 0.0, 0.0));
  } else if (d05_11 > 0.0 && d06_08 > 0.0) {
    final.x = 0.45;
    final.y = pack(vec3(0.0, 0.0, 1.0));
  } else if (d04_10 > 0.0 && d07_09 > 0.0) {
    final.x = 0.45;
    final.y = pack(vec3(0.0, 1.0, 1.0));
  } else if (d05_14 > 0.0 && d02_09 > 0.0) {
    final.x = 0.45;
    final.y = pack(vec3(0.0, 0.0, 0.0));
  } else if (d01_10 > 0.0 && d06_13 > 0.0) {
    final.x = 0.45;
    final.y = pack(vec3(1.0, 0.0, 0.0));
  } else if (d05_11 > 0.0 && d04_10 > 0.0) {
    final.x = 0.35;
    final.y = pack(vec3(0.0, 0.0, 1.0));
  } else if (d06_08 > 0.0 && d07_09 > 0.0) {
    final.x = 0.35;
    final.y = pack(vec3(1.0, 0.0, 1.0));
  } else if (d01_10 > 0.0 && d05_14 > 0.0) {
    final.x = 0.35;
    final.y = pack(vec3(0.0, 0.0, 0.0));
  } else if (d02_09 > 0.0 && d06_13 > 0.0) {
    final.x = 0.35;
    final.y = pack(vec3(1.0, 0.0, 0.0));
  } else if (d04_10 > 0.0) {
    final.x = 0.25;
    final.y = pack(vec3(1.0, 1.0, 1.0));
  } else if (d06_13 > 0.0) {
    final.x = 0.25;
    final.y = pack(vec3(1.0, 0.0, 0.0));
  } else if (d05_11 > 0.0) {
    final.x = 0.25;
    final.y = pack(vec3(0.0, 0.0, 1.0));
  } else if (d05_14 > 0.0) {
    final.x = 0.25;
    final.y = pack(vec3(0.0, 0.0, 0.0));
  } else if (d07_09 > 0.0) {
    final.x = 0.25;
    final.y = pack(vec3(0.0, 1.0, 1.0));
  } else if (d01_10 > 0.0) {
    final.x = 0.25;
    final.y = pack(vec3(1.0, 1.0, 0.0));
  } else if (d06_08 > 0.0) {
    final.x = 0.25;
    final.y = pack(vec3(1.0, 0.0, 1.0));
  } else if (d02_09 > 0.0) {
    final.x = 0.25;
    final.y = pack(vec3(0.0, 1.0, 0.0));
  } else if (d05_10 > 0.0) {
    final.x = 0.15;
    final.y = pack(vec3(0.0, 0.0, 0.0));
  } else if (d06_09 > 0.0) {
    final.x = 0.15;
    final.y = pack(vec3(1.0, 0.0, 0.0));
  }

  gl_FragColor = vec4(final.x, final.y / 255.0, 0.0, 1.0);
}