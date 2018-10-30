default:
	cd makeprocs; make

clean:
	cd makeprocs; make clean

run:
	cd ../../bin; dlxsim -x os.dlx.obj -a -u makeprocs.dlx.obj; ee469_fixterminal

drun:
	cd ../../bin; dlxsim -D m -x os.dlx.obj -a -D m -u makeprocs.dlx.obj; ee469_fixterminal
