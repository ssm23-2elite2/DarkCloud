su -c mount -o rw,remount -t yaffs2 /dev/block/mtdblock3 /system
su -c cat /sdcard/libdarkcloudv1.so > /system/lib/libdarkcloudv1.so

