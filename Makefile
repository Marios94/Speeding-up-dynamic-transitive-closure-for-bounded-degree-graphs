LEDALIB = '/usr/local/LEDA'
LEDAINCL = '/usr/local/LEDA/incl'

f=main.cpp

compile: $(f)
	g++ $(f) -o erg1 -O3 -std=c++0x -fno-strict-aliasing -I $(LEDAINCL) -lleda -L$(LEDALIB)

clean:
	rm -f erg1
run:
	./erg1

