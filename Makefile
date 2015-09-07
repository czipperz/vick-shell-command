CFLAGS=-std=c++11 -I../../src -I../../test -Isrc
LDFLAGS=`find ../../out -type f -not \( -name main.o -o -name configuration.o \)` \
        ../../testout/main.o -lncurses
O=out
S=src
T=test
TO=testout
CXX=clang++

# files in form ``$O/file.o'' format
#
# Example:
#
# files=$O/move.o        \
#       $O/newmove.o
files=

# set ot tests : we use ``catch'' as the framework as it is incredibly
# well designed.  https://github.com/philsquared/Catch
testfiles=

all: ${files}

$O/%.o: $S/%.cc $S/%.hh
	@mkdir -p $O
	${CXX} -o $@ -c $< ${CFLAGS}

$O/%.o: $S/%.cc
	@mkdir -p $O
	${CXX} -o $@ -c $< ${CFLAGS}

${TO}/%.o: $T/%.cc
	@mkdir -p ${TO}
	${CXX} -o $@ -c $< ${CFLAGS}

clean:
	[ ! -d out ] || rm -R out
	[ -z "`find -name '*~'`" ] || rm `find -name '*~'`

cleantest:
	rm -R ${TO}

test: ${files} ${testfiles}
	@mkdir $T
	${CXX} -o $T/out $^ ${CFLAGS} ${LDFLAGS}
	./$T/out

tags:
	etags `find src -name '*.cc'`
