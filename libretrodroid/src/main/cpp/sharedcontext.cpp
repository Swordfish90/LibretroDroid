#include "sharedcontext.h"

#include <stdexcept>

namespace libretrodroid {

SharedContext::SharedContext() {
    display = eglGetCurrentDisplay();
    frontend = eglGetCurrentContext();
    EGLint configId, version, count;
    EGLConfig config;
    if (display == EGL_NO_DISPLAY || frontend == EGL_NO_CONTEXT ||
        !eglQueryContext(display, frontend, EGL_CONFIG_ID, &configId) ||
        !eglQueryContext(display, frontend, EGL_CONTEXT_CLIENT_VERSION, &version)) {
        throw std::runtime_error("Cannot query the frontend EGL context");
    }
    const EGLint configAttributes[] = {EGL_CONFIG_ID, configId, EGL_NONE};
    auto result = eglChooseConfig(display, configAttributes, &config, 1, &count);
    if (!result || count != 1) {
        throw std::runtime_error("Cannot find the frontend EGL configuration");
    }
    const EGLint contextAttributes[] = {EGL_CONTEXT_CLIENT_VERSION, version, EGL_NONE};
    core = eglCreateContext(display, config, frontend, contextAttributes);
    if (core == EGL_NO_CONTEXT) {
        throw std::runtime_error("Cannot create the shared EGL context");
    }
}

SharedContext::~SharedContext() {
    if (core != EGL_NO_CONTEXT) {
        eglDestroyContext(display, core);
    }
}

void SharedContext::bind(EGLContext context) {
    if (eglGetCurrentContext() == context) return;
    auto result = eglMakeCurrent(
        display,
        eglGetCurrentSurface(EGL_DRAW),
        eglGetCurrentSurface(EGL_READ),
        context
    );
    if (!result) {
        throw std::runtime_error("Cannot switch EGL context");
    }
}

void SharedContext::bindCore() { bind(core); }
void SharedContext::bindFrontend() { bind(frontend); }

} // namespace libretrodroid
