package com.swordfish.libretrodroid.session

import com.swordfish.libretrodroid.ImmersiveMode
import com.swordfish.libretrodroid.ShaderConfig
import com.swordfish.libretrodroid.Variable
import com.swordfish.libretrodroid.ViewportAlignment
import com.swordfish.libretrodroid.VirtualFile

internal data class SessionConfig(
    val coreFilePath: String,
    val game: GameSource,
    val systemDirectory: String,
    val savesDirectory: String,
    val variables: Array<Variable>,
    val saveRAMState: ByteArray?,
    val shader: ShaderConfig,
    val viewportAlignment: ViewportAlignment,
    val rumbleEventsEnabled: Boolean,
    val preferLowLatencyAudio: Boolean,
    val enableMicrophone: Boolean,
    val immersiveMode: ImmersiveMode?,
    val glesVersion: Int,
    val refreshRate: Float,
    val language: String,
)

internal sealed interface GameSource {
    data class Path(val path: String) : GameSource
    data class Bytes(val bytes: ByteArray) : GameSource
    data class VirtualFiles(val files: List<VirtualFile>) : GameSource

    companion object {
        fun from(path: String?, bytes: ByteArray?, files: List<VirtualFile>): GameSource {
            require(listOf(path != null, bytes != null, files.isNotEmpty()).count { it } == 1) {
                "Exactly one game source is required"
            }
            return when {
                path != null -> Path(path)
                bytes != null -> Bytes(bytes)
                else -> VirtualFiles(files.toList())
            }
        }
    }
}
