default:
	cd makeprocs; make
	cd producer; make
	cd consumer; make

clean:
	cd makeprocs; make clean
	cd producer; make clean
	cd consumer; make clean

run:
	cd ../../bin; dlxsim -x os.dlx.obj -a -u makeprocs.dlx.obj 4; ee469_fixterminal
