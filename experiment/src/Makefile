CC=clang
CFLAGS:= -O2 -Wall -fPIC -g

BASIC_DIR=./lib/basics
EVSET_DIR=./lib/eviction_set

SUBDIRS=$(BASIC_DIR) $(EVSET_DIR)

BUILD_SUBDIRS=$(SUBDIRS:%=build-%)
CLEAN_SUBDIRS=$(SUBDIRS:%=clean-%)

OBJECTS=$(shell find ./lib -name '*.o')
EXECS=$(shell find . -name '*.out')
ASMS=$(shell find . -name '*.s')
LDFLAGS=-lm

# $(error   OBJECTS is $(OBJECTS))

all: evset_gen.out quick_check.out \
	miss_rate.out blinding.out

profile: evset_gen.out

quick: quick_check.out

miss: miss_rate.out

blind: blinding.out

%.out: %.o $(BUILD_SUBDIRS)
	$(CC) $(LDFLAGS) -o $@ $< $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) $^ -c -o $@

$(BUILD_SUBDIRS):
	$(MAKE) -C $(@:build-%=%)

clean: $(CLEAN_SUBDIRS)
	rm -rf $(OBJECTS) $(EXECS) $(ASMS)

$(CLEAN_SUBDIRS):
	$(MAKE) clean -C $(@:clean-%=%)

