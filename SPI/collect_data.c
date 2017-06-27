/*
 *  written by Steven
 * 
 * 	comments: wiringPiSPISetup(0, 25000000 ) -> 25 MHz asked, 21.3 MHz measured with scope
 * 											 -> 50 MHz asked, 42.6 MHz measured with scope
 * 
 * 			  a subdirectory "results/" is assumed
 * 	
 * 	to do: fprintf multiple samples at a time to improve performance + test speed
 * 
 * 
 * 
*/

#include <wiringPi.h>
#include <wiringPiSPI.h>

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>

/* print byte values in binary */
#define BYTE_TO_BINARY(byte) \
	(byte & 0x80 ? '1' : '0'), \
        (byte & 0x40 ? '1' : '0'), \
        (byte & 0x20 ? '1' : '0'), \
        (byte & 0x10 ? '1' : '0'), \
        (byte & 0x08 ? '1' : '0'), \
        (byte & 0x04 ? '1' : '0'), \
        (byte & 0x02 ? '1' : '0'), \
        (byte & 0x01 ? '1' : '0')
#define BINARY_PRINT_PATTERN "%c%c%c%c%c%c%c%c\n"
#define LONG_PRINT_PATTERN   "%c%c%c%c%c%c%c%c\t%c%c%c%c%c%c%c%c\t%c%c%c%c%c%c%c%c\t%c%c%c%c%c%c%c%c\n"


/* function declarations */
void * blink(void * arg);


int main(void)
{
	printf("Starting collect_data.c\n");	

	/* local variables */
	pthread_t thread_heartbeat;
	unsigned char SPI_buffer[4] = {0}; /* 1 char = 8 bit -> 32 bit buffer */
	uint32_t index = 0;
	FILE * fp;
	char filename[50], dir[20]; /* string containing filename */
	time_t now;
	
	wiringPiSetup();
	
	/* configure pins */
	pinMode(0, OUTPUT); /* physical 11 */
	pinMode(3, INPUT); /* physical 15 */
	pinMode(2, OUTPUT); /* physical 13 */

	/* setup SPI */
	if( wiringPiSPISetup(0, 25000000 ) == -1 ) /* channel 0, 25MHz */ 
 	{
		printf("Error setting up SPI: %s\n", strerror(errno));
	}	


	/* create heartbeat thread */
	if(pthread_create(&thread_heartbeat,NULL,&blink,NULL))
	{
		fprintf(stderr, "Error creating thread\n");
		return 1;
	}
	
	/* open new file, with timestamp, to write samples to */
	now = time(0)+7200; /* add 2 hours = 7200 sec for GMT+02 */
	strftime( filename , 50 , "%d-%m-%y_%H:%M:%S" , localtime(&(now)));
	
	strcpy(dir,"results/"); /* write in directory "results/" */
	fp = fopen( strcat(dir,filename) ,"a");
	if (fp == NULL )
	{
		printf("Error opening file: %s\n", strerror(errno));
	}	
	
	
	while(1)
	{
		/* execute main code */
		
		/* if PIC signals RBP */
		if (digitalRead(3) == HIGH )
		{
			//usleep(15.625); //to test: frequency of 64000 Hz
			
			/* collect sample; 32 bits */
			digitalWrite(2, HIGH); /* debugging led */
			
			wiringPiSPIDataRW(0, SPI_buffer , 4 ); /* channel 0; 4 byte -> 32 bit */ 

			/* write samples to text file */
//			fprintf(fp,"%d\t",index); 
			fprintf(fp,"%02X%02X%02X%02X\n",*SPI_buffer,*(SPI_buffer+1),*(SPI_buffer+2),*(SPI_buffer+3) ); 
						
			
			/* print received SPI word (32 bit) */
			/* ///print index */
//			printf("%d\t",index); 
			
			/* ///print SPI word in hexadecimal */
//			printf("%02X%02X%02X%02X\t\t",*SPI_buffer,*(SPI_buffer+1),*(SPI_buffer+2),*(SPI_buffer+3) ); 

			/* ///print binary representation */
//			printf(LONG_PRINT_PATTERN,BYTE_TO_BINARY(*SPI_buffer),BYTE_TO_BINARY(* (SPI_buffer+1) ), \
				BYTE_TO_BINARY(* (SPI_buffer+2) ),BYTE_TO_BINARY(* (SPI_buffer+3) ) );
			
			
			
			index++;
		}
		else
		{
			digitalWrite(2, LOW); /* debugging led */
		}
	};


	/* some management */
	pthread_join(thread_heartbeat,NULL);
	return 0;
}


void * blink(void * arg)
{
	while(1)
	{
		digitalWrite(0, HIGH); delay(500);
		digitalWrite(0, LOW); delay(500);
	}

	return 0;
}
