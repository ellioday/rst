/* trim_snd.c
   ==========
   Author: E.G.Thomas
*/

/*
 Copyright (C) <year>  <name of author>

Copyright (C) <year>  <name of author>

This file is part of the Radar Software Toolkit (RST).

RST is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of

MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <https://www.gnu.org/licenses/>.

Modifications:

RST is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of

MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <https://www.gnu.org/licenses/>.

Modifications:
 
 Copyright (c) 2012 The Johns Hopkins University/Applied Physics Laboratory
 
 Copyright (C) <year>  <name of author>

This file is part of the Radar Software Toolkit (RST).

RST is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of

MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <https://www.gnu.org/licenses/>.

Modifications:
 
 RST is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 any later version.
 
 RST is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with RST.  If not, see <http://www.gnu.org/licenses/>.
 
 
 
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ctype.h>
#include <zlib.h>
#include "rtypes.h"
#include "dmap.h"
#include "rtime.h"
#include "option.h"
#include "dmap.h"
#include "snddata.h"

#include "sndread.h"
#include "sndwrite.h"
#include "sndseek.h"

#include "errstr.h"
#include "hlpstr.h"


struct SndData *snd;
struct OptionData opt;

double strdate(char *text) {
  double tme;
  int val;
  int yr,mo,dy;
  val=atoi(text);
  dy=val % 100;
  mo=(val / 100) % 100;
  yr=(val / 10000);
  if (yr<1970) yr+=1900;
  tme=TimeYMDHMSToEpoch(yr,mo,dy,0,0,0);

  return tme;
}

double strtime(char *text) {
  int hr,mn;
  double sc;
  int i,j;
  for (i=0;(text[i] !=':') && (text[i] !=0);i++);
  if (text[i]==0) return atoi(text)*3600L;
  text[i]=0;
  hr=atoi(text);
  for (j=i+1;(text[j] !=':') && (text[j] !=0);j++);
  if (text[j]==0) {
    mn=atoi(text+i+1);
    return (double) hr*3600L+mn*60L;
  }
  text[j]=0;
  mn=atoi(text+i+1);
  sc=atof(text+j+1);
  return (double) hr*3600L+mn*60L+sc;
}

int rst_opterr (char *txt) {
  fprintf(stderr,"Option not recognized: %s\n",txt);
  fprintf(stderr,"Please try: trim_snd --help\n");
  return(-1);
}

int main (int argc,char *argv[]) {

  int arg;

  int status=0;
  FILE *fp=NULL;

  int yr,mo,dy,hr,mt;
  double sc;
 
  double atime;

  char *stmestr=NULL;
  char *etmestr=NULL;
  char *sdtestr=NULL;
  char *edtestr=NULL;
  char *exstr=NULL;

  double stime=-1;
  double etime=-1;
  double extime=0;
 
  double sdate=-1;
  double edate=-1;

  unsigned char vb=0;
  unsigned char help=0;
  unsigned char option=0;
  unsigned char version=0;

  time_t ctime;
  int c,n;
  char command[128];
  char tmstr[40];

  OptionAdd(&opt,"-help",'x',&help);
  OptionAdd(&opt,"-option",'x',&option);
  OptionAdd(&opt,"-version",'x',&version);

  OptionAdd(&opt,"vb",'x',&vb);
  OptionAdd(&opt,"st",'t',&stmestr);
  OptionAdd(&opt,"et",'t',&etmestr);
  OptionAdd(&opt,"sd",'t',&sdtestr);
  OptionAdd(&opt,"ed",'t',&edtestr);
  OptionAdd(&opt,"ex",'t',&exstr);

  arg=OptionProcess(1,argc,argv,&opt,rst_opterr);

  if (arg==-1) {
    exit(-1);
  }

  if (help==1) {
    OptionPrintInfo(stdout,hlpstr);
    exit(0);
  }
  if (option==1) {
    OptionDump(stdout,&opt);
    exit(0);
  }

  if (version==1) {
    OptionVersion(stdout);
    exit(0);
  }


  if (exstr !=NULL) extime=strtime(exstr);
  if (stmestr !=NULL) stime=strtime(stmestr);
  if (etmestr !=NULL) etime=strtime(etmestr);
  if (sdtestr !=NULL) sdate=strdate(sdtestr);
  if (edtestr !=NULL) edate=strdate(edtestr);

  command[0]=0;
  n=0;
  for (c=0;c<argc;c++) {
    n+=strlen(argv[c])+1;
    if (n>127) break;
    if (c !=0) strcat(command," ");
    strcat(command,argv[c]);
  }

  snd=SndMake();

  if (arg==argc) fp=stdin;
  else fp=fopen(argv[arg],"r");

  if (fp==NULL) {
    fprintf(stderr,"File not found.\n");
    exit(-1);
  }

  if (SndFread(fp,snd)==-1) {
    fprintf(stderr,"Error reading file\n");
    exit(-1);
  }

  atime=TimeYMDHMSToEpoch(snd->time.yr,snd->time.mo,snd->time.dy,
                          snd->time.hr,snd->time.mt,
                          snd->time.sc+snd->time.us/1.0e6);

  /* rewind file pointer in case we want to trim first record */
  rewind(fp);

  /* skip here */

  if ((stime !=-1) || (sdate !=-1)) {
    if (stime==-1) stime= ( (int) atime % (24*3600));
    if (sdate==-1) stime+=atime - ( (int) atime % (24*3600));
    else stime+=sdate;

    TimeEpochToYMDHMS(stime,&yr,&mo,&dy,&hr,&mt,&sc);
    status=SndFseek(fp,yr,mo,dy,hr,mt,sc,NULL);
    if (status ==-1) {
      fprintf(stderr,"File does not contain the requested interval.\n");
      exit(-1);
    }
    if (SndFread(fp,snd)==-1) {
      fprintf(stderr,"Error reading file\n");
      exit(-1);
    }
  } else stime=atime;

  if (etime !=-1) {
     if (edate==-1) etime+=atime - ( (int) atime % (24*3600));
     else etime+=edate;
  }
  if (extime !=0) etime=stime+extime;

  do {
    atime=TimeYMDHMSToEpoch(snd->time.yr,snd->time.mo,snd->time.dy,
                            snd->time.hr,snd->time.mt,
                            snd->time.sc+snd->time.us/1.0e6);

    if ((etime !=-1) && (atime>=etime)) break;

    TimeEpochToYMDHMS(atime,&yr,&mo,&dy,&hr,&mt,&sc);
    if (vb==1) fprintf(stderr,"%d-%d-%d %d:%d:%d",
                       yr,mo,dy,hr,mt,(int) sc);

    snd->origin.code=1;
    ctime = time((time_t) 0);
    SndSetOriginCommand(snd,command);
    strcpy(tmstr,asctime(gmtime(&ctime)));
    tmstr[24]=0;
    SndSetOriginTime(snd,tmstr);

    status=SndFwrite(stdout,snd);

  } while (SndFread(fp,snd) !=-1);
  if (fp !=stdin) fclose(fp);

  return 0;
}

