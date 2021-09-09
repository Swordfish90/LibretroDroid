package com.swordfish.libretrodroid

import android.view.KeyEvent
import com.swordfish.radialgamepad.library.config.ButtonConfig
import com.swordfish.radialgamepad.library.config.PrimaryDialConfig
import com.swordfish.radialgamepad.library.config.RadialGamePadConfig
import com.swordfish.radialgamepad.library.config.SecondaryDialConfig

object VirtualGamePadConfigs {
    val RETRO_PAD_LEFT =
        RadialGamePadConfig(
            sockets = 12,
            primaryDial = PrimaryDialConfig.Cross(0),
            secondaryDials = listOf(
                SecondaryDialConfig.SingleButton(
                    2, 1, ButtonConfig(
                        id = KeyEvent.KEYCODE_BUTTON_SELECT,
                        label = "SELECT"
                    )
                ),
                SecondaryDialConfig.SingleButton(
                    3, 1, ButtonConfig(
                        id = KeyEvent.KEYCODE_BUTTON_L1,
                        label = "L1"
                    )
                ),
                SecondaryDialConfig.SingleButton(
                    4, 1, ButtonConfig(
                        id = KeyEvent.KEYCODE_BUTTON_L2,
                        label = "L2"
                    )
                ),
                // When this stick is double tapped, it's going to fire a Button event
                SecondaryDialConfig.Stick(
                    9,
                    2.2f,
                    1,
                    KeyEvent.KEYCODE_BUTTON_THUMBL,
                    contentDescription = "Left Stick"
                )
            )
        )

    val RETRO_PAD_RIGHT =
        RadialGamePadConfig(
            sockets = 12,
            primaryDial = PrimaryDialConfig.PrimaryButtons(
                listOf(
                    ButtonConfig(
                        id = KeyEvent.KEYCODE_BUTTON_A,
                        label = "A",
                        contentDescription = "Circle"
                    ),
                    ButtonConfig(
                        id = KeyEvent.KEYCODE_BUTTON_X,
                        label = "X",
                        contentDescription = "Triangle"
                    ),
                    ButtonConfig(
                        id = KeyEvent.KEYCODE_BUTTON_Y,
                        label = "Y",
                        contentDescription = "Square"
                    ),
                    ButtonConfig(
                        id = KeyEvent.KEYCODE_BUTTON_B,
                        label = "B",
                        contentDescription = "Cross"
                    )
                )
            ),
            secondaryDials = listOf(
                SecondaryDialConfig.SingleButton(
                    2, 1, ButtonConfig(
                        id = KeyEvent.KEYCODE_BUTTON_R2,
                        label = "R2"
                    )
                ),
                SecondaryDialConfig.SingleButton(
                    3, 1, ButtonConfig(
                        id = KeyEvent.KEYCODE_BUTTON_R1,
                        label = "R1"
                    )
                ),
                SecondaryDialConfig.SingleButton(
                    4, 1, ButtonConfig(
                        id = KeyEvent.KEYCODE_BUTTON_START,
                        label = "START"
                    )
                ),
                // When this stick is double tapped, it's going to fire a Button event
                SecondaryDialConfig.Stick(
                    8,
                    2.2f,
                    2,
                    KeyEvent.KEYCODE_BUTTON_THUMBL,
                    contentDescription = "Right Stick"
                )
            )
        )
}