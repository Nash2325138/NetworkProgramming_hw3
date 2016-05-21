CC=g++ -std=c++11 -Wall
ser_c=server_hw3.cpp
cli_c=client_hw3.cpp
cliName=HW3_103062224_Cli
serName=HW3_103062224_Ser
all: ${ser_c} ${cli_c}
	${CC} -o ${cliName} ${cli_c} -lpthread
	${CC} -o ${serName} ${ser_c} -lpthread
client: ${cli_c}
	${CC} -o ${cliName} ${cli_c} -lpthread
server: ${ser_c}
	${CC} -o ${serName} ${ser_c} -lpthread
clean:
	rm -f ${cliName} ${serName}
