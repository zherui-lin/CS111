CXX = g++ $(ARCH) -std=c++17
CXXFLAGS = -ggdb -Wall -Werror
STACK_OBJS = stack_init.o stack_switch.o timer.o

all: lambda threads coroutine threadlet preempt

lambda: lambda.o $(STACK_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

threads: threads.o
	$(CXX) $(CXXFLAGS) -o $@ $< -lpthread

coroutine: coroutine.o $(STACK_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

threadlet: threadlet.o $(STACK_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

preempt: preempt.o $(STACK_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

#procedure: procedure.cc Makefile
#	$(CXX) $(CXXFLAGS) -static -fno-pie -o $@ $<
#procedure.s: procedure
#	objdump -S procedure > procedure.s

#%.s: %.cc
#	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -S $<

clean:
	rm -f lambda threads coroutine threadlet preempt *.o *.s *~ .*~

.PHONY: all clean
