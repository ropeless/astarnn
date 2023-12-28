# Makefile for AStarNN

TOP         = $(shell pwd)
LIBNAME     = AStarNN
SRC_DIR     = $(TOP)/src
PATHED_SRCS = $(wildcard  $(SRC_DIR)/*.cpp)
PATHED_HDRS = $(wildcard  $(SRC_DIR)/*.h)
INSTALL     = $(TOP)/../AStarNN_py

SRCS = $(patsubst $(SRC_DIR)/%, %, $(PATHED_SRCS))
HDRS = $(patsubst $(SRC_DIR)/%, %, $(PATHED_HDRS))

BUILD_DIR_R32 = $(TOP)/Release32
BUILD_DIR_R64 = $(TOP)/Release64
BUILD_DIR_D32 = $(TOP)/Debug32
BUILD_DIR_D64 = $(TOP)/Debug64

SYS = $(shell if test `uname` = 'Darwin'; then echo 'mac'; else echo 'lin'; fi)

RELEASE_FLAGS = -O2 -DNDEBUG -fno-strict-aliasing
DEBUG_FLAGS   = -g
LDFLAGS       = -shared -ldl -fPIC -rdynamic

CXXFLAGS += -std=c++11

SHARE_R32 = $(BUILD_DIR_R32)/$(LIBNAME).so
SHARE_R64 = $(BUILD_DIR_R64)/$(LIBNAME).so
SHARE_D32 = $(BUILD_DIR_D32)/$(LIBNAME).so
SHARE_D64 = $(BUILD_DIR_D64)/$(LIBNAME).so

SHARE_OBJS_R32 = $(patsubst %.cpp, $(BUILD_DIR_R32)/%.o, $(SRCS))
SHARE_OBJS_R64 = $(patsubst %.cpp, $(BUILD_DIR_R64)/%.o, $(SRCS))
SHARE_OBJS_D32 = $(patsubst %.cpp, $(BUILD_DIR_D32)/%.o, $(SRCS))
SHARE_OBJS_D64 = $(patsubst %.cpp, $(BUILD_DIR_D64)/%.o, $(SRCS))



.PHONY: all install clean build_R64 build_D64



all : build_R64 build_D64

install : build_R64 build_D64
	cp $(SHARE_R64) $(INSTALL)/$(LIBNAME)_$(SYS)64.so
	cp $(SHARE_D64) $(INSTALL)/$(LIBNAME)_$(SYS)64d.so

clean :
	rm -rf $(BUILD_DIR_D32)
	rm -rf $(BUILD_DIR_D64)
	rm -rf $(BUILD_DIR_R32)
	rm -rf $(BUILD_DIR_R64)


build_R32: $(BUILD_DIR_R32) $(SHARE_R32)
build_R64: $(BUILD_DIR_R64) $(SHARE_R64)
build_D32: $(BUILD_DIR_D32) $(SHARE_D32)
build_D64: $(BUILD_DIR_D64) $(SHARE_D64)


$(BUILD_DIR_R32) :
	mkdir -p $(BUILD_DIR_R32)
$(BUILD_DIR_R64) :
	mkdir -p $(BUILD_DIR_R64)
$(BUILD_DIR_D32) :
	mkdir -p $(BUILD_DIR_D32)
$(BUILD_DIR_D64) :
	mkdir -p $(BUILD_DIR_D64)

$(SHARE_R32) : $(SHARE_OBJS_R32)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -m32 -o $@ $^
$(SHARE_R64) : $(SHARE_OBJS_R64)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -m64 -o $@ $^
$(SHARE_D32) : $(SHARE_OBJS_D32)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -m32 -o $@ $^	
$(SHARE_D64) : $(SHARE_OBJS_D64)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -m64 -o $@ $^


$(BUILD_DIR_R32)/%.o : $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -m32 -I$(SRC_DIR) -fPIC -c $(RELEASE_FLAGS) -o $@ $<

$(BUILD_DIR_R64)/%.o : $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -m64 -I$(SRC_DIR) -fPIC -c $(RELEASE_FLAGS) -o $@ $<

$(BUILD_DIR_D32)/%.o : $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -m32 -I$(SRC_DIR) -fPIC -c $(DEBUG_FLAGS) -o $@ $<

$(BUILD_DIR_D64)/%.o : $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -m64 -I$(SRC_DIR) -fPIC -c $(DEBUG_FLAGS) -o $@ $<
