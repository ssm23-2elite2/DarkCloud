package com.android.systemui;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.provider.Settings;
import android.view.Window;
import android.view.WindowManager;

public class DCloudAlert extends Activity {
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		requestWindowFeature(Window.FEATURE_NO_TITLE);      
		getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
		getWindow().setFlags(WindowManager.LayoutParams.FLAG_BLUR_BEHIND, 
				WindowManager.LayoutParams.FLAG_BLUR_BEHIND);

		// 다이얼로그 연습
		AlertDialog.Builder alt_bld = new AlertDialog.Builder(this);
		alt_bld.setMessage("인터넷이 연결되어 있지 않습니다.\n인터넷 연결설정으로 이동하겠습니까?").setCancelable(false)
		.setPositiveButton("예", new DialogInterface.OnClickListener() {
			public void onClick(DialogInterface dialog, int id) {
				startActivity(new Intent(Settings.ACTION_WIFI_SETTINGS));
				finish();
			}
		}).setNegativeButton("아니오", new DialogInterface.OnClickListener() {
			public void onClick(DialogInterface dialog, int id) {
				dialog.cancel();
				
				Intent i = new Intent("DCLOUD");
				i.putExtra("KEY", "TURNON");
				i.putExtra("MSG", "0");
				SystemUI.mContext.startService(i);
				
				finish();
			}
		});
		AlertDialog alert = alt_bld.create();
		alert.setTitle("Dark Cloud");
		alert.show();
	}
}
