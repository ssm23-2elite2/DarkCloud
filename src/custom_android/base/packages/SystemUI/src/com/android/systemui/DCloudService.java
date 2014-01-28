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

package com.android.systemui;

import java.util.ArrayList;
import java.util.List;

import org.apache.http.HttpEntity;
import org.apache.http.HttpResponse;
import org.apache.http.client.HttpClient;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.conn.ClientConnectionManager;
import org.apache.http.impl.client.DefaultHttpClient;
import org.apache.http.impl.conn.tsccm.ThreadSafeClientConnManager;
import org.apache.http.params.HttpParams;
import org.apache.http.util.EntityUtils;

import android.app.PendingIntent;
import android.app.PendingIntent.CanceledException;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.Uri;
import android.net.wifi.WifiManager;
import android.os.Environment;
import android.os.IBinder;
import android.provider.Settings;
import android.util.Log;

public class DCloudService extends Service {
	private static HttpClient client;
	private DCloudNative nat = new DCloudNative();
	private boolean running = false;
	
	@Override
	public IBinder onBind(Intent intent) {
		return null;
	}

	@Override
	public void onCreate() {
		// 핸드폰이 켜지자마자 실행(꺼짐 Default)
		Settings.System.putInt(SystemUI.mContext.getContentResolver(), "DCLOUD_MODE_ON", 0);
		
		// 인터넷 변화 감지
		DCloudNetworkChecker checker = new DCloudNetworkChecker(SystemUI.mContext);
		IntentFilter receiverFilter = new IntentFilter();
		receiverFilter.addAction(WifiManager.WIFI_STATE_CHANGED_ACTION);
		receiverFilter.addAction(WifiManager.NETWORK_STATE_CHANGED_ACTION);
		registerReceiver(checker, receiverFilter);
		 
		/*
		// 실행여부 파악
		running = Settings.System.getInt(SystemUI.mContext.getContentResolver(), "DCLOUD_MODE_ON", 0) != 0;

		// BR 방송! (status_bar를 위해서)
		Intent intent = new Intent();
		intent.setAction("DCLOUD_BR");
		intent.putExtra("MODE", running);
		SystemUI.mContext.sendBroadcast(intent);
		 */

		super.onCreate();
	}

	@Override
	public int onStartCommand(Intent intent, int flags, int startId) {
		String KEY = intent.getStringExtra("KEY");
		String MSG = intent.getStringExtra("MSG");

		if(KEY != null && MSG != null) {
			if(KEY.equals("TURNON")) {
				// ON / OFF 기능
				boolean isChecked = MSG.equals("1");


				if(isChecked) {
					// ON
					if(isConnectedToInternet()) {
						// 인터넷이 사용가능하다면 마운트
						int ret = nat.mountDarkCloud();
						if(ret == 0) {
							Settings.System.putInt(SystemUI.mContext.getContentResolver(), "DCLOUD_MODE_ON", isChecked ? 1 : 0);
							Log.i("TEST", "JK_KEY = " + KEY + ", MSG = " + MSG + "INTERNET_available : " + isConnectedToInternet());

							// BR 방송! (status_bar를 위해서)
							Intent intent_br = new Intent();
							intent_br.setAction("DCLOUD_BR");
							intent_br.putExtra("MODE", isChecked);
							SystemUI.mContext.sendBroadcast(intent_br);

							// 미디어 재스캔 (DCloud 폴더만)
							sendBroadcast(new Intent(Intent.ACTION_MEDIA_MOUNTED,Uri.parse("file://"+Environment.getExternalStorageDirectory()+"/darkcloud")));
						}
						Log.w("TEST", "MOUNT = " + ret);
					} else {
						// 인터넷 사용 불가능하다면
						Intent i = new Intent(SystemUI.mContext, DCloudAlert.class);
						PendingIntent p = PendingIntent.getActivity(SystemUI.mContext, 0, i, 0);
						try { p.send(); }
						catch (CanceledException e) {}
					}
				} else {
					// OFF
					int ret = nat.unmountDarkCloud();
					if(ret == 0 || ret == 256) {
						Settings.System.putInt(SystemUI.mContext.getContentResolver(), "DCLOUD_MODE_ON", isChecked ? 1 : 0);
						Log.i("TEST", "JK_KEY = " + KEY + ", MSG = " + MSG + "INTERNET_available : " + isConnectedToInternet());

						// BR 방송! (status_bar를 위해서)
						Intent intent_br = new Intent();
						intent_br.setAction("DCLOUD_BR");
						intent_br.putExtra("MODE", isChecked);
						SystemUI.mContext.sendBroadcast(intent_br);

						// 미디어 재스캔 (DCloud 폴더만)
						sendBroadcast(new Intent(Intent.ACTION_MEDIA_MOUNTED,Uri.parse("file://"+Environment.getExternalStorageDirectory()+"/darkcloud")));
					}
					Log.w("TEST", "UMOUNT = " + ret);
				}
			}
		}
		return super.onStartCommand(intent, flags, startId);
	}

	public boolean isConnectedToInternet() {
		ConnectivityManager manager = (ConnectivityManager)SystemUI.mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
		List<NetworkInfo> netInfos = new ArrayList<NetworkInfo>();
		netInfos.add(manager.getNetworkInfo(ConnectivityManager.TYPE_MOBILE));
		netInfos.add(manager.getNetworkInfo(ConnectivityManager.TYPE_WIFI));
		netInfos.add(manager.getNetworkInfo(ConnectivityManager.TYPE_WIMAX));

		for(NetworkInfo netInfo : netInfos) {
			if(netInfo != null) {
				if(netInfo.isConnected()) return true;
			}
		}
		return false;
	}

	public static HttpClient getHttpClient() {
		if (client == null) {
			client = new DefaultHttpClient();
			ClientConnectionManager mgr = client.getConnectionManager();
			HttpParams params = client.getParams();
			client = new DefaultHttpClient(new ThreadSafeClientConnManager(params, mgr.getSchemeRegistry()), params);
		}
		return client;
	}

	public String InternetTest(String url) {
		try {
			HttpClient client = getHttpClient();
			HttpPost post = new HttpPost(url);
			HttpResponse responsePOST = client.execute(post);
			HttpEntity resEntity = responsePOST.getEntity();
			return EntityUtils.toString(resEntity);
		} catch (Exception e) {
			e.printStackTrace();
			return null;
		} catch (Error e) {
			e.printStackTrace();
			return null;
		}
	}
}
