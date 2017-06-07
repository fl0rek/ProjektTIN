import fileinput, sys, filecmp


lines_seen = set()
lines_seen2 = set()
result1 = open("result1.txt", "w")
result2 = open("result2.txt", "w")

lines1 = open(sys.argv[1]).readlines()
lines2 = open(sys.argv[2]).readlines()

for l in lines1:
        if l not in lines_seen:
                lines_seen.add(l)
                print(l)
                result1.write(l)

for l in lines2:
        if l not in lines_seen2:
                lines_seen2.add(l)
                print(l)
                result2.write(l)
result1.close()
result2.close()

print(filecmp.cmp("result1.txt", "result2.txt"))
                                                         
