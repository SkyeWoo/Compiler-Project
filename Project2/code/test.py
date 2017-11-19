#!/usr/bin/python
# -*- coding: utf-8 -*-

import subprocess

test_dir = "../pretest/"

if __name__ == "__main__":
	test_files = []
	output = subprocess.check_output("cd %s; ls *.txt" % test_dir, shell = True)
	test_files = output.split()
	for i in range(len(test_files)):
		test_files[i] = int(test_files[i].split(".")[0])
	
	test_files.sort()

	for case in test_files:
		output = subprocess.check_output("./parser %s%s.txt" %(test_dir, str(case)), shell = True)
		print "case %d: %s" %(case, output),
