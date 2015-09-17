all: master worker

master:
	gcc Master.c CmdHandler2.c StringManip.c Runner.c -g -o master.out -ldl -lpthread
worker:
	gcc Worker.c CmdHandler2.c StringManip.c Runner.c -g -o worker.out -ldl -lpthread
clearCache:
	rm Cache/temp.so Cache/temp.txt
