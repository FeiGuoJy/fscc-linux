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

using System;
using System.IO;

namespace FSCC
{
	class MainClass
	{
		public static void Main(string[] args)
		{
		    FSCC.Port port = null;

		    try
		    {
			    port = new Port("/dev/fscc0", FileAccess.Write);
			}
			catch (System.UnauthorizedAccessException e)
			{
			    Console.WriteLine(e.Message);

			    return;
			}

            Console.WriteLine(String.Format("CCR0 = 0x{0:x8}", port.Registers.CCR0));
            Console.WriteLine(String.Format("CCR1 = 0x{0:x8}", port.Registers.CCR1));
            Console.WriteLine(String.Format("CCR2 = 0x{0:x8}", port.Registers.CCR2));

			port.Close();
		}
	}
}