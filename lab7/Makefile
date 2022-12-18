PROGS = paging
CXX = g++ $(ARCH) -std=c++17
CXXFLAGS = -ggdb -Wall -Werror

PROJ = cmap
PROJ_URL = https://web.stanford.edu/class/cs111/starters/$(PROJ).git
PROJ_SRCS := cmap/cryptfile.cc cmap/crypto.cc cmap/mcryptfile.cc cmap/vm.cc
PROJ_OBJS := $(PROJ_SRCS:.cc=.o)

CPPFLAGS := -I$(PROJ) $$(pkg-config --cflags libcrypto)
LDLIBS := $$(pkg-config --libs libcrypto)

all:: $(PROGS)

paging: paging.cc $(PROJ_OBJS)

$(PROJ_OBJS): $(PROJ)
	make -C $(PROJ) $(@F)

$(PROJ):
	git clone $(PROJ_URL)

clean::
	rm -f $(PROGS) *.o *.s *~ .*~ *.enc *.clear
	rm -fr cmap

spartan:: clean

.PHONY: all clean spartan
