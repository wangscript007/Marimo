PROJECT_DIR = .
CLSTD_DIR = ../..
LIB_DIR=$(CLSTD_DIR)\Bin

INCLUDES += -I$(PROJECT_DIR) -I$(CLSTD_DIR)
CPPFLAGS += -D_DEBUG -D_CL_SYSTEM_LINUX -DPOSIX_THREAD -D_CRT_FINDFILE -D_CL_ENABLE_ICONV -fpermissive -std=gnu++11 -fexec-charset=GBK -g
CXX := g++


LDFLAGS += -L$(CLSTD_DIR) -lc -lclstd -lpthread
#LIBSRCS=\
#	clString.cpp
	
OBJS=\
	Sample_Storage.o
	

	
all:$(OBJS)
#	$(LD) -o test.out $(OBJS) $(LDFLAGS)
	
Sample_Storage.o : Sample_Storage.cpp $(CLSTD_DIR)/libclstd.a
	$(CXX) $(INCLUDES) $(CFLAGS) $(CPPFLAGS) $< -o $@ $(LDFLAGS)

clean:
	-rm $(OBJS)