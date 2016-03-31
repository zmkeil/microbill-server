#COMAKE2 edit-mode: -*- Makefile -*-
####################64Bit Mode####################
ifeq ($(shell uname -m),x86_64)
CC=gcc
CXX=g++
CXXFLAGS=-g \
  -pipe \
  -W \
  -Wall \
  -fPIC \
  -std=c++11
CFLAGS=-g \
  -pipe \
  -W \
  -Wall \
  -fPIC
CPPFLAGS=-g \
  -O2 \
  -pipe \
  -W \
  -Wall \
  -fPIC \
  -Wno-deprecated \
  -D__const__=
#  -Werror 
INCPATH=-I. \
  -I./include \
  -I../../../common/ \
  -I../../../nrpc/output/include/ \
  -I../../../thirdsrc/gtest-1.7.0/include
LIBPATH=-L. \
  -L../../../thirdsrc/gtest-1.7.0/cmake-build \
  -L../../../common \
  -L../../../nrpc/output/lib/ \
  -L/usr/local/lib \
  -L/usr/lib64/mysql
LDFLAGS=-lgtest -lgtest_main -lprotobuf -lpthread -lssl -lcrypto -ldl -lmysqlclient_r -lz -lngxrpc -lcommon


######### files #########################################

PROTO=../proto/bill \
	  ../proto/bill_config

objs=pending_events.o \
	 mysql_client.o \
	 db_helper.o \
	 user_manager.o \
	 bill_service_impl.o \
	 main.o \
	 bill.pb.o \
	 bill_config.pb.o

.PHONY:all
all:pb_c libmicrobill.a bill UT
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mall[0m']"
	@echo "make all done"

.PHONY:clean
clean:
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mclean[0m']"
	rm -rf $(objs)
	rm -rf libmicrobill.a
	rm -rf bill
	make -C test clean

.PHONY:love
love:
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mlove[0m']"
	@echo "make love done"

pb_c:
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mpb_c[0m']"
	@for pb in $(PROTO); \
	do \
		if [ -f $$pb.pb.cc ]; then \
			timeold=`stat $$pb.pb.cc | grep -i Modify`; \
			timenew=`stat $$pb.proto | grep -i Modify`; \
			if [ timenew -lt timeold ]; then \
				echo "protoc --cpp_out=./ $$pb.proto"; \
				( protoc --cpp_out=./ $$pb.proto ) || exit 1; \
			fi \
		else \
			echo "protoc --cpp_out=./ --proto_path=../proto/ $$pb.proto"; \
			( protoc --cpp_out=./ --proto_path=../proto/ $$pb.proto ) || exit 1; \
		fi \
	done

libmicrobill.a:$(filter-out main.o, $(objs))
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mlibmicrobill.a[0m']"
	ar rcs libmicrobill.a $^

bill:$(objs)
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mbill[0m']"
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LIBPATH) $^ -o $@ $(LDFLAGS)

# obscure rules
$(filter %.pb.o,$(objs)): %.o: %.cc
	$(CXX) -c $(INCPATH) $(CPPFLAGS) $(CXXFLAGS) $< -o $@

$(filter-out %.pb.o,$(filter %.o,$(objs))): %.o : %.cpp
	$(CXX) -c $(INCPATH) $(CPPFLAGS) $(CXXFLAGS) $< -o $@

# UT
UT:
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mtest[0m']"
	make -C test all

endif #ifeq ($(shell uname -m),x86_64)


