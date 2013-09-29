/*************************************************************************
    > File Name: getinfo.c
    > Author: onerhao
    > Mail: haodu@hustunique.com
    > Created Time: Sat 24 Aug 2013 12:34:50 PM CST
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <endian.h>
#include <math.h>
#include "sensor.h"

char *inquire_code[]=
{
    QUERY_SLAVE_S,
    QUERY_AD1_S,
    QUERY_AD2_S,
    QUERY_AD3_S,
    QUERY_AD4_S,
    QUERY_TEMPERATURE_S,
};

char *config_code[]=
{
    "#\x06\x01\x01\x0D", //ad 1
    "#\x06\x00\x02\x0D", //ad 2
    "#\x06\x01\x04\x0D", //ad 3
    "#\x06\x01\x07\x0D", //ad 4
};

void dumpbuffer(unsigned char *buffer,int n)
{
    int i;
    printf("[ ");
    for(i=0; i<n; i++)
    {
        if(i>0)
        {
            printf(",");
        }
        printf("0X%02X",(unsigned int)buffer[i]);
    }
    printf(" ]\n");
}

int getint(unsigned char *buffer,int *res,size_t n)
{
    //buffer to integer
    int i;
    unsigned char *p=buffer+n-1,*q=(void*)res;
    if(n>4)
        return -1;
    memset(q,0,sizeof(*res));
    for(i=0; i<n; i++)
    {
        *(char*)(q+i)=*(char*)(p-i);
    }
    return 0;
}

int getfloat(unsigned char *buffer,float *res)
{
    //buffer to float number
    int exp,a;//exponent
    float b;
    unsigned int mask=0x00800000;
    unsigned char sign=*(int*)buffer&mask;
    exp=*(unsigned char*)buffer-0x80;
    *(char*)(buffer+1)=*(char*)(buffer+1)|0x80;//assign the sign bit to 1
    //dumpbuffer(buffer,4);
    getint(buffer+1,&a,3);
    //dumpbuffer((unsigned char*)&a,4);
    b=a/powf(2,24-exp);
    *res=sign?0-b:b;
    return 0;
}

FT_HANDLE find_device()
{
	unsigned char * pcbuf[MAX_DEVICES+1];
    unsigned char   cbuf[MAX_DEVICES][64];

    FT_HANDLE fthandle[MAX_DEVICES];
	FT_STATUS ftstatus;
    int iNumDevs=0;
    int iDevicesOpen=0;
    int i;


    for(i=0; i<MAX_DEVICES; i++)
    {
        pcbuf[i]=cbuf[i];
    }
    pcbuf[MAX_DEVICES]=NULL;

    ftstatus=FT_ListDevices(pcbuf,&iNumDevs,FT_LIST_ALL | FT_OPEN_BY_SERIAL_NUMBER);

    if(ftstatus!=FT_OK)
    {
        //printf("Error:FT_ListDevices(%d)\n",(int)ftstatus);
        return NULL;
    }

    for(i=0; ((i<MAX_DEVICES)&&(i<iNumDevs)); i++)
    {
        //printf("Device %d Serial Number - %s\n",i,cbuf[i]);
    }
    for(i=0; ((i<MAX_DEVICES)&& (i<iNumDevs)); i++)
    {
        if((ftstatus=FT_OpenEx(cbuf[i],FT_OPEN_BY_SERIAL_NUMBER,
                               &fthandle[i]))!=FT_OK)
        {
            printf("Error FT_OpenEx(%d), device %d\n", (int)ftstatus, i);
            printf("Use lsmod to check if ftdi_sio (and usbserial) are present.\n");
            printf("If so, unload them using rmmod, as they conflict with ftd2xx.\n");
			continue;
        }
        //printf("Opened device %s for handshaking\n",cbuf[i]);

		config_serial(fthandle[i],57600);//configure com


		if(verify_sensor(fthandle[i]))
		{
			//FT_Close(fthandle[i]);
			return fthandle[i];
		}
        FT_Close(fthandle[i]);
        iDevicesOpen++;
        printf("Closed device %s\n",cbuf[i]);
	}
	return NULL;
}

int close_device(FT_HANDLE *fthandle)
{
	FT_STATUS ftstatus;
	ftstatus=FT_Close(fthandle);
	return ftstatus==FT_OK;
}

int inquire_device(
		FT_HANDLE fthandle,
		unsigned char *txbuf,
		unsigned char *rxbuf,
		DWORD txBytes,
		DWORD *rxBytes)
{//transmit instructions to the device and return the length of received data
	FT_STATUS ftstatus;
	DWORD bytes;
	ftstatus=FT_Write(fthandle,txbuf,txBytes,&bytes);
	if(ftstatus!=FT_OK)
	{
		perror("inquire_device(),FT_Write");
		return -1;
	}
	else
	{
		//dumpbuffer(txbuf,bytes);
	}

	sleep(1);

	ftstatus=FT_GetQueueStatusEx(fthandle,&bytes);
	if(ftstatus!=FT_OK)
	{
		perror("inquire_device()->FT_GetQueueStatus()");
		return -1;
	}
	ftstatus=FT_Read(fthandle,rxbuf,bytes,rxBytes);
	if(ftstatus!=FT_OK)
	{
		perror("inquire_device()->FT_Read()");
		return -1;
	}
	else
	{
		//dumpbuffer(rxbuf,*rxBytes);
	}
	return *rxBytes;
}

int verify_sensor(FT_HANDLE fthandle)
{
	DWORD rxbytes,txbytes;
	unsigned char rxbuf[BUF_SIZE],txbuf[INSTRUCTION_SIZE];
	int flag=0;
	txbytes=sizeof(QUERY_SLAVE_S)-1;
	memcpy(txbuf,QUERY_SLAVE_S,txbytes);
	rxbytes=inquire_device(fthandle,txbuf,rxbuf,txbytes,&rxbytes);
	if(rxbytes<=0)
	{
		perror("verify_tempsensor()->FT_Read()");
		return 0;
	}
	flag=(memcmp(rxbuf,RIGHT_SLAVE,rxbytes)==0);
	return flag;
}

int config_serial(
		FT_HANDLE fthandle,
		DWORD baudRate
		)
{
	FT_STATUS ftstatus;
	ftstatus=FT_SetBaudRate(fthandle,baudRate);
	if(ftstatus!=FT_OK)
	{
		printf("Error FT_SetBaudRate(%d)\n",
			   (int)ftstatus);
		return -1;
	}

	//FT_SetFLowControl(fthandle);
	return 0;
}

int inquire_temperature(FT_HANDLE fthandle,float *t)
{
    DWORD rxBytes,txBytes;
    unsigned char   rxbuf[BUF_SIZE],txbuf[INSTRUCTION_SIZE];
	time_t ti;//current time
    txBytes=sizeof(QUERY_TEMPERATURE_S);
    memcpy(txbuf,QUERY_TEMPERATURE_S,txBytes);

    //float t;//temperature

	rxBytes=inquire_device(fthandle,txbuf,rxbuf,txBytes,&rxBytes);
	ti=time(NULL);

    if(txBytes<=0)
    {
        perror("FT_Read");
		return -1;
    }
    else
    {
        //dumpbuffer(rxbuf,rxBytes);
		//convert data into temperature
		getfloat(rxbuf,t);
		savefile(ti,*t);
		//printf("%g\n",t);
		return 0;
    }
}

int savefile(long ti,float temp)
{//save the data in JSON array in a file
	char c,path[MAX_PATH],entry[MAX_ENTRY];
	struct tm *time_tm;
	//struct stat st;
	FILE *fp;
	time_tm=localtime(&ti);//put time in the struct tm
	sprintf(path,"%04d-%02d-%02d.log",
			time_tm->tm_year+1900,
			time_tm->tm_mon+1,
			time_tm->tm_mday);
	fp=fopen(path,"r+");//"r+",read and write,not "a"
	if(!fp)
	{
		fp=fopen(path,"w");
		fclose(fp);
		fp=fopen(path,"r+");
	}
	fseek(fp,0,SEEK_END);
	if(ftell(fp)==0)
	{//empty file
		fprintf(fp,"[\n]");
	}
	sprintf(entry,"[%ld,%g]",ti,temp);
	fseek(fp,-3,SEEK_END);
	c=fgetc(fp);
	if(c!='[')
	{
		fseek(fp,-2,SEEK_END);
		fprintf(fp,",\n%s\n]",entry);
	}
	else
	{
		fseek(fp,-1,SEEK_END);
		fprintf(fp,"%s\n]",entry);
	}
	fclose(fp);
	return 0;
}

int max_min_temperature(char *path,time_t time_start,time_t time_stop,
		float *max_temp,float *min_temp)
{
	char buf[MAX_BUF];
	time_t ti;//time read from file
	float temp;
	int n,flag=0;//flag indicates whether max and min are initialized
	FILE *fp=fopen(path,"r");
	if(!fp)
	{
		return -1;
	}
	memset(buf,0,sizeof(buf));
	while(!feof(fp)&&!ferror(fp))
	{
		fgets(buf,sizeof(buf)-1,fp);
		if(strcmp(buf,"[\n")==0)
		{
			continue;
		}
		else if(strcmp(buf,"]")==0)
		{//last line with the closing square bracket
			break;
		}
		else
		{
			//n=sscanf(buf,"[%[^,],%[^]]",&ti,&temp);
			n=sscanf(buf,"[%ld,%g",&ti,&temp);
			if(n!=2)
			{
				fclose(fp);
				return -1;
			}
			if(ti<time_start || ti>time_stop)
			{//not in the required time interval
				continue;
			}

			//entry in demanded scope
			if(flag==0)
			{
				*max_temp=temp;
				*min_temp=temp;
				flag=1;
			}
			else
			{//update the max_temp and min_temp
				if(temp>*max_temp)
				{
					*max_temp=temp;
				}
				else if(temp<*min_temp)
				{
					*min_temp=temp;
				}
			}
		}
	}
	fclose(fp);
	if(flag==0)
	{
		return -1;
	}
	return 0;
}
