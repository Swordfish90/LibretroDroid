package com.swordfish.libretrodroid.session

import android.graphics.RectF
import android.opengl.EGL14
import android.opengl.EGLConfig
import android.opengl.EGLSurface
import android.util.Log
import android.view.Surface
import android.view.SurfaceView
import androidx.annotation.MainThread
import androidx.graphics.opengl.GLRenderer
import androidx.graphics.opengl.egl.EGLConfigAttributes
import androidx.graphics.opengl.egl.EGLManager
import androidx.graphics.opengl.egl.EGLSpec
import com.swordfish.libretrodroid.Controller
import com.swordfish.libretrodroid.DetachedVirtualFile
import com.swordfish.libretrodroid.KtUtils.awaitUninterruptibly
import com.swordfish.libretrodroid.KtUtils.joinUninterruptibly
import com.swordfish.libretrodroid.KtUtils.runOnUIThread
import com.swordfish.libretrodroid.LibretroDroid
import com.swordfish.libretrodroid.RetroException
import com.swordfish.libretrodroid.RumbleEvent
import com.swordfish.libretrodroid.ShaderConfig
import com.swordfish.libretrodroid.Variable
import com.swordfish.libretrodroid.ViewportAlignment
import com.swordfish.libretrodroid.toNativeShader
import kotlinx.coroutines.CompletableDeferred
import kotlinx.coroutines.Deferred
import kotlinx.coroutines.completeWith
import kotlinx.coroutines.channels.BufferOverflow
import kotlinx.coroutines.flow.MutableSharedFlow
import kotlinx.coroutines.flow.asSharedFlow
import java.util.concurrent.CountDownLatch
import kotlin.properties.Delegates

