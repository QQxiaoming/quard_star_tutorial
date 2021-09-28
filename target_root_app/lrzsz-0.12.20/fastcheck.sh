#! /bin/sh

srcdir="$1"
if test $srcdir = . ; then
	srcdir=`pwd`
fi
if test $srcdir = .. ; then
	srcdir=`pwd`/..
fi
objdir="$2"
if test $objdir = . ; then
	objdir=`pwd`
fi
testdir=$objdir/fastcheck.lrzsz

SZ="$objdir/src/lsz"
RZ="$objdir/src/lrz"

echo checking with srcdir = $1 and objdir = $2

z_test_files=""
for i in $srcdir/src/l?z.c ; do
	z_test_files="$z_test_files $i" 
done
for i in $objdir/src/l?z ; do
	z_test_files="$z_test_files $i" 
done

# change to tmp dir
if test "x$TMPDIR" = x ; then
	if test "x$TMP" = x ; then
		cd /tmp
	else
		cd $TMP || cd /tmp
	fi
else
	cd $TMPDIR || cd /tmp
fi

rm -rf $testdir
mkdir $testdir
exec 5>$testdir/error.log
(mkfifo $testdir/pipe || mknod $testdir/pipe p) 2>&5

mkdir $testdir/zmodem
failed=0
($SZ -q $z_test_files ) <$testdir/pipe | \
	(cd $testdir/zmodem ; exec $RZ $QUIET >>../pipe )
for i in $z_test_files ; do 
	bn=`basename $i`
	cmp $i $testdir/zmodem/$bn
	if test $? -eq 0 ; then
		rm -f $testdir/zmodem/$bn
	else
		failed=1
	fi
done
rm -rf $testdir

if test "x$failed" = x0  ; then
	:
else
	echo "the test failed." >&2
	echo "use 'make check' or 'make vcheck' for a more detailed test" >&2
	touch $objdir/fastcheck.failed
	exit 1
fi


touch $objdir/fastcheck.ok
exit 0

