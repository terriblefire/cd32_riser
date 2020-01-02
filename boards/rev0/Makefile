REVISION=0
BOARD=cd32_riser
BOARDPREFIX=$(BOARD)r$(REVISION)

RTLFOLDER=../../rtl/
RTLSOURCES="$(RTLFOLDER)/main_top.v $(RTLFOLDER)/user_io.v"

all: clean $(BOARD)r$(REVISION)
$(BOARD)r$(REVISION):
	make -f ../Makefile.cpld BOARD=$(BOARD) SOURCES=$(RTLSOURCES) SUBPROJ=main REVISION=$(REVISION) OPTMODE=speed OPTIMISE=speed TOP=main_top
zip: distclean $(BOARD)r$(REVISION)
	zip $(BOARD)r$(REVISION)_`date +"%Y_%m_%d"`_alpha.zip *.jed
clean:	
	rm -rf work _xmsgs *.zip xlnx_auto_*
distclean: clean
	rm -f *.jed *~ *.svf 
