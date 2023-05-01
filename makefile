all: s c 

s: ser.cc
	g++ -o s.out ser.cc

c: cli.cc
	g++ -o c.out cli.cc

