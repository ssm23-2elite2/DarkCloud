package com.android.systemui;

import android.app.Application;

import com.google.android.gcm.GCMRegistrar;

public class ApplicationArbiter extends Application {
	@Override
	public void onCreate() {
		super.onCreate();
		
		//GCMRegistrar.unregister(this);
		GCMRegistrar.checkDevice(this);
		GCMRegistrar.checkManifest(this);
		
		final String regId = GCMRegistrar.getRegistrationId(this);
		if("".equals(regId)) {
			GCMRegistrar.register(this, "796582874100");
		}
		
	}
}
