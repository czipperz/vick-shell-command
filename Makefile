CFLAGS=-std=c++11 -I../../src -I../../test -Isrc
LDFLAGS=`find ../../out -type f -not \( -name main.o -o -name configuration.o \)` \
        -lncurses ../../testout/test_main.o
        # `find` object files for dependencies here (put at end of LDFLAGS).
        # Keep ``s so it is shell command.
        # example for vick-move:
        #
        # `find ../vick-move/out -type f`
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

begin:
	# Install dependencies here.
	# Example for vick-move:
	#
	# [ -d ../vick-move ] || git clone "https://github.com/czipperz/vick-move" ../vick-move
	# cd ../vick-move && make begin && make
	#
	# Ensure that you call ``make begin`` AND ``make`` on it!
	# It may be built after yours, don't make a data race

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
	@mkdir -p $T
	@touch $T/blank

test: ${files} ${testfiles} $T/blank
	@rm $T/blank
	@mkdir -p $T
	${CXX} -o $T/out ${files} ${testfiles} ${CFLAGS} ${LDFLAGS} ../../src/configuration.cc -Dtesting
	./$T/out

tags:
	etags `find src -name '*.cc'`
