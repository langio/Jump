
PBOUTPUTDIR = pb

RM = rm -vrf

.PHONY : all clean

all: $(PBOUTPUTDIR)
	./gen_pb.sh
	
$(PBOUTPUTDIR):
	@mkdir $@
	
clean:
	@cd $(PBOUTPUTDIR) && $(RM) *pb*
	
