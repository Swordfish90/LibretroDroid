package com.swordfish.libretrodroid

import android.content.Context
import android.opengl.GLSurfaceView
import android.widget.FrameLayout
import kotlin.math.abs

open class AspectRatioGLSurfaceView(context: Context) : GLSurfaceView(context) {
    companion object {
        /**
         * Either the width or height is decreased to obtain the desired aspect ratio.
         */
        const val RESIZE_MODE_FIT = 0
        /**
         * The width is fixed and the height is increased or decreased to obtain the desired aspect ratio.
         */
        const val RESIZE_MODE_FIXED_WIDTH = 1
        /**
         * The height is fixed and the width is increased or decreased to obtain the desired aspect ratio.
         */
        const val RESIZE_MODE_FIXED_HEIGHT = 2
        /**
         * The specified aspect ratio is ignored.
         */
        const val RESIZE_MODE_FILL = 3

        /**
         * The [FrameLayout] will not resize itself if the fractional difference between its natural
         * aspect ratio and the requested aspect ratio falls below this threshold.
         *
         *
         * This tolerance allows the view to occupy the whole of the screen when the requested aspect
         * ratio is very close, but not exactly equal to, the aspect ratio of the screen. This may reduce
         * the number of view layers that need to be composited by the underlying system, which can help
         * to reduce power consumption.
         */
        private const val MAX_ASPECT_RATIO_DEFORMATION_FRACTION = 0.01f
    }

    private var videoAspectRatio = 0f
    private var resizeMode = RESIZE_MODE_FIT

    /**
     * Set the aspect ratio that this view should satisfy.
     *
     * @param widthHeightRatio The width to height ratio.
     */
    fun setAspectRatio(widthHeightRatio: Float) {
        if (abs(videoAspectRatio - widthHeightRatio) > MAX_ASPECT_RATIO_DEFORMATION_FRACTION) {
            videoAspectRatio = widthHeightRatio
            requestLayout()
        }
    }

    /**
     * Sets the resize mode.
     *
     * @param resizeMode The resize mode.
     */
    fun setResizeMode(resizeMode: Int) {
        if (this.resizeMode != resizeMode) {
            this.resizeMode = resizeMode
            requestLayout()
        }
    }

    override fun onMeasure(widthMeasureSpec: Int, heightMeasureSpec: Int) {
        super.onMeasure(widthMeasureSpec, heightMeasureSpec)
        if (resizeMode == RESIZE_MODE_FILL || videoAspectRatio <= 0) { // Aspect ratio not set.
            return
        }
        
        var width = measuredWidth
        var height = measuredHeight
        val viewAspectRatio = width.toFloat() / height
        val aspectDeformation: Float = videoAspectRatio / viewAspectRatio - 1
        if (Math.abs(aspectDeformation) <= MAX_ASPECT_RATIO_DEFORMATION_FRACTION) { // We're within the allowed tolerance.
            return
        }
        when (resizeMode) {
            RESIZE_MODE_FIXED_WIDTH -> height = (width / videoAspectRatio).toInt()
            RESIZE_MODE_FIXED_HEIGHT -> width = (height * videoAspectRatio).toInt()
            else -> if (aspectDeformation > 0) {
                height = (width / videoAspectRatio).toInt()
            } else {
                width = (height * videoAspectRatio).toInt()
            }
        }
        super.onMeasure(MeasureSpec.makeMeasureSpec(width, MeasureSpec.EXACTLY),
                MeasureSpec.makeMeasureSpec(height, MeasureSpec.EXACTLY))
    }
}
