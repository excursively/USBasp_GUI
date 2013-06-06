//      avrdude_string.h
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


#ifndef AVRDUDE_STRING_H
#define AVRDUDE_STRING_H

#include <windows.h>

const char AVRDUDE_NOTFOUND[] = "avrdude: error: could not find USB device \"USBasp\" with vid=0x16c0 pid=0x5dc";
const char AVRDUDE_WRONGPART_1[] = "(unused)         Double check chip, or use -F to override this check.";
const char AVRDUDE_WRONGPART_2[] = "         Double check chip, or use -F to override this check.";
const char AVRDUDE_INVALIDSIG[] = "avrdude: Yikes!  Invalid device signature.";
const char AVRDUDE_UNEXPECTEDPART[] = "avrdude: Expected signature for %*s is %0.2*X %0.2*X %0.2*X";
const char AVRDUDE_SELECTPART[] = "Valid parts are:";
//const char AVRDUDE_SIGNATURE_READ[] = "avrdude: reading signature memory:";
const char AVRDUDE_SIGNATURE_READ[] = "avrdude: Device signature = 0x%*s";

const char AVRDUDE_HFUSE_READ[] = "avrdude: reading hfuse memory:";
const char AVRDUDE_HFUSE_WRITE[] = "avrdude: writing hfuse (1 bytes):";
const char AVRDUDE_LFUSE_READ[] = "avrdude: reading lfuse memory:";
const char AVRDUDE_LFUSE_WRITE[] = "avrdude: writing lfuse (1 bytes):";
const char AVRDUDE_EFUSE_READ[] = "avrdude: reading efuse memory:";
const char AVRDUDE_EFUSE_WRITE[] = "avrdude: writing efuse (1 bytes):";

const char AVRDUDE_FLASH_READ[] = "avrdude: reading flash memory:";
const char AVRDUDE_FLASH_WRITE[] = "avrdude: writing flash (%u bytes):";;
const char AVRDUDE_FLASH_ERASE[] = "avrdude: erasing chip";
const char AVRDUDE_FLASH_VERIFY[] = "avrdude: reading on-chip flash data:";
const char AVRDUDE_EEPROM_READ[] = "avrdude: reading eeprom memory:";
const char AVRDUDE_EEPROM_WRITE[] = "avrdude: writing eeprom memory:";

const char AVRDUDE_DONE[] = "avrdude done.  Thank you.";

#endif
