/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.systemui.statusbar.policy;

import java.util.ArrayList;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.util.Log;
import android.view.View;
import android.widget.ImageView;

import com.android.systemui.R;

public class DcloudController extends BroadcastReceiver {
	private Context mContext;
	private ArrayList<ImageView> mIconViews = new ArrayList<ImageView>();

	public DcloudController(Context context) {
		mContext = context;

		IntentFilter filter = new IntentFilter();
		filter.addAction("DCLOUD_BR");
		context.registerReceiver(this, filter);
	}

	public void addIconView(ImageView v) {
		mIconViews.add(v);
	}

	public void onReceive(Context context, Intent intent) {
		final String action = intent.getAction();
		
		if (action.equals("DCLOUD_BR")) {
			boolean running = intent.getBooleanExtra("MODE", false);
			Log.i("TEST", "JK_BR_MODE : " + running);
			
			ImageView v = mIconViews.get(0);
			v.setImageResource(R.drawable.stat_sys_dcloud);
			if(!running)	v.setVisibility(View.GONE);
			else			v.setVisibility(View.VISIBLE);
		}
	}
}
