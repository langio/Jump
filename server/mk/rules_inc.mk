#包含文件需要指定下面几个参数
TARGET ?= 
SRC_DIR_RECURSION ?= Y
SRC_FILE_DIRS += 
OTHER_SRC_FILES += 
INCLUDE += 
RECURSION_INCLUDE +=
LIBS += 

XML_SEARCH_DIR +=
XML2H +=
XML2C += 
XML2CPP += 
XML2TDR +=
JCE_DIRS += 
OTHER_JCE_FILES +=

#以下几个可选，以下为默认指定目录
XML2H_DIR ?= $(PROTO_GEN_DIR)/xml2h
XML2C_DIR ?= $(PROTO_GEN_DIR)/xml2c
XML2CPP_DIR ?= $(PROTO_GEN_DIR)/xml2cpp
XML2TDR_DIR ?= $(PROTO_GEN_DIR)/xml2tdr
JCE2CPP_DIR ?= $(PROTO_GEN_DIR)/jce2cpp

#make目标之前需要提前make的目录或makefile
NEED_MAKE_DEP ?= N
DEP_MAKE_SUBDIRS +=
RECURSION_DEP_MAKE_SUBDIRS +=
DEP_MAKE_MAKEFILES += 
#目标需要额外检测依赖的其它对象
TARGET_DEP_OBJS +=

#中间文件保存目录
BUILD_DIR ?=

#make run运行命令
RUN_CMD ?=

#单元测试，make unittest/rununittest
UNITTEST_DIRS +=
UNITTEST_MAKEFILES +=

JCE2CPP_FLAGS +=

#------------------------------------------------------------------------------------
COMPILER ?= g++
AR = ar
RM = rm -vrf
TDR ?= $(TSF4G_INSTALL_HOME)/tools/tdr
JCE2CPP_TOOL ?= $(shell cd $(JCE_DIR)/bin; pwd)/jce2cpp
PROTO_GEN_DIR ?= protogen
PROTO_DEP_DIR ?= $(PROTO_GEN_DIR)
ifneq ($(strip $(BUILD_DIR)),)
	PROTO_DEP_DIR = $(BUILD_DIR)
endif

CFLAGS += -g -Wall -Wno-deprecated -pipe -fPIC -D_GNU_SOURCE -D_REENTRANT
#CFLAGS += -pg -Wextra -D_NEW_LIC

#SHARED_FLAGS = -shared -z defs -fPIC 
SHARED_FLAGS += -shared -z defs 

#------------------------------------------------------------------------------------

ifneq ($(strip $(SRC_DIR_RECURSION)),N)
	ALL_SOURCE_FILES += $(foreach D,$(SRC_FILE_DIRS),$(shell find $(D) -name "*.cpp" -o -name "*.cc" -o -name "*.c" 2> /dev/null))
