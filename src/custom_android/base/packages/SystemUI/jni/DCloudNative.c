#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <jni.h>

JNIEXPORT jint Java_com_android_systemui_DCloudNative_mountDarkCloud(JNIEnv* env, jobject thiz)
{
	int ret;
	ret = system("su -c ./data/local/tmp/darkcloud /sdcard/darkcloud");
	
    return (jint)ret;
}

JNIEXPORT jint Java_com_android_systemui_DCloudNative_unmountDarkCloud(JNIEnv* env, jobject thiz)
{
	int ret;
	ret = system("su -c umount /sdcard/darkcloud");
	
    return (jint)ret;
}
