client: 
	g++ -fpermissive -w -o client clientDriver.cpp

server:
	g++ -fpermissive -w -o server serverDriver.cpp

clean:
	rm client && rm server
