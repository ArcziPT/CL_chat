client:
	g++ -o client Server/client/main.cpp Server/client/Client.cpp include/TextHelper/TextHelper.cpp include/socket/Socket.cpp -pthread
	
server:
	g++ -o server Server/main.cpp Server/Server.cpp Server/channel/Channel.cpp Server/client/Client.cpp include/TextHelper/TextHelper.cpp include/DataManager/DataManager.cpp include/socket/Socket.cpp -pthread -g
	
all:
	g++ -o client Server/client/main.cpp Server/client/Client.cpp include/TextHelper/TextHelper.cpp include/socket/Socket.cpp -pthread -lncurses
	g++ -o server Server/main.cpp Server/Server.cpp Server/channel/Channel.cpp Server/client/Client.cpp include/TextHelper/TextHelper.cpp include/DataManager/DataManager.cpp include/socket/Socket.cpp -pthread -g -lncurses
