#include <stdio.h>

int main(int argc, char **argv)
{
	int ret;

	ret = system("su -c ./data/local/tmp/darkcloud /sdcard/darkcloud");
	printf("ret = %d\n", ret);

	return ret;
}
