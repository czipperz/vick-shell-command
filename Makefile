CFLAGS=-std=c++11 -I../../src -I../../test -Isrc
LDFLAGS=`find ../../out -type f -not \( -name main.o -o -name configuration.o \)` \
        ../../testout/test_main.o

# `find` object files for dependencies here (put in LDLIBS).
# Keep ``s so it is shell command.
# example for vick-move:
#
# `find ../vick-move/out -type f`
LDLIBS=

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
	git pull
	# Install dependencies here.
	# Example for vick-move:
	#
	# [ -d ../vick-move ] || git clone "https://github.com/czipperz/vick-move" ../vick-move
	# cd ../vick-move && make begin
	#
	# Ensure that you call ``make begin`` on it so that it gets
	# its dependencies!

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

test: ${files} ${testfiles}
	@mkdir -p ${TO}
	${CXX} -o ${TO}/out $^ ${CFLAGS} ${LDFLAGS} \
            ../../src/configuration.cc -Dtesting
	./${TO}/out

# PHONY ensures that commands aren't file generators
.PHONY: all begin clean test
