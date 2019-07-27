INCLUDE = -I. -I.. -I../..
LIBRARY = -L../../libtinyredis

CXXFLAGS    = -Wall -ggdb -pthread -D__DEBUG__
CXXFLAGS_R    = -Wall -O2 -O3 -pthread

LINKFLAGS   = -Wl,-rpath,./:../bin
LINKFLAGS_MT = $(LINKFLAGS)

CXXFLAGS_EXTERN = -DTHREAD
LINKFLAGS_EXTERN = libserverfinder.a -ltinyredis_mt -lhiredis -ljsoncpp -lboost_thread

TARGET   = libserverfinder.a
TARGET_R   = libserverfinder.ra

SRC_FILES   = redis.cpp push.cpp pull.cpp container.cpp

OBJ_FILES   = $(SRC_FILES:.cpp=.o)
OBJ_FILES_R   = $(SRC_FILES:.cpp=.ro)

$(TARGET) : $(OBJ_FILES)
	ar -rus -o $@ $(OBJ_FILES)
	
$(TARGET_R) : $(OBJ_FILES_R)
	ar -rus -o $@ $(OBJ_FILES_R)

TARGET_SAMPLE = sample
SRC_SAMPLE_FILES   = sample.cpp
OBJ_SAMPLE_FILES = $(SRC_SAMPLE_FILES:.cpp=.o)

$(TARGET_SAMPLE) : $(OBJ_SAMPLE_FILES)
	g++ -o $@ $(OBJ_SAMPLE_FILES) $(CXXFLAGS) $(LINKFLAGS) $(LINKFLAGS_EXTERN) $(LIBRARY)

%.o : %.cpp
	g++ -c -o $@ $< $(CXXFLAGS) $(CXXFLAGS_EXTERN) $(INCLUDE)
%.ro : %.cpp
	g++ -c -o $@ $< $(CXXFLAGS_R) $(CXXFLAGS_EXTERN) $(INCLUDE)

.PHONY : all clean

all : $(TARGET) $(TARGET_R) $(TARGET_SAMPLE)

clean :
	-rm $(OBJ_FILES) $(OBJ_FILES_R) 
	-rm $(TARGET) $(TARGET_R) 
	-rm $(OBJ_SAMPLE_FILES)
