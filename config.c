//      config.c
//      
//      Copyright 2009 Sataporn Saijai <nothing2give@gmail.com>
//      
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 2 of the License, or
//      (at your option) any later version.
//      
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//      
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
//      MA 02110-1301, USA.


#include <windows.h>
#include <stdio.h>
#include "config.h"

int read_config(PART_INFO **lpPartList) {
	FILE *f;
	unsigned char *lineBuf;
	unsigned char *partName;
	unsigned char *partID;
	unsigned char partCount = 0;
	lineBuf = malloc(256);
	partName = malloc(32);
	partID = malloc(32);
	memset(lineBuf, 0, 256);
	memset(partName, 0, 32);
	memset(partID, 0, 32);
	unsigned long partSig;

	// add to list
	
	PART_INFO *partinfo = (PART_INFO *)malloc(sizeof(PART_INFO));
	memset(partinfo, 0, sizeof(PART_INFO));
	partinfo->next = *lpPartList;
	*lpPartList = partinfo;
	partinfo->partSig = 0x000000;
	strcpy(partinfo->partName, "-");
	strcpy(partinfo->partID, "?");								
	
	f = fopen("avrdude.conf", "r");
	if (f != NULL) {
		while (!feof(f)) {
			unsigned char sx[3];
			int ret;
			fgets(lineBuf, 256, f);			
			sscanf(lineBuf, " desc = \"%s", partName);
			sscanf(lineBuf, " id = \"%s", partID);
			ret = sscanf(lineBuf, " signature = 0x%x 0x%x 0x%x;", &sx[0], &sx[1], &sx[2]);
			if (ret>2) {
				partSig = (sx[0]<<16) + (sx[1]<<8) + sx[2];
				partID[strlen(partID)-2] = '\0';
				partName[strlen(partName)-2] = '\0';
				// add to list
				PART_INFO *partinfo = (PART_INFO *)malloc(sizeof(PART_INFO));
				memset(partinfo, 0, sizeof(PART_INFO));
				partinfo->next = *lpPartList;
				*lpPartList = partinfo;
				partinfo->partSig = partSig;
				strcpy(partinfo->partName, partName);
				strcpy(partinfo->partID, partID);								
				partCount++;
			}
		}
		fclose(f);
		return partCount;
	} else {
		return -1;
	}
	return 0;
}

PART_INFO **find_part(PART_INFO **lpPartList, unsigned long partSig, char *partName, BOOL *found) {
	*found = FALSE;
	while (*lpPartList!=NULL) {
		if (partSig == 0) { // find from part name
			if (strcmpi(partName, (*lpPartList)->partName) == 0) {
				*found = TRUE;
				return lpPartList;
			}
		}
		else { // find from signature
			unsigned long fSig = 0;
			fSig = (unsigned long)(*lpPartList)->partSig;
			if (fSig == partSig) {
				*found = TRUE;
				return lpPartList;
			}
		}
		lpPartList = &(*lpPartList)->next;
	}
	*found = FALSE;
	return NULL;
}
