cloud: cloud.cpp
	@g++ -std=c++17 -no-pie -o cloud cloud.cpp -L. -lbundle -ljsoncpp -lstdc++fs -lpthread

.PHONY: clean
clean:
	rm -rf cloud
reset:
	rm -rf backdir packdir
