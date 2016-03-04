CC	=	g++

TARGET	=	main

$(TARGET)	:	main.cpp
	$(CC) -o $(TARGET) main.cpp -L. -lsvelts -std=c++11
	
all	:	$(TARGET)

clean	:
	rm -fr $(TARGET)
