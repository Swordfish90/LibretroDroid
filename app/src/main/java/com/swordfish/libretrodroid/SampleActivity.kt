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

import android.app.Activity
import android.os.Bundle

class SampleActivity : Activity() {

    private lateinit var retroView: GLRetroView

    override fun onCreate(icicle: Bundle?) {
        super.onCreate(icicle)

        // Here we just have a bunch of preloaded roms used for testing. This are hardcoded path, so replace them.
        retroView = GLRetroView(this, "mupen64plus_next_gles3_libretro_android.so", "/storage/emulated/0/Roms Test/n64/Super Mario 64/Super Mario 64.n64")
        //retroView = GLRetroView(this, "snes9x_libretro_android.so", "/storage/emulated/0/Roms Test/snes/BioMetal.smc", GLRetroView.SHADER_CRT)
        //retroView = GLRetroView(this, "mupen64plus_next_gles3_libretro_android.so", "/storage/emulated/0/Roms Test/n64/Legend of Zelda, The - Ocarina of Time - Master Quest/Legend of Zelda, The - Ocarina of Time - Master Quest.z64", GLRetroView.SHADER_CRT)
        //retroView = GLRetroView(this, "gambatte_libretro_android.so", "/storage/emulated/0/Roms Test/gb/Pokemon Blue Version.gb", LibretroDroid.SHADER_LCD)
        //retroView = GLRetroView(this, "gambatte_libretro_android.so", "/storage/emulated/0/Roms Test/gb/Super Mario Land.gb", GLRetroView.SHADER_LCD)
        //retroView = GLRetroView(this, "mgba_libretro_android.so", "/storage/emulated/0/Roms Test/gba/Advance Wars.gba", GLRetroView.SHADER_LCD)

        setContentView(retroView)

        retroView.onCreate()
    }

    override fun onDestroy() {
        super.onDestroy()
        retroView.onDestroy()
    }

    override fun onPause() {
        super.onPause()
        retroView.onPause()
    }

    override fun onResume() {
        super.onResume()
        retroView.onResume()
    }
}
