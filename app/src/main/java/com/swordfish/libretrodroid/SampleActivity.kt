/*
 *     Copyright (C) 2019  Filippo Scognamiglio
 *
 *     This program is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 *
 *     This program is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public License
 *     along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

package com.swordfish.libretrodroid

import android.os.Bundle
import android.util.Log
import android.view.Gravity
import android.view.KeyEvent
import android.view.MotionEvent
import android.widget.FrameLayout
import androidx.appcompat.app.AppCompatActivity
import com.android.libretrodroid.R
import com.swordfish.radialgamepad.library.RadialGamePad
import com.swordfish.radialgamepad.library.event.Event
import io.reactivex.disposables.CompositeDisposable
import io.reactivex.schedulers.Schedulers

class SampleActivity : AppCompatActivity() {

    companion object {
        private val TAG_LOG = SampleActivity::class.java.simpleName
    }

    private lateinit var retroView: GLRetroView

    private lateinit var leftPad: RadialGamePad
    private lateinit var rightPad: RadialGamePad

    private val resumeDisposables = CompositeDisposable()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.sample_activity)

        /* Prepare config for GLRetroView */
        val data = GLRetroViewData(this).apply {
            /*
             * The name of the LibRetro core to load.
             * The typical location that libraries should be stored in is
             * app/src/main/jniLibs/<ABI>/
             *
             * ABI can be arm64-v8a, armeabi-v7a, x86, or x86_64
             */
            coreFilePath = "libmgba_libretro_android.so"

            /*
             * The path to the ROM to load.
             * Example: /data/data/<package-id>/files/example.gba
             */
            gameFilePath = "${filesDir}/example.gba"

            /*
             * Direct ROM bytes to load.
             * This is mutually exclusive with gameFilePath.
             */
            gameFileBytes = null

            /* (Optional) System directory */
            systemDirectory = filesDir.absolutePath

            /* (Optional) Save file directory */
            savesDirectory = filesDir.absolutePath

            /* (Optional) Variables to give the LibRetro core */
            variables = arrayOf()

            /*
            * (Optional) SRAM state to deserialize upon init.
            * When games save their data, they store it in their SRAM.
            * It is necessary to preserve the SRAM upon closing the app
            * in order to load it again later.
            *
            * The SRAM can be serialized to a ByteArray via serializeSRAM().
            */
            saveRAMState = null

            /*
             * (Optional) Shader to apply to the view.
             *
             * SHADER_DEFAULT:      Bilinear filtering, can cause fuzziness in retro games.
             * SHADER_CRT:          Classic CRT scan lines.
             * SHADER_LCD:          Grid layout, similar to Nintendo DS bottom screens.
             * SHADER_SHARP:        Raw, unfiltered image.
             */
            shader = LibretroDroid.SHADER_DEFAULT

            /* Rumble events enabled */
            rumbleEventsEnabled = true

            /* Use low-latency audio on supported devices */
            preferLowLatencyAudio = true
        }

        /* Initialize the main emulator view */
        retroView = GLRetroView(this, data)

        lifecycle.addObserver(retroView)

        /* Get the FrameLayout to house the GLRetroView */
        val frameLayout = findViewById<FrameLayout>(R.id.gamecontainer)

        /* Add and center the GLRetroView */
        frameLayout.addView(retroView)
        retroView.layoutParams = FrameLayout.LayoutParams(
                FrameLayout.LayoutParams.WRAP_CONTENT,
                FrameLayout.LayoutParams.WRAP_CONTENT
        ).apply {
            gravity = Gravity.CENTER_HORIZONTAL
        }

        initializeVirtualGamePad()
    }

    /* Pipe motion events to the GLRetroView */
    override fun onGenericMotionEvent(event: MotionEvent?): Boolean {
        if (event != null) {
            sendMotionEvent(
                event,
                GLRetroView.MOTION_SOURCE_DPAD,
                MotionEvent.AXIS_HAT_X,
                MotionEvent.AXIS_HAT_Y,
                0
            )
            sendMotionEvent(
                event,
                GLRetroView.MOTION_SOURCE_ANALOG_LEFT,
                MotionEvent.AXIS_X,
                MotionEvent.AXIS_Y,
                0
            )
            sendMotionEvent(
                event,
                GLRetroView.MOTION_SOURCE_ANALOG_RIGHT,
                MotionEvent.AXIS_Z,
                MotionEvent.AXIS_RZ,
                0
            )
        }
        return super.onGenericMotionEvent(event)
    }

    /*
     * Pipe hardware key events to the GLRetroView.
     *
     * WARNING: This method can override volume key events.
     */
    override fun onKeyDown(keyCode: Int, event: KeyEvent): Boolean {
        retroView.sendKeyEvent(event.action, keyCode)
        return super.onKeyDown(keyCode, event)
    }

    override fun onKeyUp(keyCode: Int, event: KeyEvent): Boolean {
        retroView.sendKeyEvent(event.action, keyCode)
        return super.onKeyUp(keyCode, event)
    }

    private fun sendMotionEvent(
            event: MotionEvent,
            source: Int,
            xAxis: Int,
            yAxis: Int,
            port: Int
    ) {
        retroView.sendMotionEvent(
            source,
            event.getAxisValue(xAxis),
            event.getAxisValue(yAxis),
            port
        )
    }

    override fun onResume() {
        super.onResume()
        resumeDisposables.add(
            leftPad.events()
                .subscribe { handleEvent(it) }
        )
        resumeDisposables.add(
            rightPad.events()
                .subscribe { handleEvent(it) }
        )
        resumeDisposables.add(
            retroView.getRumbleEvents()
                .subscribeOn(Schedulers.single())
                .subscribe { handleRumbleEvent(it) }
        )
    }

    override fun onPause() {
        super.onPause()
        resumeDisposables.dispose()
    }

    private fun handleEvent(event: Event) {
        when (event) {
            is Event.Button -> retroView.sendKeyEvent(event.action, event.id)
            is Event.Direction -> retroView.sendMotionEvent(event.id, event.xAxis, event.yAxis)
        }
    }

    private fun handleRumbleEvent(rumbleEvent: RumbleEvent) {
        Log.i(TAG_LOG, "Received rumble event: $rumbleEvent")
    }

    private fun initializeVirtualGamePad() {
        leftPad = RadialGamePad(VirtualGamePadConfigs.RETRO_PAD_LEFT, 8f, this)
        rightPad = RadialGamePad(VirtualGamePadConfigs.RETRO_PAD_RIGHT, 8f,this)

        // We want the pad anchored to the bottom of the screen
        leftPad.gravityX = -1f
        leftPad.gravityY = 1f

        rightPad.gravityX = 1f
        rightPad.gravityY = 1f

        findViewById<FrameLayout>(R.id.leftcontainer).addView(leftPad)
        findViewById<FrameLayout>(R.id.rightcontainer).addView(rightPad)
    }
}
