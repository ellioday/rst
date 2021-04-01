/* GridIntegrate.c
   ===============
   Author: R.J.Barnes
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


*/


#include <stdio.h>
#include <stdlib.h>

#include "rtypes.h"
#include "rtime.h"
#include "dmap.h"
#include "limit.h"
#include "griddata.h"
#include "gridread.h"
#include "gridwrite.h"


struct GridData grd;
struct GridData ogrd;
struct GridData igrd;

int main(int argc,char *argv[]) {
 
  FILE *fp;

  double err[3]={100,0.1,10};
  int num=0;


  int yr,mo,dy,hr,mt;
  double sc;

  fp=fopen(argv[1],"r");

  if (fp==NULL) {
    fprintf(stderr,"File not found.\n");
    exit(-1);
  }

  while(GridFread(fp,&grd) !=-1) {

    TimeEpochToYMDHMS(grd.st_time,&yr,&mo,&dy,&hr,&mt,&sc);
    fprintf(stderr,"%.4d-%.2d-%.2d %.2d:%.2d:%.2d pnts=%d\n",
            yr,mo,dy,hr,mt,(int) sc,grd.vcnum);

   if (num==0) GridCopy(&ogrd,&grd);
   else GridAdd(&ogrd,&grd,num);
   num++;
   if (num==10) {
     GridAdd(&ogrd,NULL,num);
     ogrd.ed_time=grd.ed_time;
     GridIntegrate(&igrd,&ogrd,err);
     GridFwrite(stdout,&igrd);
     num=0;
   }    
  }
 
  fclose(fp);


  return 0;
}
