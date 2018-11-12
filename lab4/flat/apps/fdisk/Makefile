default:
	cd fdisk; make

clean:
	cd fdisk; make clean

run:
	cd ../../os; make; cd ../bin; dlxsim -x os.dlx.obj -a -D F -u fdisk.dlx.obj; ee469_fixterminal
