
PBOUTPUTDIR = pb

RM = rm -vrf

.PHONY : all clean

all: $(PBOUTPUTDIR)
	./gen_pb.sh
	$(MAKE) -f Makefile.real $@
	
$(PBOUTPUTDIR):
	@mkdir $@
	
clean:
	@cd $(PBOUTPUTDIR) && $(RM) *pb*
	
