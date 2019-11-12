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

package com.swordfish.libretrodroid;

import android.app.Activity;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;

public class SampleActivity extends Activity {

    GLRetroView mView;

    @Override
    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        mView = new GLRetroView(getApplication());
	    setContentView(mView);
        mView.setFocusable(true);

        Log.d("FILIPPO", "Environment: " + Environment.getExternalStorageDirectory().getPath());
    }

    @Override
    protected
    void onDestroy() {
        super.onDestroy();
        mView.onDestroy();
    }

    @Override
    protected void onPause() {
        super.onPause();
        mView.onPause();
    }

    @Override
    protected void onResume() {
        super.onResume();
        mView.onResume();
    }
}
