
include proto.mk

define gen_proto
	@for file in `ls *proto`; do \
	(protoc --cpp_out=../pb $$file) \
	done;
endef

define make_subdir
	@for subdir in $(SUBDIRS); do \
	( \
	echo "enter directory " $$subdir && \
	cd $$subdir && \
	$(call gen_proto) && \
	echo "leave directory " $$subdir && \
	cd - \
	) \
	done;
endef


RM = rm -vrf

.PHONY : all clean

all: $(PBOUTPUTDIR)
	+$(call make_subdir)
	
$(PBOUTPUTDIR):
	@mkdir $@
	
clean:
	@cd $(PBOUTPUTDIR) && $(RM) *pb*
	
