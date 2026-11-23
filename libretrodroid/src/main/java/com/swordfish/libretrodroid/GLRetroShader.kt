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

internal data class GLRetroShader(val type: Int, val params: Map<String, String> = emptyMap())

internal fun ShaderConfig.toNativeShader(): GLRetroShader {
    return when (val config = this) {
        is ShaderConfig.Default -> GLRetroShader(LibretroDroid.SHADER_DEFAULT)
        is ShaderConfig.CRT -> GLRetroShader(LibretroDroid.SHADER_CRT)
        is ShaderConfig.LCD -> GLRetroShader(LibretroDroid.SHADER_LCD)
        is ShaderConfig.Sharp -> GLRetroShader(LibretroDroid.SHADER_SHARP)
        is ShaderConfig.CUT -> GLRetroShader(
            LibretroDroid.SHADER_UPSCALE_CUT,
            mapOf(
                LibretroDroid.SHADER_UPSCALE_CUT_PARAM_USE_DYNAMIC_BLEND to toParam(config.useDynamicBlend),
                LibretroDroid.SHADER_UPSCALE_CUT_PARAM_BLEND_MIN_CONTRAST_EDGE to toParam(config.blendMinContrastEdge),
                LibretroDroid.SHADER_UPSCALE_CUT_PARAM_BLEND_MAX_CONTRAST_EDGE to toParam(config.blendMaxContrastEdge),
                LibretroDroid.SHADER_UPSCALE_CUT_PARAM_BLEND_MIN_SHARPNESS to toParam(config.blendMinSharpness),
                LibretroDroid.SHADER_UPSCALE_CUT_PARAM_BLEND_MAX_SHARPNESS to toParam(config.blendMaxSharpness),
                LibretroDroid.SHADER_UPSCALE_CUT_PARAM_STATIC_BLEND_SHARPNESS to toParam(config.staticSharpness),
                LibretroDroid.SHADER_UPSCALE_CUT_PARAM_EDGE_USE_FAST_LUMA to toParam(config.edgeUseFastLuma),
                LibretroDroid.SHADER_UPSCALE_CUT_PARAM_EDGE_MIN_VALUE to toParam(config.edgeMinValue),
                LibretroDroid.SHADER_UPSCALE_CUT_PARAM_EDGE_MIN_CONTRAST to toParam(config.edgeMinContrast),
            )
        )

        is ShaderConfig.CUT2 -> GLRetroShader(
            LibretroDroid.SHADER_UPSCALE_CUT2,
            mapOf(
                LibretroDroid.SHADER_UPSCALE_CUT2_PARAM_USE_DYNAMIC_BLEND to toParam(config.useDynamicBlend),
                LibretroDroid.SHADER_UPSCALE_CUT2_PARAM_BLEND_MIN_CONTRAST_EDGE to toParam(config.blendMinContrastEdge),
                LibretroDroid.SHADER_UPSCALE_CUT2_PARAM_BLEND_MAX_CONTRAST_EDGE to toParam(config.blendMaxContrastEdge),
                LibretroDroid.SHADER_UPSCALE_CUT2_PARAM_BLEND_MIN_SHARPNESS to toParam(config.blendMinSharpness),
                LibretroDroid.SHADER_UPSCALE_CUT2_PARAM_BLEND_MAX_SHARPNESS to toParam(config.blendMaxSharpness),
                LibretroDroid.SHADER_UPSCALE_CUT2_PARAM_STATIC_BLEND_SHARPNESS to toParam(config.staticSharpness),
                LibretroDroid.SHADER_UPSCALE_CUT2_PARAM_EDGE_USE_FAST_LUMA to toParam(config.edgeUseFastLuma),
                LibretroDroid.SHADER_UPSCALE_CUT2_PARAM_SOFT_EDGES_SHARPENING to toParam(config.softEdgesSharpening),
                LibretroDroid.SHADER_UPSCALE_CUT2_PARAM_SOFT_EDGES_SHARPENING_AMOUNT to toParam(config.softEdgesSharpeningAmount),
                LibretroDroid.SHADER_UPSCALE_CUT2_PARAM_HARD_EDGES_SEARCH_MAX_ERROR to toParam(config.hardEdgesSearchMaxError),
            )
        )

        is ShaderConfig.CUT3 -> GLRetroShader(
            LibretroDroid.SHADER_UPSCALE_CUT3,
            mapOf(
                LibretroDroid.SHADER_UPSCALE_CUT3_PARAM_USE_DYNAMIC_BLEND to toParam(config.useDynamicBlend),
                LibretroDroid.SHADER_UPSCALE_CUT3_PARAM_BLEND_MIN_CONTRAST_EDGE to toParam(config.blendMinContrastEdge),
                LibretroDroid.SHADER_UPSCALE_CUT3_PARAM_BLEND_MAX_CONTRAST_EDGE to toParam(config.blendMaxContrastEdge),
                LibretroDroid.SHADER_UPSCALE_CUT3_PARAM_BLEND_MIN_SHARPNESS to toParam(config.blendMinSharpness),
                LibretroDroid.SHADER_UPSCALE_CUT3_PARAM_BLEND_MAX_SHARPNESS to toParam(config.blendMaxSharpness),
                LibretroDroid.SHADER_UPSCALE_CUT3_PARAM_STATIC_BLEND_SHARPNESS to toParam(config.staticSharpness),
                LibretroDroid.SHADER_UPSCALE_CUT3_PARAM_EDGE_USE_FAST_LUMA to toParam(config.edgeUseFastLuma),
                LibretroDroid.SHADER_UPSCALE_CUT3_PARAM_SOFT_EDGES_SHARPENING to toParam(config.softEdgesSharpening),
                LibretroDroid.SHADER_UPSCALE_CUT3_PARAM_SOFT_EDGES_SHARPENING_AMOUNT to toParam(config.softEdgesSharpeningAmount),
                LibretroDroid.SHADER_UPSCALE_CUT3_PARAM_HARD_EDGES_SEARCH_MAX_ERROR to toParam(config.hardEdgesSearchMaxError),
                LibretroDroid.SHADER_UPSCALE_CUT3_PARAM_HARD_EDGES_SEARCH_MAX_DISTANCE to toParam(config.hardEdgesSearchMaxDistance),
            )
        )
    }
}

private fun toParam(param: Float): String {
    return param.toString()
}

private fun toParam(param: Boolean): String {
    return if (param) {
        "1"
    } else {
        "0"
    }
}

private fun toParam(param: Int): String {
    return param.toString()
}
