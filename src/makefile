OBJS = shell.cpp builtins.cpp
NAME = myshell

all: $(NAME)

myshell: $(OBJS)
	g++ $(OBJS) -g -l readline -o $(NAME)

clean:
	rm -rf $(NAME)
