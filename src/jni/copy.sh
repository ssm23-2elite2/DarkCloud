mv /home/jake/Workspace/framework/frameworks/base/packages/SystemUI/jni/Android.mk. /home/jake/Workspace/framework/frameworks/base/packages/SystemUI/jni/Android.mk
#ndk-build clean
ndk-build
#mkdir /home/jake/Workspace/framework/out
#mkdir /home/jake/Workspace/framework/out/target
#mkdir /home/jake/Workspace/framework/out/target/product
#mkdir /home/jake/Workspace/framework/out/target/product/crespo
#mkdir /home/jake/Workspace/framework/out/target/product/crespo/system
#mkdir /home/jake/Workspace/framework/out/target/product/crespo/system/app
#mkdir /home/jake/Workspace/framework/out/target/product/crespo/system/lib

adb push /home/jake/Workspace/framework/frameworks/base/packages/SystemUI/obj/local/armeabi-v7a/darkcloud /data/local/tmp/
adb push umount /data/local/tmp/
#adb push /home/jake/Workspace/framework/frameworks/base/packages/SystemUI/libs/armeabi-v7a/darkcloud /data/local/tmp/


cp /home/jake/Workspace/framework/frameworks/base/packages/SystemUI/obj/local/armeabi-v7a/libdarkcloudv1.so /home/jake/Workspace/framework/out/target/product/crespo/system/lib/libdarkcloudv1.so
adb push /home/jake/Workspace/framework/frameworks/base/packages/SystemUI/obj/local/armeabi-v7a/libdarkcloudv1.so /sdcard/
#adb push /home/jake/Workspace/framework/frameworks/base/packages/SystemUI/libs/armeabi-v7a/libdarkcloudv1.so /sdcard/
mv /home/jake/Workspace/framework/frameworks/base/packages/SystemUI/jni/Android.mk /home/jake/Workspace/framework/frameworks/base/packages/SystemUI/jni/Android.mk.

# adb shell 
# su
# mount -o rw,remount -t yaffs2 /dev/block/mtdblock3 /system
# cat /sdcard/libdarkcloudv1.so > /system/lib/libdarkcloudv1.so

