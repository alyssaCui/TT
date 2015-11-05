########################################################################
#                                                                      #
#   Makefile template file                                             #
#   COMPONENT:   Makefile for TT                                      #
#   DATE:        2015/01/18                                            #
#   CREATOR:                                                           #
#                                                                      #
########################################################################

# DEBUG can be set to YES to include debugging info, or NO otherwise
DEBUG          := YES
PLATFORM    = LINUX

CXX    := g++
LD     := g++
AR     := ar 

CPP         = g++ $(DBGFLAG) -c -Wall   
LINK        = g++ $(DBGFLAG)
LINK_SO     = g++ $(DBGFLAG) -shared  -fPIC


DEBUG_CXXFLAGS     := -Wall -g
RELEASE_CXXFLAGS   := -Wall

DEBUG_LDFLAGS    := -g
RELEASE_LDFLAGS  :=

ifeq (YES, ${DEBUG}) 
   CXXFLAGS     := ${DEBUG_CXXFLAGS}
   LDFLAGS      := ${DEBUG_LDFLAGS}
else
   CXXFLAGS     := ${RELEASE_CXXFLAGS}
   LDFLAGS      := ${RELEASE_LDFLAGS}
endif

ifeq ($(PLATFORM), LINUX)
	CXXFLAGS   += -DLINUX
	LDFLAGS += -DLINUX
endif

ARFLAGS     = -rv 

####################################################################
# definition for directory
PROJECTNAME = tt

MAIN_DIR    = $(TTROOT)
PROJ_DIR    = $(MAIN_DIR)/code/apps
FRAME_DIR   = $(MAIN_DIR)/code/frame
SO_DIR      = $(MAIN_DIR)/code/so_simu
CTP_SO_DIR  = $(SO_DIR)/ctp_so_simu
TINYXML_DIR = $(MAIN_DIR)/code/tinyxml
GTEST_DIR   = /home/cuiju/gtest

OBJ_DIR     = $(MAIN_DIR)/build/obj
BIN_DIR     = $(MAIN_DIR)/bin

INC_FLAG = -I$(PROJ_DIR)/inc -I$(CTP_SO_DIR)/inc -I$(FRAME_DIR)/inc -I$(TINYXML_DIR)/inc
LDFLAGS = -L. -lpthread  
#LDFLAGS = -L. -lpthread -L$(GTEST_DIR) -lgtest 

CPPFLAGS += -isystem $(GTEST_DIR)/include

NORMAL_OBJS = 

SRCS = $(wildcard $(PROJ_DIR)/src/*.cpp $(PROJ_DIR)/src/*.c) $(PROJ_DIR)/src/main.cpp $(wildcard $(FRAME_DIR)/src/*.cpp $(FRAME_DIR)/src/*.c)
#OBJS = $(subst src,obj,$(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SRCS)))) 
#OBJS = $(wildcard *.o)
TARGET_LIB  = 
TARGET_EXE	= $(PROJECTNAME)

####################################################################

all:  clean MK_DIR  $(TARGET_EXE)  install

MK_DIR:
	@echo $(PROJ_DIR)
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(BIN_DIR)

$(TARGET_EXE): OBJS_STEP libtinyxml.a
	$(LINK) -o $@ *.o $(LDFLAGS) libtinyxml.a

OBJS_STEP:
	$(CPP) $(CXXFLAGS) $(SRCS) $(INC_FLAG) $(CPPFLAGS)
	
install:
	mv $(TARGET_EXE) $(MAIN_DIR)/bin/ 

clean:
	rm -f $(TARGET_EXE) $(TARGET_LIB)
	rm -f *.o
	
libtinyxml.a: tinyxml.o tinystr.o tinyxmlerror.o tinyxmlparser.o
	$(AR) $(ARFLAGS) $@ $^

tinyxml.o: $(TINYXML_DIR)/src/tinyxml.cpp \
			$(TINYXML_DIR)/inc/*.h
	$(CXX) $(CXXFLAGS) -I$(TINYXML_DIR)/inc/ -c $(TINYXML_DIR)/src/tinyxml.cpp
	
tinystr.o: $(TINYXML_DIR)/src/tinystr.cpp \
			$(TINYXML_DIR)/inc/*.h
	$(CXX) $(CXXFLAGS) -I$(TINYXML_DIR)/inc/ -c $(TINYXML_DIR)/src/tinystr.cpp
	
tinyxmlerror.o: $(TINYXML_DIR)/src/tinyxmlerror.cpp \
			$(TINYXML_DIR)/inc/*.h
	$(CXX) $(CXXFLAGS) -I$(TINYXML_DIR)/inc/ -c $(TINYXML_DIR)/src/tinyxmlerror.cpp
	
tinyxmlparser.o: $(TINYXML_DIR)/src/tinyxmlparser.cpp \
			$(TINYXML_DIR)/inc/*.h
	$(CXX) $(CXXFLAGS) -I$(TINYXML_DIR)/inc/ -c $(TINYXML_DIR)/src/tinyxmlparser.cpp
	

