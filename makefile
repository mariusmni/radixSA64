all:
	make -C Release/
	cp Release/radixSA .

clean:
	make -C Release/ clean
	rm -f radixSA Release/radixSA
