//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015
// http://www.mkonrad.net
//
// See LICENSE file in project repository root for the license.
//

// Copyright (c) 2016-2017, David Hirvonen (this file)

#include "thresh.h"
#include "../common_includes.h"

using namespace std;
using namespace ogles_gpgpu;

// Simple thresholding fragment shader
// Requires a grayscale image as input!

// clang-format off
const char *ThreshProc::fshaderSimpleThreshSrc = 

#if defined(OGLES_GPGPU_OPENGLES)
OG_TO_STR(precision mediump float;)
#endif
OG_TO_STR(
varying vec2 vTexCoord;
uniform float uThresh;
uniform sampler2D uInputTex;
void main() {
    float gray = texture2D(uInputTex, vTexCoord).r;
    float bin = step(uThresh, gray);
    gl_FragColor = vec4(bin, bin, bin, 1.0);
});
// clang-format on

ThreshProc::ThreshProc() {
    // set defaults
    threshVal = 0.5f;
}

int ThreshProc::init(int inW, int inH, unsigned int order, bool prepareForExternalInput) {
    OG_LOGINF(getProcName(), "initialize");

    // create fbo for output
    createFBO();

    // parent init - set defaults
    baseInit(inW, inH, order, prepareForExternalInput, procParamOutW, procParamOutH, procParamOutScale);

    // FilterProcBase init - create shaders, get shader params, set buffers for OpenGL
    filterInit(vshaderDefault, fshaderSimpleThreshSrc);

    // get additional shader params
    shParamUThresh = shader->getParam(UNIF, "uThresh");

    return 1;
}

int ThreshProc::render(int position) {
    OG_LOGINF(getProcName(), "input tex %d, target %d, framebuffer of size %dx%d", texId, texTarget, outFrameW, outFrameH);

    filterRenderPrepare();

    glUniform1f(shParamUThresh, threshVal); // thresholding value for simple thresholding

    Tools::checkGLErr("ThreshProc", "render prepare");

    filterRenderSetCoords();
    Tools::checkGLErr("ThreshProc", "render set coords");

    filterRenderDraw();
    Tools::checkGLErr("ThreshProc", "render draw");

    filterRenderCleanup();
    Tools::checkGLErr("ThreshProc", "render cleanup");

    return 0;
}
