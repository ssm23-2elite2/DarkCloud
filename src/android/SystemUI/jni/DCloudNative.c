#include <android/log.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <jni.h>

#define LOG_TAG "DCLOUD"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

JNIEXPORT jint Java_com_android_systemui_DCloudNative_setUserid(JNIEnv* env, jobject thiz, jstring juserid)
{
	const char *userid = (*env)->GetStringUTFChars(env, juserid, 0);
	int ret = 0;
	FILE *fp;

	fp = fopen("/data/local/tmp/userid", "w");
	
	if(fp == NULL) {
		LOGE("/data/local/tmp/userid Open Failed[%d]", errno);
		LOGE("/data/local/tmp/userid Open Failed[%s]", strerror(errno));
		ret = -1;
	} else {
		fprintf(fp, "%s", userid);
		fclose(fp);
		ret = 0;
	}
	LOGD("ret(%d) userid(%s)", ret, userid);

	(*env)->ReleaseStringUTFChars(env, juserid, userid);
    return (jint)ret;
}

JNIEXPORT jint Java_com_android_systemui_DCloudNative_mountDarkCloud(JNIEnv* env, jobject thiz, jstring jipaddr)
{
	int ret;
	const char *ipaddr = (*env)->GetStringUTFChars(env, jipaddr, 0);
	char cmd[200];
	ret = system("su -c rm -r /sdcard/darkcloud/*");
	if(ret != 0 && ret != errno) {
		LOGE("rm Error [%s]", strerror(errno));
	}
	sprintf(cmd, "su -c ./data/local/tmp/darkcloud /sdcard/darkcloud -oallow_other -obig_writes %s", ipaddr);
	LOGD("%s", cmd);
	ret = system(cmd);
	if(ret != 0) {
		LOGE("mount Error [%s]", strerror(errno));
	}
	
    return (jint)ret;
}

JNIEXPORT jint Java_com_android_systemui_DCloudNative_unmountDarkCloud(JNIEnv* env, jobject thiz)
{
	int ret;
	ret = system("su -c chmod 755 ./data/local/tmp/umount");
	ret = system("su -c ./data/local/tmp/umount /sdcard/darkcloud -l");
	if(ret != 0) {
		LOGE("umount Error [%s]", strerror(errno));
	}
	
    return (jint)ret;
}
