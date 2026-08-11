#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

int main(void)
{
	char tmp[20];
	memset(tmp, 0x00, sizeof(tmp));
	double val = pow(10, -1);
	sprintf(tmp, "%.*f", 1, val);
	printf("tmp[%s]\n", tmp);
	return 0;

}