else
	ALL_SOURCE_FILES += $(foreach D,$(SRC_FILE_DIRS),$(shell ls $(D)/*.cpp $(kD)/*.cc $(D)/*.c 2> /dev/null))
endif

ALL_SOURCE_FILES += $(OTHER_SRC_FILES)

ifneq ($(strip $(BUILD_DIR)),)
	TMP_OBJS = $(patsubst %.cpp,%.o,$(patsubst %.cc,%.o, $(patsubst %.c,%.o, $(ALL_SOURCE_FILES))))
	ALL_OBJS += $(addprefix $(BUILD_DIR)/, $(notdir $(TMP_OBJS)))
else
	ALL_OBJS += $(patsubst %.cpp,%.o,$(patsubst %.cc,%.o, $(patsubst %.c,%.o, $(ALL_SOURCE_FILES))))
endif

INCLUDE += -I$(XML2H_DIR) -I$(XML2CPP_DIR) -I$(JCE2CPP_DIR)
INCLUDE += $(foreach D,$(RECURSION_INCLUDE),$(shell find $(D) -path '*/.svn' -prune -o -type d -exec echo "-I"{} \; 2> /dev/null))

CLEANOBJS += $(TARGET) $(ALL_OBJS) $(SRC_DEP_FILE)

SRC_JCE_FILES += $(OTHER_JCE_FILES)
SRC_JCE_FILES += $(foreach D,$(JCE_DIRS),$(shell find $(D) -name "*.jce" 2> /dev/null))
JCE_FILES = $(shell echo $(SRC_JCE_FILES) | awk '{for(i=1;i<=NF;++i){a[$$i]=1}}END{for(k in a){print k}}')
JCE_SEARCH_DIR += $(JCE_DIRS)
ABS_JCE2CPP_DIR = $(shell cd $(JCE2CPP_DIR); pwd)

XML2H_FILES = $(addprefix $(XML2H_DIR)/, $(notdir $(XML2H:.xml=.h)))
XML2C_FILES = $(addprefix $(XML2C_DIR)/, $(notdir $(XML2C:.xml=.c)))
XML2CPP_FILES = $(addprefix $(XML2CPP_DIR)/, $(notdir $(XML2CPP:.xml=.h)))
XML2TDR_FILES = $(addprefix $(XML2TDR_DIR)/, $(notdir $(XML2TDR:.xml=.tdr)))
JCE2CPP_FILES = $(addprefix $(JCE2CPP_DIR)/, $(notdir $(JCE_FILES:.jce=.h)))

XML2H_FILES_DEP = $(addprefix $(PROTO_DEP_DIR)/, $(notdir $(XML2H:.xml=.h.xml2h.d)))
XML2C_FILES_DEP = $(addprefix $(PROTO_DEP_DIR)/, $(notdir $(XML2C:.xml=.c.xml2c.d)))
XML2CPP_FILES_DEP = $(addprefix $(PROTO_DEP_DIR)/, $(notdir $(XML2CPP:.xml=.h.xml2cpp.d)))
XML2TDR_FILES_DEP = $(addprefix $(PROTO_DEP_DIR)/, $(notdir $(XML2TDR:.xml=.tdr.xml2tdr.d)))

XML_AND_JCE_GEN_FILES = $(XML2H_FILES) $(XML2C_FILES) $(XML2CPP_FILES) $(XML2TDR_FILES) $(JCE2CPP_FILES)
XML_AND_JCE_GEN_DIRS = $(XML2H_DIR) $(XML2C_DIR) $(XML2CPP_DIR) $(XML2TDR_DIR) $(JCE2CPP_DIR) $(PROTO_DEP_DIR)

CLEANOBJS += $(XML_AND_JCE_GEN_FILES) $(XML_DEP_FILE) $(JCE_DEP_FILE) $(PROTO_GEN_DIR) $(PROTO_DEP_DIR)

ifneq ($(strip $(BUILD_DIR)),)
	CLEANOBJS += $(BUILDDIR_RULES_FILE) $(BUILD_DIR)
endif

DEP_MAKE_SUBDIRS += $(foreach D,$(RECURSION_DEP_MAKE_SUBDIRS),$(shell find $(D) -mindepth 1 -maxdepth 1 -type d 2> /dev/null))

#------------------------------------------------------------------------------------
find_dep_xmls = $(shell \
    XMLS=`echo $(1)`; \
    while [[ ! -z "`echo -n $$XMLS`" ]]; \
    do \
    	DEPXMLS=$$DEPXMLS" "$$XMLS; \
    	TMPXMLS=$$XMLS; \
    	XMLS=; \
    	for f in `grep "<[ \t]*include " $$TMPXMLS 2> /dev/null | awk -F\" '{print $$2}' | xargs`; \
    	do \
    		XMLS=$$XMLS" "`find $(XML_SEARCH_DIR) -maxdepth 1 -name "$$f" | xargs`; \
    	done; \
    done; \
    echo $$DEPXMLS | awk '{for(i=1;i<=NF;++i){a[$$i]=1}}END{for(k in a){print k}}' | xargs; \
)

define compile_src_file
    $(COMPILER) $(CFLAGS) $(INCLUDE) -c $1 -o $2
endef	

define create_src_dep_file
    echo "update $2 ..."; \
    set -e; rm -f $2; \
    DCONTENT=`$(COMPILER) -MM $(INCLUDE) $1 | sed "s,.*\.o[ ]*:,$(2:.d=.o) $2 :,g"`; \
    echo "$$DCONTENT" > $2; \
    echo "$$DCONTENT" | sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
                -e '/^$$/d' -e 's/$$/ :/' >> $2; \
    if [[ -z `cat $2` ]]; then rm -f $2; fi;
endef

define create_xmldestfile_dep_file
	echo "update $2 ..."; \
	set -e; rm -f $2; \
	basefile=`basename $2`; \
	destfile=`echo "$(strip $3)/$${basefile%%.*}.$(strip $4)"`; \
	echo "$$destfile $2 : $1" >> $2; \
	if [[ -z `cat $2` ]]; then rm -f $2; fi;
endef

define create_builddir_rules_file 
    echo "update builddir_rules_file.ruled ..."; \
    mkdir -p $(BUILD_DIR); rm -f $(BUILDDIR_RULES_FILE); \
    for srcfile in $(ALL_SOURCE_FILES); do \
        obj="$(BUILD_DIR)/"`basename $${srcfile%.*}`".o"; \
        echo "$$obj : $$srcfile" >> $(BUILDDIR_RULES_FILE); \
        echo -e '\t$$(call compile_src_file, $$<, $$@)' >> $(BUILDDIR_RULES_FILE); \
        objd="$(BUILD_DIR)/"`basename $${srcfile%.*}`".d"; \
        echo "$$objd : $$srcfile" >> $(BUILDDIR_RULES_FILE); \
        echo -e '\t@$$(call create_src_dep_file, $$<, $$@)' >> $(BUILDDIR_RULES_FILE); \
    done;
endef

define make_subdir
	@for subdir in $1; do \
        (cd $$subdir && $(MAKE) $2) \
	done;
endef

define make_submakefile
	@for file in $1; do \
        ($(MAKE) $2 -f $$file) \
	done;
endef

#------------------------------------------------------------------------------------
vpath %.xml $(XML_SEARCH_DIR)
vpath %.jce $(JCE_SEARCH_DIR)

$(filter %.a,$(TARGET)) : $(ALL_OBJS) $(TARGET_DEP_OBJS)
	$(AR) rcs $@ $(ALL_OBJS) 
	ranlib $(TARGET)

$(filter %.so,$(TARGET)) : $(ALL_OBJS) $(TARGET_DEP_OBJS)
	$(COMPILER) $(CFLAGS) $(SHARED_FLAGS) $(ALL_OBJS) $(LIBS) -o $@
	
$(filter-out %.so %.a,$(TARGET)) : $(ALL_OBJS) $(TARGET_DEP_OBJS)
	$(COMPILER) $(CFLAGS) $(INCLUDE) $(ALL_OBJS) $(LIBS) -o $@

.PHONY: clean cleanall run unittest rununittest
clean:
	@$(RM) $(CLEANOBJS)

cleanall: clean
    ifneq ($(strip $(NEED_MAKE_DEP)),N)
	$(call make_subdir, $(DEP_MAKE_SUBDIRS), clean)
	$(call make_submakefile, $(DEP_MAKE_MAKEFILES), clean)
    endif

run: $(TARGET)
	$(call RUN_CMD)

unittest:
	+$(call make_subdir, $(UNITTEST_DIRS))
	+$(call make_submakefile, $(UNITTEST_MAKEFILES))

rununittest:
	+$(call make_subdir, $(UNITTEST_DIRS), run)
	+$(call make_submakefile, $(UNITTEST_MAKEFILES), run)

#------------------------------------------------------------------------------------
SRC_DEP_FILE = $(ALL_OBJS:.o=.d)

ifneq ($(strip $(MAKECMDGOALS)),clean)
ifneq ($(strip $(MAKECMDGOALS)),cleanall)
ifneq ($(strip $(MAKECMDGOALS)),unittest)
ifneq ($(strip $(MAKECMDGOALS)),rununittest)

#生成源文件依赖文件
ifneq ($(strip $(SRC_DEP_FILE)),)
-include $(SRC_DEP_FILE)
endif

#如果有指定编译中间目录，则需要提前生成相关编译命令文件
ifneq ($(strip $(BUILD_DIR)),)
BUILDDIR_RULES_FILE = $(BUILD_DIR)/builddir_rules_file.ruled
ifneq ($(strip $(BUILDDIR_RULES_FILE)),)
-include $(BUILDDIR_RULES_FILE)
endif
endif

ifneq ($(strip $(XML_AND_JCE_GEN_FILES)),)
#生成jce目标文件
JCE_DEP_FILE = $(PROTO_DEP_DIR)/.jce2cpp.jced
ifneq ($(strip $(JCE_DEP_FILE)),)
-include $(JCE_DEP_FILE)
endif

#生成xml目标文件
XML_DEP_FILE = $(PROTO_DEP_DIR)/.xml2xx.xmld
ifneq ($(strip $(XML_DEP_FILE)),)
-include $(XML_DEP_FILE)
endif

#生成xml目标文件依赖关系文件
XMLDESTFILE_DEP_XML = $(XML2H_FILES_DEP) $(XML2C_FILES_DEP) $(XML2CPP_FILES_DEP) $(XML2TDR_FILES_DEP)
ifneq ($(strip $(XMLDESTFILE_DEP_XML)),)
-include $(XMLDESTFILE_DEP_XML)
endif

#提前建立相关协议目录
PROTO_GEN_DIR_CREATE_DEP = .xml2xx.mkdir
ifneq ($(strip $(PROTO_GEN_DIR_CREATE_DEP)),)
-include $(PROTO_GEN_DIR_CREATE_DEP)
endif
endif

#执行makefile依赖关系
MAKEFILE_DEP_FILE = .make.makefiled
ifneq ($(strip $(NEED_MAKE_DEP)),N)
ifneq ($(strip $(MAKEFILE_DEP_FILE)),)
-include $(MAKEFILE_DEP_FILE)
endif
endif

endif
endif
endif
endif

#------------------------------------------------------------------------------------
#源文件编译规则
%.o:%.cpp
	$(call compile_src_file, $<, $@)

%.o:%.cc
	$(call compile_src_file, $<, $@)

%.o:%.c
	$(call compile_src_file, $<, $@)

#源文件依赖文件生成规则
%.d: %.cpp
	@$(call create_src_dep_file, $<, $@)
	
%.d: %.cc
	@$(call create_src_dep_file, $<, $@)

%.d: %.c
	@$(call create_src_dep_file, $<, $@)

#如果有指定编译中间目录，生成相关编译命令文件规则
ifneq ($(strip $(BUILD_DIR)),)
$(BUILDDIR_RULES_FILE) : $(ALL_SOURCE_FILES)
	@$(call create_builddir_rules_file)
endif

#所有xml目标文件生成规则
$(XML_DEP_FILE) : $(XML2H_FILES) $(XML2C_FILES) $(XML2CPP_FILES) $(XML2TDR_FILES)
	@touch $(XML_DEP_FILE)

#所有jce目标文件生成规则
$(JCE_DEP_FILE) : $(JCE2CPP_FILES)
	@touch $(JCE_DEP_FILE)

#协议目录创建规则
$(PROTO_GEN_DIR_CREATE_DEP)	:
	@mkdir -p $(XML_AND_JCE_GEN_DIRS)

#makefile依赖规则
$(MAKEFILE_DEP_FILE) :
	$(call make_subdir, $(DEP_MAKE_SUBDIRS), $(MAKECMDGOALS))
	$(call make_submakefile, $(DEP_MAKE_MAKEFILES), $(MAKECMDGOALS))

#xml目标文件生成规则
$(XML2H_DIR)/%.h : %.xml
	@$(TDR) -H -O "$(XML2H_DIR)" $(call find_dep_xmls, $<)

$(XML2C_DIR)/%.c : %.xml
	@$(TDR) -C $(call find_dep_xmls, $<) -o $@

$(XML2CPP_DIR)/%.h : %.xml
	@$(TDR) -P $(call find_dep_xmls, $<) -O "$(XML2CPP_DIR)"

$(XML2TDR_DIR)/%.tdr : %.xml
	@$(TDR) -B $(call find_dep_xmls, $<) -o $@

$(JCE2CPP_DIR)/%.h : %.jce
	@echo "jce2cpp $< ..."; \
	cd `dirname $<`;\
	chmod +x $(JCE2CPP_TOOL); \
	$(JCE2CPP_TOOL) $(JCE2CPP_FLAGS) --dir=$(ABS_JCE2CPP_DIR) `basename $<`

#xml目标文件依赖生成规则
$(PROTO_DEP_DIR)/%.h.xml2h.d : %.xml
	@$(call create_xmldestfile_dep_file, $(call find_dep_xmls, $<), $@, $(XML2H_DIR), "h")

$(PROTO_DEP_DIR)/%.c.xml2c.d : %.xml
	@$(call create_xmldestfile_dep_file, $(call find_dep_xmls, $<), $@, $(XML2C_DIR), "c")

$(PROTO_DEP_DIR)/%.h.xml2cpp.d : %.xml
	@$(call create_xmldestfile_dep_file, $(call find_dep_xmls, $<), $@, $(XML2CPP_DIR), "h")

$(PROTO_DEP_DIR)/%.tdr.xml2tdr.d : %.xml
	@$(call create_xmldestfile_dep_file, $(call find_dep_xmls, $<), $@, $(XML2TDR_DIR), "tdr")
	
#以下进行错误提示
$(XML2H_DIR)/%.h :
	$(warning ERROR: file "$(shell srcfile=`basename $@`;echo $${srcfile%.*}.xml)" not exist)

$(XML2C_DIR)/%.c :
	$(warning ERROR: file "$(shell srcfile=`basename $@`;echo $${srcfile%.*}.xml)" not exist)

$(XML2CPP_DIR)/%.h :
	$(warning ERROR: file "$(shell srcfile=`basename $@`;echo $${srcfile%.*}.xml)" not exist)

$(XML2TDR_DIR)/%.tdr :
	$(warning ERROR: file "$(shell srcfile=`basename $@`;echo $${srcfile%.*}.xml)" not exist)

$(JCE2CPP_DIR)/%.h :
	$(warning ERROR: file "$(shell srcfile=`basename $@`;echo $${srcfile%.*}.jce)" not exist)

$(PROTO_DEP_DIR)/%.h.xml2h.d :
	$(warning ERROR: file "$(shell srcfile=`basename $@`;echo $${srcfile%%.*}.xml)" not exist)

$(PROTO_DEP_DIR)/%.c.xml2c.d :
	$(warning ERROR: file "$(shell srcfile=`basename $@`;echo $${srcfile%%.*}.xml)" not exist)

$(PROTO_DEP_DIR)/%.h.xml2cpp.d :
	$(warning ERROR: file "$(shell srcfile=`basename $@`;echo $${srcfile%%.*}.xml)" not exist)

$(PROTO_DEP_DIR)/%.tdr.xml2tdr.d :
	$(warning ERROR: file "$(shell srcfile=`basename $@`;echo $${srcfile%%.*}.xml)" not exist)

%.cpp :
	$(error ERROR: file "$@" not exist)

%.c :
	$(error ERROR: file "$@" not exist)

%.cc :
	$(error ERROR: file "$@" not exist)
