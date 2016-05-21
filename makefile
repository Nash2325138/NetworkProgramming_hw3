CC=g++ -Wall
ser_c=server.cpp
cli_c=client.cpp
cliName=HW3_103062224_Cli
serName=HW3_103062224_Ser
all: ${ser_c} ${cli_c}
	${CC} -o ${cliName} ${cli_c}
	${CC} -o ${serName} ${ser_c}
client: ${cli_c}
	${CC} -o client ${cli_c} 
server: ${ser_c}
	${CC} -o server ${ser_c}
clean:
	rm -f ${cliName} ${serName}
