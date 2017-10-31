TGT=edns_dial
SRCS=$(wildcard ./src/*.cpp)

LIBRAYS= -lrt -pthread -lssl -lcrypto -ldl -lz ./lib/*.a
#LIBRAYS= -lrt -pthread -lssl -lcrypto -ldl -lz ./lib/*.a ./lib/base_lib/libevent.a
#LIBRAYS= -lrt -pthread -ldl -lz ./lib/base_lib/*.a ./lib/*.a 
#LIBRAYS= -levent -lrt -pthread -lssl -lcrypto -ldl -lz -lthrift -lthriftnb ./lib/*.a

COMPILE_FLAGS= -g -W -O2 -DHAVE_NETINET_IN_H -I./include -I./clib/include -I/usr/include/openssl 
#COMPILE_FLAGS= -g -W -O2 -DHAVE_NETINET_IN_H -I./include -I./clib/include -I/usr/include/glib-2.0 -I/usr/lib64/glib-2.0/include/ -I/usr/include/openssl 

CC=g++

all:$(TGT)
	@echo Generation target!	

$(TGT):$(SRCS:.cpp=.o)
	$(CC) -o $@ $^ $(LIBRAYS) $(COMPILE_FLAGS) 
	
%.o : %.cpp
	$(CC) -c $(COMPILE_FLAGS) $< -o $@
	
.PHONY:	clean rpmclean 

clean:
	rm -rf $(TGT) $(SRCS:.cpp=.o)


RPM_VERSION = 1.1.1.9
COMMIT = $(shell git rev-list HEAD |head -1|cut -c 1-6)
#RPM_RELEASE = $(shell git branch --no-color 2> /dev/null | sed -e '/^[^*]/d' -e 's/* \(.*\)/\1/' -e 's/-/_/g')_$(COMMIT)
RPM_RELEASE = enterprise
RPM_TOP_DIR = $(shell rpm -E %{_topdir})
PRJHOME = $(shell pwd)

rpm:
	@echo [RPM] ; \
    	sed -e "s/@VERSION@/$(RPM_VERSION)/g" -e "s/@RELEASE@/$(RPM_RELEASE)/g" $(TGT).spec.tmp > ${RPM_TOP_DIR}/SPECS/$(TGT).spec ; \
    	cp -a -r ${PRJHOME} /tmp/$(TGT)-$(RPM_VERSION) ; \
    	cd /tmp ; \
    	tar zcvf $(RPM_TOP_DIR)/SOURCES/$(TGT)-$(RPM_VERSION).tar.gz $(TGT)-$(RPM_VERSION) ; \
    	rm -rf $(TGT)-$(RPM_VERSION) ; \
    	rpmbuild -bb $(RPM_TOP_DIR)/SPECS/$(TGT).spec ; \

rpmclean:	
	cp -r ~/rpmbuild/RPMS/x86_64/$(TGT)*$(RPM_VERSION)* ./  
	rm -rf ~/rpmbuild/SOURCES/$(TGT)* \
	~/rpmbuild/BUILD/$(TGT)* \
	~/rpmbuild/RPMS/x86_64/$(TGT)* \
	~/rpmbuild/SPEC/$(TGT)* 



