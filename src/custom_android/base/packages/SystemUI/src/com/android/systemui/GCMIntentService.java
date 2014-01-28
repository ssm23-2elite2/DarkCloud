package com.android.systemui;

import org.apache.http.HttpEntity;
import org.apache.http.HttpResponse;
import org.apache.http.client.HttpClient;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.impl.client.DefaultHttpClient;
import org.apache.http.util.EntityUtils;

import android.accounts.Account;
import android.accounts.AccountManager;
import android.app.Notification;
import android.app.NotificationManager;
import android.content.Context;
import android.content.Intent;
import android.provider.Settings;
import android.util.Log;

import com.google.android.gcm.GCMBaseIntentService;

/**
 * GCM 서비스 처리기
 * @author JHG
 *
 */
public class GCMIntentService extends GCMBaseIntentService {
	public static final int NOTIFICATION_ID = 5526;
	private DCloudNative nat = new DCloudNative();
	
	public GCMIntentService() {this("796582874100"); }
	public GCMIntentService(String senderId) { 
		super(senderId);
	}

	/* 메시지 수신 시 */
	@Override
	protected void onMessage(final Context context, Intent intent) {
		String msg = intent.getStringExtra("msg");

		// 상태바에 등록 될 내용 (아이콘, Ticker 텍스트, 알림 시간)
		Notification noti = new Notification(R.drawable.ic_notify_dcloud_pressed, msg, System.currentTimeMillis());
		noti.vibrate = new long[] {300, 100, 300, 100, 300, 100};
		noti.defaults |= Notification.DEFAULT_SOUND;
		noti.flags |= Notification.FLAG_AUTO_CANCEL;

		// 알림 터치 시 실행 될 액티비티
		//Intent notifyIntent = new Intent(context, Initializer.class);
		//notifyIntent.putExtra("msg", msg);
		//notifyIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
		//PendingIntent pendingIntent = PendingIntent.getActivity(context, 0, notifyIntent, 0);

		// 상태바 펼쳤을 때 리스트에 표시 될 내용
		noti.setLatestEventInfo(context, "DarkCloud", msg, null);
		NotificationManager notiManager = (NotificationManager)context.getSystemService(Context.NOTIFICATION_SERVICE);
		notiManager.notify(NOTIFICATION_ID, noti);
	}

	/* 에러 발생 시 */
	@Override
	protected void onError(Context context, String errorId) {
	}

	/* GCM 서버에 기기 등록 시 */
	@Override
	protected void onRegistered(Context context, String regId) {
		// DCloud 서버에 가입
		AccountManager mgr = AccountManager.get(this);
		Account[] accts = mgr.getAccounts();
		Account acct = null;
		
		acct = accts[0];
		for(int i = 0; i< accts.length; i++) {
			if(accts[i].type.equals("com.google")) {
				acct = accts[i];
			}
		}
		
		// JNI User Id Setting
		nat.setUserid(acct.name);
		Log.i("TEST", "nat.setUserid 호출");
		
		Log.i("TEST", "등.록 Account - name="+acct.name+", type="+acct.type + ", key="+regId);
		Settings.System.putString(SystemUI.mContext.getContentResolver(), "DCLOUD_GCM_KEY", regId);
		Settings.System.putString(SystemUI.mContext.getContentResolver(), "DCLOUD_ACCOUNT", acct.name);
		
		try {
			HttpClient client = new DefaultHttpClient();
			HttpGet get = new HttpGet("http://211.189.20.179:3000/addUser/" + acct.name + "/" + regId);
			HttpResponse responsePOST = client.execute(get);
			HttpEntity resEntity = responsePOST.getEntity();
			Log.i("TEST", "등록결과 : " + EntityUtils.toString(resEntity));
		} 
		catch(Exception e) {
		}
		catch(Error e) {
		}
	}

	/* GCM 서버에서 기기 등록 해제 시 */
	@Override
	protected void onUnregistered(Context context, String regId) {
		Log.i("TEST", "해.제");
	}

	@Override
	protected boolean onRecoverableError(Context context, String errorId) {
		return super.onRecoverableError(context, errorId);
	}
}