/***************************************************************************
 *   Copyright (C) 2007 by Stoian Ivanov                                   *
 *   sdr@tera-com.com                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU  General Public License version 2       *
 *   as published by the Free Software Foundation.                         *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
 /** \file
      \brief Easy parser implementation
	  \author Stoian Ivanov sdr@tera-com.com
  */


#include <string.h>

#include <stdlib.h>
#include <stdio.h>

#include "easyparse.h"

int easyparse(char *buf,int bufl,easyparse_cb_t *callback,void*userdata) {
	if (!callback) return -1;
	if (bufl<0) bufl=strlen(buf);
	if (bufl==0) return -1;
	char *cls=buf;
	int firsteq=0;
	int firstlt=0;
	char *datastart=NULL;
	int datal=0;
	
	char *dterm=NULL;
	int dterml=0;
	
	int cwsrun=0;
	int trimstart=-1;
	int trimmidl=-1;
	int trimmidr=-1;
	int trimend=-1;
		
	int bleft=bufl;
	int cidx=0;
	char c;
	int mode=0;
	/*
	 modes:
	  0 - unknown
	  1 - comment
	  2 - name=val line
	  3 - name<terminator line
	  4 - data line
	*/
	while (cidx<=bleft) {
		if (cidx==bleft) {
			if (mode!=2) break;
			else c='\n';
		} else c=cls[cidx];
		if ((mode==1 && c!='\n') || (mode==0 && c=='#') ) { //commens fast forward 
			cidx++;
			mode=1;
			cwsrun=0;
			continue;
		}
		if ((mode==1 || mode==0) && c=='\n') {//EOL in unknown or comment
			//===NEXT LINE==
			cls+=cidx+1;
			bleft-=cidx+1;
			cidx=0;
			mode=0;
			firsteq=0;
			firstlt=0;
			cwsrun=0;
			trimstart=-1;
			trimend=-1;
			trimmidl=-1;
			trimmidr=-1;
			datastart=NULL;
			datal=0;
			dterm=NULL;
			dterml=0;
			//===============
			continue;
		}

		if (mode==0 && c=='=') {//unknown to name=val
			trimmidr=-2;
			trimmidl=cwsrun;
			mode=2;
			firsteq=cidx;
			cwsrun=0;
			cidx++; 
			continue;
		} 
		if (mode==0 && c=='<') {//unknown to name<term
			trimmidr=-2;
			trimmidl=cwsrun;
			mode=3;
			firstlt=cidx;
			cwsrun=0;
			cidx++; 
			continue;
		} 
		
		if (c=='\n'){//EOL in non (comment or unknown)
			trimend=cwsrun;
			cwsrun=0;
			if (mode==2){ //name=val
				char * name=cls;
				int namel=firsteq;
				if (trimstart>0) {
					name+=trimstart;
					namel-=trimstart;
				}
				if (trimmidl>0) namel-=trimmidl;
				char * value=cls+firsteq+1;
				int valuel=cidx-firsteq-1;
				if (trimmidr>0){
					value+=trimmidr;
					valuel-=trimmidr;
				}
				if (trimend>0){
					valuel-=trimend;
				}
				callback (userdata,name,namel,value,valuel);
				
				//===NEXT LINE==
				cls+=cidx+1;
				bleft-=cidx+1;
				cidx=0;
				mode=0;
				firsteq=0;
				firstlt=0;
				cwsrun=0;
				trimstart=-1;
				trimend=-1;
				trimmidl=-1;
				trimmidr=-1;
				datastart=NULL;
				datal=0;
				dterm=NULL;
				dterml=0;
				//===============
				continue;
			} else if (mode==3){ //name<term
				char * name=cls;
				int namel=firsteq;
				if (trimstart>0) {
					name+=trimstart;
					namel-=trimstart;
				}
				if (trimmidl>0) namel-=trimmidl;
				dterm=cls+firsteq+1;
				dterml=cidx-firsteq-1;
				if (trimmidr>0){
					dterm+=trimmidr;
					dterml-=trimmidr;
				}
				if (trimend>0){
					dterml-=trimend;
				}
				
				if (dterml==0) { //invalid terminator
					//===NEXT LINE==
					cls+=cidx+1;
					bleft-=cidx+1;
					cidx=0;
					mode=0;
					firsteq=0;
					firstlt=0;
					cwsrun=0;
					trimstart=-1;
					trimend=-1;
					trimmidl=-1;
					trimmidr=-1;
					datastart=NULL;
					datal=0;
					dterm=NULL;
					dterml=0;
					//===============
				
				} else { //valid terminator
					//===NEXT LINE==
					cls+=cidx+1;
					bleft-=cidx+1;
					cidx=0;
					mode=4; //new mode
					firsteq=0;
					firstlt=0;
					cwsrun=0;
					trimstart=-1;
					trimend=-1;
					trimmidl=-1;
					trimmidr=-1;
					datastart=cls; //set data start
					datal=0;  //set initial datal
					//dterm=NULL; //dterm preserved from mode 3 to 4 
					//dterml=0; //dterml preserved from mode 3 to 4 
					//===============
				}
				continue;
			} else { //data
				char * name=cls;
				int namel=cidx;
				if (trimstart>0) {
					name+=trimstart;
					namel-=trimstart;
				}
				if (trimmidl>0) namel-=trimmidl;
				
				
				//===NEXT LINE==
				cls+=cidx+1;
				bleft-=cidx+1;
				cidx=0;
				mode=4; //new mode
				firsteq=0;
				firstlt=0;
				cwsrun=0;
				trimstart=-1;
				trimend=-1;
				trimmidl=-1;
				trimmidr=-1;
				datastart=cls; //set data start
				datal=0;  //set initial datal
				//dterm=NULL; //dterm preserved from mode 3 to 4 
				//dterml=0; //dterml preserved from mode 3 to 4 
				//===============
			}
		} else { //advance
			if (mode==0 ||  mode==2 || mode==3) { //unknown or name=val  or name<term
				//whitespace runs for trimming 
				if (c==' '  ) {
					cwsrun++;
				} else {
					if (trimstart==-1){
						trimstart=cwsrun;
					}
					if (trimmidr==-2){
						trimmidr=cwsrun;
					}
					cwsrun=0;
				}
			}
			cidx++;
		}
	}
	//TODO:
	//callback (userdata,"test",4,"12345",5);
	return 0;
}
