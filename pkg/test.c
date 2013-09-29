/*************************************************************************
    > File Name: test.c
    > Author: onerhao
    > Mail: haodu@hustunique.com
    > Created Time: Sat 24 Aug 2013 11:51:49 PM CST
 ************************************************************************/

#include "tempsensor.h"
//#include "tempsensor.c"
//#include "tempsensor.cgi.c"
#include <ftd2xx.h>

extern char * get_argument();

int main()
{
	//time_t t=time(NULL);
	//savefile(t,23.43);
	float max_temp,min_temp;
	max_min_temperature(1379085900,1379168928,"2013-9-14.log",
			&max_temp,&min_temp);
	sprintf("max: %g,min: %g\n",max_temp,min_temp);
	return 0;
}

