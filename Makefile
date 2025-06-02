all:
	if [ ! -d ./build ] ; then \
	mkdir build \
	&& cd build \
	&& cmake .. \
	&& make; \
	fi


.PHONY:
	clean

clean:
	rm -rf ./build
