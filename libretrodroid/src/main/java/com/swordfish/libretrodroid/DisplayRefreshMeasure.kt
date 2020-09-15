package com.swordfish.libretrodroid

import android.content.Context
import android.view.Choreographer
import android.view.WindowManager
import com.jakewharton.rxrelay2.PublishRelay
import io.reactivex.Observable
import kotlin.math.roundToLong

class DisplayRefreshMeasure(context: Context) {
    val framesRelay = PublishRelay.create<Long>()
    val choreographer = Choreographer.getInstance()
    val defaultRefreshRate = getScreenRefreshRate(context)

    fun callback(time: Long) {
        framesRelay.accept(time)

        choreographer.postFrameCallbackDelayed(this::callback, (500.0 / defaultRefreshRate).roundToLong())
    }


    private fun getScreenRefreshRate(context: Context): Float {
        return (context.getSystemService(Context.WINDOW_SERVICE) as WindowManager).defaultDisplay.refreshRate
    }

    init {
        choreographer.postFrameCallback(this::callback)
    }

    fun getEstimatedRefreshRate(): Observable<Double> {
        return framesRelay
            .buffer(2, 1)
            .map { 1000000000.0 / (it[1] - it[0]).toDouble() }
            .scan(defaultRefreshRate.toDouble()) { current: Double, new: Double ->
                current * 0.9 + new * 0.1
            }
    }
}