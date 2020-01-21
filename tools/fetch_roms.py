import sys
import csv
import os

csvfile = sys.argv[1]
baseurl = sys.argv[2]

with open(csvfile, 'rb') as csvfile:
    romreader = csv.reader(csvfile)
    for row in romreader:
        cmd = "wget -nc " + baseurl + row[1]
        os.system(cmd)