// TODO 1.0 This class should be exposed
internal class LibretroDroidSession(
    private val config: SessionConfig
) {

    enum class Event { Created } // TODO 1.0 This should be deprecated

    private var failure: Throwable? = null
    private val nativeTeardownComplete = CountDownLatch(1)
    @Volatile private var ownerThread: Thread? = null
    private var isResumed = false
    private var framePending = false
    @Volatile private var renderTarget: GLRenderer.RenderTarget? = null

    private val eventFlow = MutableSharedFlow<Event>(1, 1, BufferOverflow.DROP_OLDEST)
    private val errorFlow = MutableSharedFlow<Int>(1, 1, BufferOverflow.DROP_OLDEST)
    private val rumbleFlow = MutableSharedFlow<RumbleEvent>(0, 64, BufferOverflow.DROP_OLDEST)

    val events = eventFlow.asSharedFlow()
    val errors = errorFlow.asSharedFlow()
    val rumbleEvents = rumbleFlow.asSharedFlow()

    private val nativeCallbacks = object : LibretroDroid.NativeCallbacks {
        override fun onRumbleEvent(port: Int, strengthWeak: Float, strengthStrong: Float) {
            rumbleFlow.tryEmit(RumbleEvent(port, strengthWeak, strengthStrong))
        }
    }

    var audioEnabled: Boolean by Delegates.observable(true) { _, _, value ->
        dispatch { LibretroDroid.setAudioEnabled(value) }
    }
    var frameSpeed: Int by Delegates.observable(1) { _, _, value ->
        dispatch { LibretroDroid.setFrameSpeed(value) }
    }
    var shader: ShaderConfig by Delegates.observable(config.shader) { _, _, value ->
        dispatch { LibretroDroid.setShaderConfig(value.toNativeShader()) }
    }
    var viewport: RectF by Delegates.observable(RectF(0f, 0f, 1f, 1f)) { _, _, value ->
        val rect = RectF(value)
        dispatch { LibretroDroid.setViewport(rect.left, rect.top, rect.width(), rect.height()) }
    }
    var viewportAlignment: ViewportAlignment by Delegates.observable(config.viewportAlignment) { _, _, value ->
        dispatch { LibretroDroid.setViewportAlignment(value.value) }
    }

    private val eglSession = LibretroDroidEglSession(config.glesVersion)
    private val renderer = buildGLRenderer()

    @MainThread
    fun create() {
        synchronized(sessionLock) {
            check(ownerThread == null) { "A session can only be created once" }
            check(activeSession == null) { "Only one libretro session can be active" }
            activeSession = this
            renderer.start()
        }
    }

    @MainThread
    fun resume() {
        dispatch {
            LibretroDroid.resume()
            isResumed = true
            render()
        }
    }

    @MainThread
    fun pause() {
        dispatch {
            isResumed = false
            LibretroDroid.pause()
        }
    }

    @MainThread
    fun destroy() {
        stop()
        nativeTeardownComplete.awaitUninterruptibly()
        ownerThread?.joinUninterruptibly()
    }

    @MainThread
    fun attach(surfaceView: SurfaceView) {
        check(renderTarget == null) { "Detach the previous view before attaching another" }
        renderTarget = renderer.attach(surfaceView, object : GLRenderer.RenderCallback {
            override fun onSurfaceCreated(
                spec: EGLSpec,
                config: EGLConfig,
                surface: Surface,
                width: Int,
                height: Int,
            ): EGLSurface? {
                if (renderTarget == null || failure != null) return null
                val createSurfaceResult = runCatching {
                    LibretroDroid.onSurfaceChanged(width, height)
                    spec.eglCreateWindowSurface(config, surface, null).also {
                        check(it != EGL14.EGL_NO_SURFACE) { "Cannot create the window EGL surface" }
                    }
                }
                createSurfaceResult.onFailure(::fail)
                return createSurfaceResult.getOrNull()
            }

            override fun onDrawFrame(eglManager: EGLManager) {
                if (!isResumed || renderTarget == null) return
                val stepResult = runCatching {
                    eglSession.swapRequested = LibretroDroid.step(nativeCallbacks)
                }
                stepResult.onFailure(::fail)
                render()
            }
        })
        renderTarget?.requestRender()
    }

    @MainThread
    fun detach() {
        val currentRenderTarget = renderTarget ?: return
        renderTarget = null
        val detachmentComplete = CountDownLatch(1)
        currentRenderTarget.detach(false) { detachmentComplete.countDown() }
        detachmentComplete.awaitUninterruptibly()
    }

    suspend fun serializeState(): ByteArray {
        return dispatch { LibretroDroid.serializeState() }.await()
    }

    suspend fun unserializeState(data: ByteArray): Boolean {
        return dispatch { LibretroDroid.unserializeState(data) }.await()
    }

    suspend fun serializeSRAM(): ByteArray {
        return dispatch { LibretroDroid.serializeSRAM() }.await()
    }

    suspend fun unserializeSRAM(data: ByteArray): Boolean {
        return dispatch { LibretroDroid.unserializeSRAM(data) }.await()
    }

    fun setCheat(index: Int, enable: Boolean, code: String) {
        dispatch { LibretroDroid.setCheat(index, enable, code) }
    }

    fun reset() {
        dispatch { LibretroDroid.reset() }
    }

    suspend fun getControllers(): Array<Array<Controller>> {
        return dispatch { LibretroDroid.getControllers() }.await()
    }

    fun setControllerType(port: Int, type: Int) {
        dispatch { LibretroDroid.setControllerType(port, type) }
    }

    suspend fun getVariables(): Array<Variable> {
        return dispatch { LibretroDroid.getVariables() }.await()
    }

    fun updateVariables(vararg variables: Variable) {
        dispatch { variables.forEach { LibretroDroid.updateVariable(it) } }
    }

    suspend fun getAvailableDisks(): Int {
        return dispatch { LibretroDroid.availableDisks() }.await()
    }

    suspend fun getCurrentDisk(): Int {
        return dispatch { LibretroDroid.currentDisk() }.await()
    }

    fun changeDisk(index: Int) {
        dispatch { LibretroDroid.changeDisk(index) }
    }

    fun sendKeyEvent(action: Int, keyCode: Int, port: Int) {
        dispatch { LibretroDroid.onKeyEvent(port, action, keyCode) }
    }

    fun sendMotionEvent(source: Int, xAxis: Float, yAxis: Float, port: Int) {
        dispatch { LibretroDroid.onMotionEvent(port, source, xAxis, yAxis) }
    }

    fun sendTouchEvent(x: Float, y: Float) {
        dispatch { LibretroDroid.onTouchEvent(x, y) }
    }

    private fun buildGLRenderer(): GLRenderer {
        val renderer = GLRenderer(
            eglSpecFactory = { eglSession },
            eglConfigFactory = { eglSession.chooseConfig(this) }
        )

        renderer.registerEGLContextCallback(object : GLRenderer.EGLContextCallback {
            override fun onEGLContextCreated(eglManager: EGLManager) {
                ownerThread = Thread.currentThread()
                val result = runCatching {
                    check(eglManager.makeCurrent(eglManager.defaultSurface)) { "Cannot bind the default EGL surface" }
                    createLibretroDroidSession()
                    renderFirstFrame(eglManager)
                    eventFlow.tryEmit(Event.Created)
                }
                result.onFailure(::fail)
            }

            override fun onEGLContextDestroyed(eglManager: EGLManager) {
                val result = runCatching {
                    check(eglManager.makeCurrent(eglManager.defaultSurface)) { "Cannot bind the default EGL surface" }
                    LibretroDroid.destroy()
                    synchronized(sessionLock) { activeSession = null }
                }
                result.onFailure(::fail)
                nativeTeardownComplete.countDown()
            }
        })

        return renderer
    }

    private fun createLibretroDroidSession() {
        LibretroDroid.create(
            config.glesVersion,
            config.coreFilePath,
            config.systemDirectory,
            config.savesDirectory,
            config.variables,
            shader.toNativeShader(),
            config.refreshRate,
            config.preferLowLatencyAudio,
            config.game is GameSource.VirtualFiles,
            config.enableMicrophone,
            false,
            config.immersiveMode,
            config.language,
        )
        LibretroDroid.setAudioEnabled(audioEnabled)
        LibretroDroid.setFrameSpeed(frameSpeed)
        val rect = RectF(viewport)
        LibretroDroid.setViewport(rect.left, rect.top, rect.width(), rect.height())
        LibretroDroid.setRumbleEnabled(config.rumbleEventsEnabled)
        LibretroDroid.setViewportAlignment(viewportAlignment.value)
        when (val game = config.game) {
            is GameSource.Path -> LibretroDroid.loadGameFromPath(game.path)
            is GameSource.Bytes -> LibretroDroid.loadGameFromBytes(game.bytes)
            is GameSource.VirtualFiles -> LibretroDroid.loadGameFromVirtualFiles(
                game.files.map { DetachedVirtualFile(it.virtualPath, it.fileDescriptor.detachFd()) }
            )
        }
        config.saveRAMState?.let { LibretroDroid.unserializeSRAM(it) }
        // TODO The first save loading could be loaded here.
        LibretroDroid.onContextCreated()
    }

    private fun renderFirstFrame(eglManager: EGLManager) {
        val surface = eglManager.eglSpec.eglCreatePBufferSurface(
            requireNotNull(eglManager.eglConfig),
            EGLConfigAttributes {
                EGL14.EGL_WIDTH to 1
                EGL14.EGL_HEIGHT to 1
            }
        )
        check(surface != EGL14.EGL_NO_SURFACE) { "Cannot create the initialization EGL surface" }
        try {
            check(eglManager.makeCurrent(surface)) { "Cannot bind the initialization EGL surface" }
            LibretroDroid.onSurfaceChanged(1, 1)
            LibretroDroid.setAudioEnabled(false)
            LibretroDroid.step(nativeCallbacks)
        } finally {
            LibretroDroid.setAudioEnabled(audioEnabled)
            eglManager.makeCurrent(eglManager.defaultSurface)
            eglManager.eglSpec.eglDestroySurface(surface)
        }
    }

    private fun render() {
        if (!isResumed || framePending) return
        framePending = true
        renderer.execute {
            framePending = false
            if (isResumed) renderTarget?.requestRender()
        }
    }

    @MainThread
    private fun stop() {
        synchronized(sessionLock) {
            renderTarget = null
            renderer.stop(false)
        }
    }

    private fun fail(error: Throwable) {
        if (failure != null) return
        failure = error
        isResumed = false
        Log.e("LibretroDroidSession", "Session failed", error)
        errorFlow.tryEmit((error as? RetroException)?.errorCode ?: LibretroDroid.ERROR_GENERIC)
        runOnUIThread { stop() }
    }

    fun <T> dispatch(block: () -> T): Deferred<T> {
        val result = CompletableDeferred<T>()
        val command = Runnable {
            failure?.let {
                result.completeExceptionally(IllegalStateException("Session failed", it))
                return@Runnable
            }
            val blockResult = runCatching(block)
                .onFailure { error ->
                    if (error is RetroException) {
                        fail(error)
                    } else {
                        Log.e("LibretroDroidSession", "Command failed", error)
                    }
                }
            result.completeWith(blockResult)
        }
        if (Thread.currentThread() === ownerThread) {
            command.run()
        } else synchronized(sessionLock) {
            if (renderer.isRunning()) {
                renderer.execute(command)
            } else {
                result.completeExceptionally(IllegalStateException("Session is not running"))
            }
        }
        return result
    }

    companion object {
        private val sessionLock = Any()
        private var activeSession: LibretroDroidSession? = null
    }
}
