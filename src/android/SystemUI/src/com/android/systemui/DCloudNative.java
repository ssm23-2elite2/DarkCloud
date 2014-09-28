package com.android.systemui;

public class DCloudNative {
	static {
		System.loadLibrary("darkcloudv1");
	}
	
	public native int mountDarkCloud(String ipaddr);
	public native int unmountDarkCloud();
	public native int setUserid(String juserid);
}
