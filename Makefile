all: server client

server:
	@mkdir test-server
	cc -O2 -x c -o test-server/server server/*.c

client:
	@mkdir test-client
	cc -O2 -x c -o test-client/client client/*.c
