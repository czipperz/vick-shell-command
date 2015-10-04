CFLAGS=-std=c++11 -I../../src -I../../test -Isrc
LDFLAGS=`find ../../out -type f -not \( -name main.o -o -name configuration.o \)` \
        -lncurses
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

# we use ``catch'' as the framework as it is amazing
# https://github.com/philsquared/Catch
#
# Example:
#
# testfiles=${TO}/move_tests.o
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
	[ ! -d $O ] || rm -R $O
	[ -z "`find -name '*~'`" ] || rm `find -name '*~'`
	[ ! -d ${TO} ] || rm -R ${TO}

$T/blank:
	@touch $T/blank

test: ${files} ${testfiles} $T/blank
	@rm $T/blank
	@mkdir -p $T
	${CXX} -o $T/out ${files} ${testfiles} ${CFLAGS} ${LDFLAGS} ../../src/configuration.cc -Dtesting
	./$T/out

tags:
	etags `find src -name '*.cc'`
