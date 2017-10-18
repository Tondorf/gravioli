mainFBS = world.fbs
cpps = $(wildcard *_generated.h)
pythons = game

all: fbs 

fbs:
	flatc --cpp $(mainFBS)
	flatc --python $(mainFBS)

.PHONY: install
install: fbs
	mkdir -p ../server/fbs
	cp $(cpps) ../server/fbs/
	mkdir -p ../clients/python/fbs
	cp -r $(pythons) ../clients/python/fbs/

clean:
	rm $(cpps)
	rm -r $(pythons)
