#!/usr/bin/python
# -*- coding: utf-8 -*-

import subprocess
import re

test_dir = "../testcase/"

if __name__ == "__main__":
	test_files = []
	output = subprocess.check_output("cd %s; ls *.txt" % test_dir, shell = True)
	test_files = output.split()
	for i in range(len(test_files)):
		test_files[i] = re.sub("\D", "", test_files[i])
	
	test_files.sort()

	for case in test_files:
		output = subprocess.check_output("./parser %stestcase%s.txt %sout%s.ir" %(test_dir, str(case), test_dir, str(case)), shell = True)
		#print "case %d: %s" %(case, output)
