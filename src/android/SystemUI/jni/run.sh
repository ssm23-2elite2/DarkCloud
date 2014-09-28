umount /sdcard/darkcloud
rm /sdcard/darkcloud/*
cd /data/local/tmp
./darkcloud /sdcard/darkcloud/ -oallow_other -obig_writes -d
