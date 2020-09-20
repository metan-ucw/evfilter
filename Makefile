all:
	make -C src

clean:
	make -C src clean

install:
	make -C src install

tar:
	make clean
	cd .. && tar czf libevfilter.tgz libevfilter/
