//
//  main.c
//  ulfcmd
//
//  Test Controller for AAL-PIP X34 buffer ADC board
//  
//

#define DAQPATH "/home/pi-unh-daq/ULF/Data_Files/"
#define ULFPATH "/home/pi-unh-daq/ULF/"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>     // for atoi()
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <time.h>       // localtime_r(), strftime()
#include <sys/time.h>
#include <pigpio.h>
#include <pigpiod_if2.h>

#define LOOPUSEC 1000000

long sec;

FILE *dfile;
long record;

long rcvok;

long paklen;

struct timeval rcvtime;

unsigned char  pkt[ 100 ];
long nread;

static long waitpkt(int fd)
{
    long timeout;

    timeout = 11;   // wait up to 1.1sec for packet to start
    while(timeout--)
    {
        nread = read(fd,pkt,sizeof(pkt));
        if(nread>0) return nread;
        usleep(100000); // 100ms
    }

    rcvok = 0;
    return 0;
}

static char * get_gps_nmea(char buff[100]){

    FILE* fp;
    system("/home/pi-unh-daq/ULF/string.sh");
    fp = fopen("/home/pi-unh-daq/ULF/NMEA_String.txt", "r");
    fgets(buff, 100, fp);
    fclose(fp);

    return buff;
}

// Open new hourly file. Make file name from rcvtime
static void newfile(void)
{
    struct tm    ttime;
    char  time_string[ 32 ], fname[ 64 ], pathname[128];
    char  gp[100]=" ";

    if(rcvok)
    {
	// Wait for GPS lock before creating file
        get_gps_nmea(gp);

	// Getting time and creating filename
        localtime_r( &( rcvtime.tv_sec ), &ttime );
        strftime( time_string, 32, "%Y%m%d_%H%M", &ttime );
        snprintf( fname, sizeof( fname ), "ULF-UNH-%s.dat", time_string );
        snprintf( pathname, sizeof( pathname ), "%s%s", DAQPATH, fname );

	// Opening and writing NMEA string to file
        if(dfile) fclose(dfile);
        dfile = fopen( pathname,"wb+" );
	fputs(gp, dfile);
	fputs("\n", dfile);
    }
    else
        printf("NO INCOMING PACKETS\n");
}

// read, unpack and dump and record incoming data
static void readpkt(int fd)
{
    int16_t x,y,z;
    long i,ns,secofhr;
    u_int32_t secofday;

    if(!waitpkt(fd)) return;        // wait for packet start
    gettimeofday(&rcvtime, NULL);

    if(nread<paklen)    // partial packet
    {
        usleep(70000); // wait 70ms for pkt to finish (should be <50ms)
        i = nread;
        nread += read(fd,&pkt[i],sizeof(pkt)-i);  // read in rest of packet
    }

    if(nread != paklen)
    {
        tcflush( fd, TCIFLUSH );    // discard any extra
        if(nread == 30)
        {
            paklen = 30;
        }
        else if(nread == 50)
        {
            paklen = 50;
        }
        return;
    }

    if(!rcvok)
    {
        fflush(stdout);
    }
    rcvok = 1;

    secofhr = rcvtime.tv_sec%3600;
    secofday = rcvtime.tv_sec%86400;
    if(record && (secofday == 0)) newfile();

    i=0;
    ns = 0;
    while(i<nread)
    {
        /*if(record && i == 0)
	{
	    fwrite(&secofday, 1, sizeof(secofday), dfile);
	}*/
        if(nread >= 30)
        {
            x = (pkt[i]<<4) + ((pkt[i+1]&0xF0)>>4);
            y = ((pkt[i+1]&0x0F)<<8) + pkt[i+2];
            i +=3;

            if(record)
	    {
		fwrite(&x, 1, sizeof(x), dfile);
		fwrite(&y, 1, sizeof(y), dfile);
	    }
        }
        if(nread == 50)
        {
            z = ((pkt[i]<<8) + pkt[i+1]);
            i+=2;

            if(record)
	    {
		fwrite(&z, 1, sizeof(z), dfile);
	    }
        }
        ns++;
    }

    if(record)
    {
        if((sec%10) == 0)
        {
            fflush(stdout);
            fflush(dfile );
        }
    }

}

int main (int argc, const char * argv[])
{
    char line[64];
    char *ttydev;
    int ttyfd;
    struct  termios com_io;
    struct timeval t0,t1,delta;
    long usec;
        
    
    // may need to be changed if usb is unplugged
    ttydev = "/dev/ttyUSB0";
    
    if(argc == 2) ttydev = (char *)argv[1]; // get from cmd line
    
    ttyfd = open(ttydev, O_RDWR | O_NONBLOCK | O_NOCTTY);
    
    while(ttyfd<0)
    {
        printf("tty open '%s' failed\ntty?",ttydev);
        ttydev = fgets( line, sizeof( line ), stdin );    // new name
        if(strlen(ttydev) < 4) return 0; // should be "/dev/xxx" give up on empty string
        ttydev[strlen(ttydev)-1]=0; // get rid of newline
        
        ttyfd = open(ttydev, O_RDWR | O_NONBLOCK | O_NOCTTY);  // try agian
    }
    
    cfmakeraw( &com_io );
    
    com_io.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
    
    com_io.c_iflag = IGNPAR;
    com_io.c_oflag = 0;
    com_io.c_lflag = 0;
    com_io.c_cc[ VTIME ] = 0;     // no wait
    com_io.c_cc[ VMIN ] = 0;      // return 0 if no data avail
    
    tcflush( ttyfd, TCIFLUSH );
    
    tcsetattr( ttyfd, TCSANOW, &com_io );
    
    fcntl(0,F_SETFL,O_NONBLOCK);    // nowait on stdin

    rcvok = 1;
    paklen = 30;    // default 2 axis
    record = 0;
    dfile = 0L;
    
    //Start new data file and begin recording automatically 
    gettimeofday(&rcvtime, NULL);
    newfile();
    record = 1;
    while(1)
    {
        gettimeofday(&t0, NULL);
        
        sec = t0.tv_sec;

        readpkt(ttyfd);
        
        gettimeofday(&t1, NULL);
        timersub(&t1,&rcvtime,&delta);
        usec = LOOPUSEC-delta.tv_usec-350000;
        if(usec>0) usleep((unsigned)usec); // wait remainder of sec
    }
    
    return 0;
}

