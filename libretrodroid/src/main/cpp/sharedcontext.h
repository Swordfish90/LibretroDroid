#ifndef LIBRETRODROID_SHAREDCONTEXT_H
#define LIBRETRODROID_SHAREDCONTEXT_H

#include <EGL/egl.h>

namespace libretrodroid {

// Both contexts are used on the renderer thread and share its current surface.
class SharedContext {
public:
    SharedContext();
    ~SharedContext();
    void bindCore();
    void bindFrontend();

private:
    void bind(EGLContext context);
    EGLDisplay display = EGL_NO_DISPLAY;
    EGLContext frontend = EGL_NO_CONTEXT;
    EGLContext core = EGL_NO_CONTEXT;
};

}
#endif
