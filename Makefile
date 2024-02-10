default:
	g++ ./srcs/*.cpp -o server
	g++ ./srcs/bot/*.cpp -o bot

run:
	./server 6697 password
