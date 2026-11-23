/*
 *     Copyright (C) 2022  Filippo Scognamiglio
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

import android.app.ActivityManager
import android.content.Context
import android.view.WindowManager
import com.swordfish.libretrodroid.session.GameSource
import com.swordfish.libretrodroid.session.SessionConfig
import java.util.Locale

// TODO... We need to delete this and simply use the session config class
class GLRetroViewData(context: Context) {
    var coreFilePath: String? = null
    var gameFilePath: String? = null
    var gameFileBytes: ByteArray? = null
    var gameVirtualFiles: List<VirtualFile> = listOf()
    var systemDirectory: String = context.filesDir.absolutePath
    var savesDirectory: String = context.filesDir.absolutePath
    var variables: Array<Variable> = arrayOf()
    var saveRAMState: ByteArray? = null
    var shader: ShaderConfig = ShaderConfig.Default
    var viewportAlignment = ViewportAlignment.CENTER
    var rumbleEventsEnabled: Boolean = true
    var preferLowLatencyAudio: Boolean = true
    @Deprecated("Duplicate frame suppression is no longer supported")
    var skipDuplicateFrames: Boolean = false
    var enableMicrophone: Boolean = false
    var immersiveMode: ImmersiveMode? = null
}

enum class ViewportAlignment(val value: Int) {
    CENTER(0),
    TOP(1),
    BOTTOM(2)
}

internal fun GLRetroViewData.toSessionConfig(context: Context): SessionConfig {
    val corePath = requireNotNull(coreFilePath) { "A core is required" }
    require(corePath.isNotBlank()) { "A core path cannot be blank" }
    val game = GameSource.from(gameFilePath, gameFileBytes, gameVirtualFiles)
    val activityManager = context.getSystemService(Context.ACTIVITY_SERVICE) as ActivityManager
    val windowManager = context.getSystemService(Context.WINDOW_SERVICE) as WindowManager
    return SessionConfig(
        coreFilePath = corePath,
        game = game,
        systemDirectory = systemDirectory,
        savesDirectory = savesDirectory,
        variables = variables.copyOf(),
        saveRAMState = saveRAMState,
        shader = shader,
        viewportAlignment = viewportAlignment,
        rumbleEventsEnabled = rumbleEventsEnabled,
        preferLowLatencyAudio = preferLowLatencyAudio,
        enableMicrophone = enableMicrophone,
        immersiveMode = immersiveMode,
        glesVersion = if (activityManager.deviceConfigurationInfo.reqGlEsVersion >= 0x30000) 3 else 2,
        refreshRate = windowManager.defaultDisplay.refreshRate,
        language = Locale.getDefault().language,
    )
}
