all:
	g++ -o tracker tracker.cpp -lpthread
	g++ -o peer peer.cpp -lpthread
	mkdir -p peer_storage

clean:
	rm -f tracker peer
	rm -rf peer_storage
.PHONY: all clean