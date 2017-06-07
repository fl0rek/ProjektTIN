import fileinput, sys, filecmp

	
if __name__ == "__main__":
	lines_seen = set()
	lines_seen2 = set()
	result1 = open("result1.txt", "w")
	result2 = open("result2.txt", "w")
	with open(sys.argv[1]) as f:
		for line in f:
			if line.rstrip():
				if line not in lines_seen:
					result1.write(line)
					lines_seen.add(line)
	with open(sys.argv[2]) as f:
		for line in f:
			if line.rstrip():
				if line not in lines_seen2:
					result2.write(line)
					lines_seen2.add(line)
	result1.close()
	result2.close()
	if((filecmp.cmp("result1.txt", "result2.txt")) == True):
		print("Files are the same, here is content of the first one:\n")
		with open("result1.txt", 'r') as f:
			for line in f:
				sys.stdout.write(line)
 
