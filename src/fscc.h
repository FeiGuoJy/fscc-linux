/*
	Copyright (C) 2010  Commtech, Inc.
	
	This file is part of fscc-linux.

	fscc-linux is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	fscc-linux is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with fscc-linux.  If not, see <http://www.gnu.org/licenses/>.
	
*/

#ifndef FSCC_H
#define FSCC_H

#define FSCC_UPDATE_VALUE -2

#define FSCC_IOCTL_MAGIC 0x18
#define FSCC_GET_REGISTERS _IOR(FSCC_IOCTL_MAGIC, 0, __u32)
#define FSCC_SET_REGISTERS _IOW(FSCC_IOCTL_MAGIC, 1, __u32)

struct fscc_registers {
	__s32 reserved1[2];

	__s32 FIFOT;

	__s32 reserved2[2];

	__s32 CMDR;
	__s32 STAR;
	__s32 CCR0;
	__s32 CCR1;
	__s32 CCR2;
	__s32 BGR;
	__s32 SSR;
	__s32 SMR;
	__s32 TSR;
	__s32 TMR;
	__s32 RAR;
	__s32 RAMR;
	__s32 PPR;
	__s32 TCR;
	__s32 VSTR; /* Read-only */
	
	__s32 reserved3[1];
	
	__s32 IMR;
};

#endif

