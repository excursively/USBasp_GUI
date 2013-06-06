//      config.h
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


#ifndef CONFIG_H
#define CONFIG_H

#include <windows.h>

#define MAXPARTLIST 100

typedef struct PART_INFO_L {
	char partID[32];
	char partName[32];
	unsigned long partSig;
	struct PART_INFO_L *next;
} PART_INFO;

int read_config(PART_INFO **lpPartList) ;
PART_INFO **find_part(PART_INFO **lpPartList, unsigned long partSig, char *partName, BOOL *found) ;

#endif
