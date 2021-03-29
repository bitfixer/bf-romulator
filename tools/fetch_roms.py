# ROMulator - RAM/ROM replacement and diagnostic for 8-bit CPUs
# Copyright (C) 2019  Michael Hill

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

import sys
import csv
import os

csvfile = sys.argv[1]
baseurl = sys.argv[2]

with open(csvfile, 'rb') as csvfile:
    romreader = csv.reader(csvfile)
    for row in romreader:
        if len(row) >= 3:
            cmd = "wget -nc " + baseurl + row[1]
            os.system(cmd)