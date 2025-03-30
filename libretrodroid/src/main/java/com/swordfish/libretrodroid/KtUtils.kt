/*
 *     Copyright (C) 1  Filippo Scognamiglio
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

import android.os.Handler
import android.os.Looper
import android.util.Log
import java.util.concurrent.CountDownLatch

object KtUtils {
    fun runOnUIThread(runnable: () -> Unit) {
        try {
            if (isUIThread()) {
                runnable()
            } else {
                Handler(Looper.getMainLooper()).post(runnable)
            }
        } catch (e: Exception) {
            Log.d("DQC", "runOnUIThread: ${e.message}")
        }
    }

    private fun isUIThread(): Boolean {
        return try {
            if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.M) {
                Looper.getMainLooper().isCurrentThread
            } else {
                Thread.currentThread() == Looper.getMainLooper().thread
            }
        } catch (e: Exception) {
            false
        }
    }

    fun CountDownLatch.awaitUninterruptibly() {
        try {
            while (true) {
                try {
                    await()
                    return
                } catch (e: InterruptedException) {

                }
            }
        } catch (e: Exception) {
            Log.d("DQC", "awaitUninterruptibly: ${e.message}")
        } finally {

        }
    }
}