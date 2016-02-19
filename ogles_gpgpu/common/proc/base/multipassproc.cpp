//
// ogles_gpgpu project - GPGPU for mobile devices and embedded systems using OpenGL ES 2.0
//
// Author: Markus Konrad <post@mkonrad.net>, Winter 2014/2015
// http://www.mkonrad.net
//
// See LICENSE file in project repository root for the license.
//

#include "multipassproc.h"

using namespace ogles_gpgpu;

#pragma mark constructor/deconstructor

MultiPassProc::~MultiPassProc() {
    // remove all pass instances
    for(auto &it : procPasses) {
        delete it;
    }

    procPasses.clear();
}

#pragma mark ProcInterface methods

int MultiPassProc::init(int inW, int inH, unsigned int order, bool prepareForExternalInput) {
    ProcInterface *prevProc = NULL;
    int num = 0;
    int numInitialized = 0;
    
    for(auto &it : procPasses) {

        // find out the input frame size for the proc
        int pipelineFrameW, pipelineFrameH;

        if (num == 0) {
            // first pipeline's frame size is the input frame size
            pipelineFrameW = inW;
            pipelineFrameH = inH;
        } else {
            // subsequent pipeline's frame size is the previous processor's output frame size
            pipelineFrameW = prevProc->getOutFrameW();
            pipelineFrameH = prevProc->getOutFrameH();
        }

        numInitialized = it->init(pipelineFrameW, pipelineFrameH, num, num == 0 ? prepareForExternalInput : false);

        // set pointer to previous proc
        prevProc = it;

        num += numInitialized;
    }

    return num;
}

int MultiPassProc::reinit(int inW, int inH, bool prepareForExternalInput) {
    ProcInterface *prevProc = NULL;
    int num = 0;
    int numInitialized = 0;
    
    for(auto &it : procPasses) {

        // find out the input frame size for the proc
        int pipelineFrameW, pipelineFrameH;

        if (num == 0) {
            // first pipeline's frame size is the input frame size
            pipelineFrameW = inW;
            pipelineFrameH = inH;
        } else {
            // subsequent pipeline's frame size is the previous processor's output frame size
            pipelineFrameW = prevProc->getOutFrameW();
            pipelineFrameH = prevProc->getOutFrameH();
        }

        numInitialized = it->reinit(pipelineFrameW, pipelineFrameH, num == 0 ? prepareForExternalInput : false);

        // set pointer to previous proc
        prevProc = it;

        num += numInitialized;
    }

    return num;
}

void MultiPassProc::cleanup() {
    for(auto &it : procPasses) {
        it->cleanup();
    }
}

void MultiPassProc::setExternalInputDataFormat(GLenum fmt) {
    assert(firstProc);
    firstProc->setExternalInputDataFormat(fmt);
}

void MultiPassProc::setExternalInputData(const unsigned char *data) {
    assert(firstProc);
    firstProc->setExternalInputData(data);
}

void MultiPassProc::createFBOTex(bool genMipmap) {
    bool first = true;
    for(auto &it : procPasses) {
        it->createFBOTex(first ? genMipmap : false);
        first = false;
    }
}

void MultiPassProc::render() {
    for(auto &it : procPasses) {
        it->render();
    }
}

void MultiPassProc::printInfo() {
    OG_LOGINF(getProcName(), "begin info for %u passes", (unsigned int)procPasses.size());
    for(auto &it : procPasses) {
        it->printInfo();
    }
    OG_LOGINF(getProcName(), "end info");
}

void MultiPassProc::useTexture(GLuint id, GLuint useTexUnit, GLenum target) {
    ProcInterface *prevProc = NULL;

    for(auto &it : procPasses) {
        if (!prevProc) {    // means this is the first proc pass
            it->useTexture(id, useTexUnit, target);
        } else {            // all other passes
            it->useTexture(prevProc->getOutputTexId(), prevProc->getTextureUnit());
        }
    }
}

GLuint MultiPassProc::getTextureUnit() const {
    assert(firstProc);
    return firstProc->getTextureUnit();
}

void MultiPassProc::setOutputSize(float scaleFactor) {
    assert(firstProc);
    firstProc->setOutputSize(scaleFactor);
}

void MultiPassProc::setOutputSize(int outW, int outH) {
    assert(firstProc);
    firstProc->setOutputSize(outW, outH);
}

int MultiPassProc::getOutFrameW() const {
    assert(lastProc);
    return lastProc->getOutFrameW();
}

int MultiPassProc::getOutFrameH() const {
    assert(lastProc);
    return lastProc->getOutFrameH();
}

int MultiPassProc::getInFrameW() const {
    assert(firstProc);
    return firstProc->getInFrameW();
}

int MultiPassProc::getInFrameH() const {
    assert(firstProc);
    return firstProc->getInFrameH();
}

bool MultiPassProc::getWillDownscale() const {
    for (auto &it : procPasses) {
        if(it->getWillDownscale()) {
            return true;
        }
    }
    return false;
}

void MultiPassProc::getResultData(unsigned char *data) const {
    assert(lastProc);
    return lastProc->getResultData(data);
}

MemTransfer *MultiPassProc::getMemTransferObj() const {
    assert(lastProc);
    return lastProc->getMemTransferObj();
}

GLuint MultiPassProc::getInputTexId() const {
    assert(firstProc);
    return firstProc->getInputTexId();
}

GLuint MultiPassProc::getOutputTexId() const {
    assert(lastProc);
    return lastProc->getOutputTexId();
}

#pragma mark protected methods

void MultiPassProc::multiPassInit() {
    firstProc = procPasses.front();
    lastProc = procPasses.back();
}