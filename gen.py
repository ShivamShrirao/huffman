with open("t.txt","w") as f:
	for i in range(80):
		out=""
		for i in range(1024*1024):
			out+='a'
		f.write(out)