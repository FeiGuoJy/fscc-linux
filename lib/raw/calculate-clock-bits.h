/*
    Copyright (C) 2014 Commtech, Inc.

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

#ifndef CALCULATE_CLOCK_BITS_H
#define CALCULATE_CLOCK_BITS_H

int calculate_clock_bits(unsigned long freq, unsigned long ppm, 
                         unsigned char *clock_bits);

#endif
