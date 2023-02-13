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

sealed interface ShaderConfig {

    object Default : ShaderConfig
    object CRT : ShaderConfig
    object LCD : ShaderConfig
    object Sharp : ShaderConfig

    data class CUT(
        val sharpnessMin: Float = 0.1f,
        val sharpnessMax: Float = 0.3f
    ) : ShaderConfig

    data class CUT2(
        val sharpnessBias: Float = 1.0f,
        val sharpnessMax: Float = 1.0f,
    ) : ShaderConfig

    data class CUT3(
        val useDynamicBlend: Boolean = true,
        val blendMinContrastEdge: Float = 0.0f,
        val blendMaxContrastEdge: Float = 1.0f,
        val blendMinSharpness: Float = 0.0f,
        val blendMaxSharpness: Float = 1.0f,
        val staticSharpness: Float = 0.5f,
        val edgeUseFastLuma: Boolean = false,
        val edgeMinValue: Float = 0.03f,
        val edgeMinContrast: Float = 1.20f,
        val lumaAdjustGamma: Boolean = false,
        val splitDemoView: Boolean = false,
    ) : ShaderConfig
}
