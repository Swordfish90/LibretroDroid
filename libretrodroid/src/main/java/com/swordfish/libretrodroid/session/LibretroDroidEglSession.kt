package com.swordfish.libretrodroid.session

import android.opengl.EGL14
import android.opengl.EGLConfig
import android.opengl.EGLContext
import android.opengl.EGLExt
import android.opengl.EGLSurface
import androidx.graphics.opengl.egl.EGLConfigAttributes
import androidx.graphics.opengl.egl.EGLManager
import androidx.graphics.opengl.egl.EGLSpec

internal class LibretroDroidEglSession(private val glesVersion: Int) : EGLSpec by EGLSpec.V14 {

    var swapRequested = false

    override fun eglSwapBuffers(surface: EGLSurface): Boolean {
        if (!swapRequested) {
            return false
        }
        swapRequested = false
        return EGLSpec.V14.eglSwapBuffers(surface)
    }

    override fun eglCreateContext(config: EGLConfig): EGLContext {
        return EGL14.eglCreateContext(
            EGL14.eglGetDisplay(EGL14.EGL_DEFAULT_DISPLAY),
            config,
            EGL14.EGL_NO_CONTEXT,
            intArrayOf(EGL14.EGL_CONTEXT_CLIENT_VERSION, glesVersion, EGL14.EGL_NONE),
            0
        )
    }

    fun chooseConfig(eglManager: EGLManager): EGLConfig {
        val type = if (glesVersion >= 3) {
            EGLExt.EGL_OPENGL_ES3_BIT_KHR
        } else {
            EGL14.EGL_OPENGL_ES2_BIT
        }
        return eglManager.loadConfig(
            EGLConfigAttributes {
                EGL14.EGL_RENDERABLE_TYPE to type
                EGL14.EGL_SURFACE_TYPE to (EGL14.EGL_WINDOW_BIT or EGL14.EGL_PBUFFER_BIT)
                EGL14.EGL_RED_SIZE to 8
                EGL14.EGL_GREEN_SIZE to 8
                EGL14.EGL_BLUE_SIZE to 8
                EGL14.EGL_ALPHA_SIZE to 8
            }
        ) ?: error("No compatible EGL config")
    }
}
