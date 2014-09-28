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

package com.android.settings;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.NetworkInfo;
import android.net.wifi.SupplicantState;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.provider.Settings;
import android.util.Log;
import android.view.View;
import android.widget.CompoundButton;
import android.widget.ImageView;
import android.widget.Switch;
import android.widget.Toast;

import com.android.settings.R;
import com.android.settings.WirelessSettings;

/**
 * DarkCloudModeEnabler is a helper to manage the DarkCloud on/off checkbox
 * preference. It turns on/off DarkCloud and ensures the summary of the
 * preference reflects the current state.
 */
public final class DarkCloudModeEnabler implements CompoundButton.OnCheckedChangeListener {
	private final Context mContext;
	private Switch mSwitch;
	private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
		@Override

		public void onReceive(Context context, Intent intent) {
			String action = intent.getAction();

			if (action.equals("DCLOUD_BR")) {
				boolean running = intent.getBooleanExtra("MODE", false);
				if(!running)	mSwitch.setChecked(false);
				else			mSwitch.setChecked(true);
			}
		}
	};

	public DarkCloudModeEnabler(Context context, Switch switch_) {
		mContext = context;
		mSwitch = switch_;
	}

	public void resume() {
		IntentFilter filter = new IntentFilter();
		filter.addAction("DCLOUD_BR");
		mContext.registerReceiver(mReceiver, filter);
		mSwitch.setOnCheckedChangeListener(this);
	}

	public void pause() {
		mContext.unregisterReceiver(mReceiver);
		mSwitch.setOnCheckedChangeListener(null);
	}

	public void setSwitch(Switch switch_) {
		if (mSwitch == switch_) return;
		mSwitch.setOnCheckedChangeListener(null);
		mSwitch = switch_;
		mSwitch.setOnCheckedChangeListener(this);

		boolean c = Settings.System.getInt(mContext.getContentResolver(), "DCLOUD_MODE_ON", 0) != 0;
		mSwitch.setChecked(c);
	}

	public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
		boolean c = Settings.System.getInt(mContext.getContentResolver(), "DCLOUD_MODE_ON", 0) != 0;
		if(c != isChecked) {
			// Dcloud 서비스 메세지 전달
			Intent i = new Intent("DCLOUD");
			i.putExtra("KEY", "TURNON");
			i.putExtra("MSG", (c==true?"0":"1"));
			mContext.startService(i);
		}
	}
}
