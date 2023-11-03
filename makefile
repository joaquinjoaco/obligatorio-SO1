# todo: main.o sistema.o arbolg.o lista.o
# 	g++ -o main main.o sistema.o arbolg.o lista.o

# main.o: main.cpp sistema.h arbolg.h lista.h
# 	g++ -c main.cpp

# sistema.o: sistema.cpp sistema.h arbolg.h lista.h
# 	g++ -c sistema.cpp

# arbolg.o: arbolg.cpp arbolg.h
# 	g++ -c arbolg.cpp

# lista.o: lista.cpp lista.h
# 	g++ -c lista.cpp

# limpiar:
# 	rm -f *.o
# 	rm -f main