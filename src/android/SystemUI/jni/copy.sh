mv Android.mk. Android.mk
ndk-build
mkdir ../../../../out
mkdir ../../../../out/target
mkdir ../../../../out/target/product
mkdir ../../../../out/target/product/crespo
mkdir ../../../../out/target/product/crespo/system
mkdir ../../../../out/target/product/crespo/system/app
mkdir ../../../../out/target/product/crespo/system/lib

adb push ../obj/local/armeabi-v7a/darkcloud /data/local/tmp/
adb push ../obj/local/armeabi-v7a/libdarkcloudv1.so /sdcard/
adb push umount /data/local/tmp/
adb push userid /data/local/tmp/
mv Android.mk Android.mk.

cat copy.sh
# adb shell 
# su
# mount -o rw,remount -t yaffs2 /dev/block/mtdblock3 /system
# cat /sdcard/libdarkcloudv1.so > /system/lib/libdarkcloudv1.so

