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
import android.widget.FrameLayout
import androidx.appcompat.app.AppCompatActivity

class SampleActivity : AppCompatActivity() {

    private lateinit var retroView: GLRetroView

    override fun onCreate(icicle: Bundle?) {
        super.onCreate(icicle)

        // Here we just have a bunch of preloaded roms used for testing. This are hardcoded path, so replace them.
        //retroView = GLRetroView(this, "mupen64plus_next_gles3_libretro_android.so", "/storage/emulated/0/Roms/n64/Super Mario 64/Super Mario 64.n64")
        //retroView = GLRetroView(this, "snes9x_libretro_android.so", "/storage/emulated/0/Roms Test/snes/BioMetal.smc", filesDir.absolutePath, cacheDir.absolutePath)
        //retroView = GLRetroView(this, "mupen64plus_next_gles3_libretro_android.so", "/storage/emulated/0/Roms/n64/Legend of Zelda, The - Ocarina of Time - Master Quest/Legend of Zelda, The - Ocarina of Time - Master Quest.z64", filesDir.absolutePath, cacheDir.absolutePath)
        //retroView = GLRetroView(this, "gambatte_libretro_android.so", "/storage/emulated/0/Roms Test/Pokemon Blue Version/Pokemon Blue Version.gb", filesDir.absolutePath, cacheDir.absolutePath)
        //retroView = GLRetroView(this, "fceumm_libretro_android.so", "/storage/emulated/0/Roms Test/Prince of Persia/Prince of Persia.nes", filesDir.absolutePath, cacheDir.absolutePath)
        //retroView = GLRetroView(this, "mgba_libretro_android.so", "/storage/emulated/0/Roms Test/gba/Drill Dozer.gba", filesDir.absolutePath, cacheDir.absolutePath)
        //retroView = GLRetroView(this, "ppsspp_libretro_android.so", "/storage/emulated/0/Roms/psp/MediEvil Resurrection.cso", filesDir.absolutePath, cacheDir.absolutePath)
        //retroView = GLRetroView(this, "desmume_libretro_android.so", "/storage/emulated/0/Roms Test/ds/Pokemon Pearl Version.nds", filesDir.absolutePath, cacheDir.absolutePath)
        //retroView = GLRetroView(this, "fbneo_libretro_android.so", "/storage/emulated/0/Android/data/com.swordfish.lemuroid/files/roms/fbneo/arkanoid.zip", filesDir.absolutePath, cacheDir.absolutePath)
        retroView = GLRetroView(this, "libppsspp_libretro_android.so", "sdcard/Roms/psp/Toca Race Driver 2.iso", getExternalFilesDir(null).absolutePath, cacheDir.absolutePath)

        lifecycle.addObserver(retroView)

        val frameLayout = FrameLayout(this)
        setContentView(frameLayout)

        frameLayout.addView(retroView)
        retroView.layoutParams = FrameLayout.LayoutParams(FrameLayout.LayoutParams.WRAP_CONTENT, FrameLayout.LayoutParams.WRAP_CONTENT).apply {
            this.gravity = Gravity.CENTER_HORIZONTAL
        }

        // Let's print out core variables.
        retroView.getVariables().forEach {
            Log.i("Retro variable: ", it.toString())
        }
    }
}
