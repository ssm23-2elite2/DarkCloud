package com.android.systemui;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.wifi.WifiManager;
import android.util.Log;

public class DCloudNetworkChecker extends BroadcastReceiver {
	public final static int WIFI_STATE_DISABLED = 0x00;
	public final static int WIFI_STATE_DISABLING = WIFI_STATE_DISABLED + 1;
	public final static int WIFI_STATE_ENABLED = WIFI_STATE_DISABLING + 1;
	public final static int WIFI_STATE_ENABLING = WIFI_STATE_ENABLED + 1;
	public final static int WIFI_STATE_UNKNOWN = WIFI_STATE_ENABLING + 1;
	public final static int NETWORK_STATE_CONNECTED = WIFI_STATE_UNKNOWN + 1;
	public final static int NETWORK_STATE_CONNECTING = NETWORK_STATE_CONNECTED + 1;
	public final static int NETWORK_STATE_DISCONNECTED = NETWORK_STATE_CONNECTING + 1;
	public final static int NETWORK_STATE_DISCONNECTING = NETWORK_STATE_DISCONNECTED + 1;
	public final static int NETWORK_STATE_SUSPENDED = NETWORK_STATE_DISCONNECTING + 1;
	public final static int NETWORK_STATE_UNKNOWN = NETWORK_STATE_SUSPENDED + 1;

	Context context;
	
	private WifiManager m_WifiManager = null;
	private ConnectivityManager m_ConnManager = null;
	
	public DCloudNetworkChecker(Context context) {
		this.context = context;
		m_WifiManager = (WifiManager)context.getSystemService(Context.WIFI_SERVICE);
		m_ConnManager = (ConnectivityManager)context.getSystemService(Context.CONNECTIVITY_SERVICE);
	}
	
	@Override
	public void onReceive(Context context, Intent intent) {
		String strAction = intent.getAction();
		
		if (strAction.equals(WifiManager.WIFI_STATE_CHANGED_ACTION)) {
			switch(m_WifiManager.getWifiState()) {
			case WifiManager.WIFI_STATE_DISABLED:
				// WIFI 꺼짐
				Intent i = new Intent("DCLOUD");
				i.putExtra("KEY", "TURNON");
				i.putExtra("MSG", "0");
				SystemUI.mContext.startService(i);
				  
				break;

			case WifiManager.WIFI_STATE_DISABLING:
				break;

			case WifiManager.WIFI_STATE_ENABLED:
				break;

			case WifiManager.WIFI_STATE_ENABLING:
				break;

			case WifiManager.WIFI_STATE_UNKNOWN:
				break;
			}
		}
		else if (strAction.equals(WifiManager.NETWORK_STATE_CHANGED_ACTION)) {
			NetworkInfo networkInfo = m_ConnManager.getNetworkInfo(ConnectivityManager.TYPE_WIFI);
			
			if ( (networkInfo != null) && (networkInfo.isAvailable() == true) ) {
				if (networkInfo.getState() == NetworkInfo.State.CONNECTED) {
					Log.i("TEST", "Connected");
				}
				else if (networkInfo.getState() == NetworkInfo.State.CONNECTING) {
					Log.i("TEST", "Connecting");
				}
				else if (networkInfo.getState() == NetworkInfo.State.DISCONNECTED) {
					Log.i("TEST", "DisConnected");
					
					// WIFI 꺼짐
					Intent i = new Intent("DCLOUD");
					i.putExtra("KEY", "TURNON");
					i.putExtra("MSG", "0");
					SystemUI.mContext.startService(i);
				}
				else if (networkInfo.getState() == NetworkInfo.State.DISCONNECTING) {
					Log.i("TEST", "DisConnecting");
				}
				else if (networkInfo.getState() == NetworkInfo.State.SUSPENDED) {
					Log.i("TEST", "Suspended");
				}
				else if (networkInfo.getState() == NetworkInfo.State.UNKNOWN) {
					Log.i("TEST", "unkwon");
				}
			}
		}
	}

}
