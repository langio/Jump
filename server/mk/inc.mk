# Generic Makefile framework
builder.version = 1.21

# inclusion guard
ifndef GENERIC_MAK_INCLUDED
GENERIC_MAK_INCLUDED := 1
else
GENERIC_MAK_INCLUDED_MESSAGE := "Generic.mak alread included, please check your \'$(firstword $(MAKEFILE_LIST))\'"
$(call color_error, $(GENERIC_MAK_INCLUDED_MESSAGE))
endif

###############################################################################
# builder options:
# - builder.debug
# - builder.verbose
# - builder.depend_makefile
# - builder.use_ccache
#
# project options:
# - project.targets
# - project.subdirs
# - project.prebuild
# - project.postbuild
# - project.extra_clean
# - project.extra_distclean
# - project.pipe
# - project.debug
# - project.optimize_flags
# - project.profile
# - project.coverage_test
# - project.includes
# - project.c_includes
# - project.cxx_includes
# - project.defines
# - project.c_defines
# - project.cxx_defines
# - project.warning
# - project.c_warning
# - project.cxx_warning
# - project.extra_warning
# - project.extra_c_warning
# - project.extra_cxx_warning

# define comma
comma :=,

# unify bool option value
# conversion rules:
# 	undefined		-> default value
# 	yes,y,1,true	-> yes
# 	null			-> 
# 	no,n,0,false	-> 
# params:
# (1) variable name
# (2) default value
get_bool_option_value = $(strip \
	$(if $(findstring undefined,$(origin $(1))), $(2), \
		$(if $(findstring y,$($(1)))$(findstring 1,$($(1)))$(findstring true,$($(1))),yes, \
			$(if $(findstring n,$($(1)))$(findstring 0,$($(1)))$(findstring false,$($(1))),,\
				$(if $($(1)),$(error Invalid bool value '$($1)' of $(1))) \
			) \
		) \
	) \
)

# (1) variable name
# (2) default value
define unify_bool_value
override $(1) := $(call get_bool_option_value,$(1),$(2))
endef

dump_variable = $(info $1='$($1)')

# // -> /
remove_duplicated_slash = $(subst //,/,$(subst //,/,$1))

###############################################################################
# detect shell type, there are some differences between bash and POSIX sh
shell_is_bash := $(shell echo $$BASH_VERSION)

# bash need -e to interpret escapes
echo-e := echo$(if $(shell_is_bash), -e)

# mkdir if not exist, 100 times faster than direct mkdir -p
mkdir_if_not_exist = [ -d $(1) ] || mkdir -p $(1)

# detect whether stderr is directed
# note: stdout can't be detected in this way, it's always redirected
stderr_is_redirected := $(shell [ -t 2 ] || echo yes)

###############################################################################
# color control
###############################################################################

# define color_enabled function
# $(1) target fd
ifndef COLOR
	color_enabled = [ -t $(1) ]
else
ifeq ($(COLOR),yes)
	color_enabled = true
else
ifeq ($(COLOR),no)
	color_enabled = false
else
$(error Invalid bool value '$(COLOR)' for COLOR)
endif
endif
endif

# echo success message
# param
# $(1) message
# $(2) emphasize
define echo_success
	if $(call color_enabled,1); then\
		$(echo-e) "\\033[1;$(2)32m$(1)\\033[m";\
	else\
		$(echo-e) "$(1)";\
	fi
endef

ifneq ($(shell_is_bash),)
shell_regex_match = [[ $(1) =~ $(2) ]]
else
shell_regex_match = echo $(1) | grep $(2) >/dev/null 2>&1
endif

warning_message = $(shell iconv -f UTF-8 -t GBK <<< ": 警告：")
error_message = $(shell iconv -f UTF-8 -t GBK <<< "错误：")

# colorfy error message
# params
# $(1) message header
# $(2) message body
#      red for error
#      yellow for warning
#      cyan for other information
define echo_diagnostic_message_colorful
	LFS=;\
	colored_result=$$($(echo-e) "$(2)" | \
		while read line; do\
			if \
				$(call shell_regex_match,"$$line",' error: ') ||\
				$(call shell_regex_match,"$$line",'错误：') ||\
				$(call shell_regex_match,"$$line","$(error_message)") ||\
				$(call shell_regex_match,"$$line",': undefined reference to'); then\
				$(echo-e) "\033[1;31m$${line}\033[m";\
			elif \
				$(call shell_regex_match,"$$line",': warning: ') ||\
				$(call shell_regex_match,"$$line",': 警告：') ||\
				$(call shell_regex_match,"$$line","$(warning_message)") ||\
				$(call shell_regex_match,"$$line",': note: '); then\
				$(echo-e) "\033[1;33m$${line}\033[m";\
			else\
				$(echo-e) "\033[1;36m$${line}\033[m";\
			fi\
		done);\
	$(echo-e) "$(1)\n$$colored_result" >&2;
endef

stderr_color := $(call get_bool_option_value,COLOR,$(if $(stderr_is_redirected),,yes))

ifeq ($(stderr_color),yes)
echo_error_message = $(call echo_diagnostic_message_colorful,\033[1;31m$(1)\033[m,$(2))
echo_warning_message = $(call echo_diagnostic_message_colorful,\033[1;33m$(1)\033[m,$(2))
echo_waring = $(echo-e) "\\033[1;33m$(1)\\033[m" >&2;
echo_error = $(echo-e) "\\033[1;31m$(1)\\033[m" >&2;
color_error = $(error $(shell $(echo-e) "\033[1;31m$1\033[m"))
color_warning = $(warning $(shell $(echo-e) "\033[1;32m$1\033[m"))
else
echo_error_message = $(echo-e) "$(1)\n$(2)" >&2;
echo_warning_message = $(echo_error_message)
echo_error = $(echo-e) "$(1)" >&2;
echo_waring = $(echo-e) "$(1)" >&2;
color_error = $(error $(1))
color_warning = $(warning $(1))
endif

# define emphasize, you can change it to change target command emphasize style
#
# ANSI control sequence to emphasize target output, may be the following values
# and can also be combined, separated by ';', eg. 1;4;7
# 0 -- normal
# 1 -- highlight
# 4 -- underline
# 5 -- blink
# 7 -- background revert
# 8 -- hidden
emphasize := 7

# run command and echo output in color according result
# params:
# $(1) prefix
# $(1) command
# $(2) message to echo when full success
# $(3) emphasize
define color_run
	cmd="$(strip $(2))"; \
	o=`$$cmd 2>&1`; \
	ret=$$?; \
	if [ $$ret -eq 0 ]; then \
		if [ -z "$$o" ]; then \
			$(call echo_success,$(1)$(if $(builder.verbose),$$cmd,$(3)),$(4))\
		else \
			$(call echo_warning_message,$(1)$$cmd,$$o) \
		fi \
	else \
		$(call echo_error_message,$(1)$$cmd,$$o) \
	fi; \
	exit $$ret;
endef

define color_run_link
	cmd="$(strip $(2)$(comma)$(3)$(comma)$(4))"; \
	o=`$$cmd 2>&1`; \
	ret=$$?; \
	if [ $$ret -eq 0 ]; then \
		if [ -z "$$o" ]; then \
			$(call echo_success,$(1)$(if $(builder.verbose),$$cmd,$(5)),$(6))\
		else \
			$(call echo_warning_message,$(1)$$cmd,$$o) \
		fi \
	else \
		$(call echo_error_message,$(1)$$cmd,$$o) \
	fi; \
	exit $$ret;
endef

###############################################################################
# detect running environment
###############################################################################

###############################################################################
# process command line variables
define apply_command_line_option
ifneq ("$(origin $(1))", "undefined")
    ifeq ("$(origin $(1))", "command line")
        $(2) := $($(1))
    else
        message:=$(shell $(echo-e) "\\033[1;33mthe '$(1)' option is only accepted from command line, ignored\\033[m")
        $$(warning $$(message))
    endif
endif
endef

$(eval $(call apply_command_line_option,V,builder.verbose))
$(eval $(call apply_command_line_option,DEBUG,project.debug))
$(eval $(call apply_command_line_option,PROFILE,project.profile))
$(eval $(call apply_command_line_option,COVERAGE,project.coverage_test))
$(eval $(call apply_command_line_option,ARCH,project.target_arch))

command_line_variables += V
V.help := "bool, enable verbose build command line"

command_line_variables += DEBUG
DEBUG.help := "bool, enable debug mode"

command_line_variables += PROFILE
PROFILE.help := "bool, enable prifiling"

command_line_variables += COVERAGE
COVERAGE.help := "bool, enable coverage test"

command_line_variables += ARCH
ARCH.help := "specify target architecture explicitly, support i386 or x86_64"

ifdef project.target_arch
ifeq ($(project.target_arch),i386)
	project.target_bits := 32
else
ifeq ($(project.target_arch),x86_64)
	project.target_bits := 64
else
$(error invalid architecture type, only support i386 and x86_64)
endif
endif
endif

$(eval $(call unify_bool_value,project.debug))
$(eval $(call unify_bool_value,project.verbose))

# try to find Config.mak
# search Config.mak upward recursively and include them
#ifneq ($(strip $(project.use_config)),)
#	project.config_files := $(shell \
#		dir=`pwd`; \
#		while [ "$$dir" != "/" ]; do \
#			if [ -f "$$dir/Config.mak" ]; then \
#				echo "$$dir/Config.mak "; \
#			fi; \
#			dir=`dirname "$$dir"`; \
#		done)
#endif
 
#ifneq ($(SOSO_BUILD),)
#	project.use_unique_output_dir := yes
#endif

this_dir := $(dir $(shell echo $(MAKEFILE_LIST) | awk '{ print $$NF }'))
#project.src_dir := $(shell cd $(this_dir) && pwd -P)/
project.root_dir := $(shell cd $(this_dir) && pwd -P )/

#ifeq ($(project.predefined_includes),)
#project.predefined_includes := $(wildcard $(project.src_dir) $(project.src_dir)/thirdparty)
#endif

# relative path from source root or project root, prefer the shorter
# try remove src_root, if failed remove root part
#this_dir_part := $(subst $(project.root_dir),,$(subst $(project.src_dir)/,,$(CURDIR)/))
this_dir_part := $(subst $(project.root_dir),,$(CURDIR)/)

os_name := $(shell uname -s | tr 'A-Z' 'a-z')

project.obj_dir ?= $(project.root_dir)output/objs
project.test_result_dir := $(project.root_dir)output/test_result

#project.use_unique_output_dir := yes
ifneq ($(project.use_unique_output_dir),)
#project.bin_dir := $(project.root_dir)output/bin/$(os_name)
#project.lib_dir := $(project.root_dir)output/lib/$(os_name)
project.bin_dir := $(project.root_dir)output/bin
project.lib_dir := $(project.root_dir)output/lib
endif

ifneq ($(builder.debug),)
$(call dump_variable,project.root_dir)
$(call dump_variable,project.obj_dir)
$(call dump_variable,project.bin_dir)
$(call dump_variable,project.lib_dir)
$(call dump_variable,this_dir_part)
endif

###############################################################################
# Common  options
###############################################################################

# whether using ccache if available, default is true
builder.use_ccache ?= 1

# detect whether ccache is available
builder.ccache_detected = $(shell ccache >/dev/null 2>&1; error=$$?; if [ $$error != 127 ]; then echo y; fi)

ifneq ($(builder.use_ccache),)
ifneq ($(builder.ccache_detected),)
CPP_PREFIX := ccache
CC_PREFIX  := ccache
CXX_PREFIX := ccache
endif
endif

# even new installed gcc existed, cc may also be old cc, override it
CPP ?= cpp
CC ?= gcc
CXX ?= g++

builder.gxx_version := $(shell g++ --version | head -n1 | cut -d' ' -f3)

PREFIXED_CPP := $(if $(CPP_PREFIX),$(CPP_PREFIX) $(CPP),$(CPP))
PREFIXED_CC  := $(if $(CC_PREFIX),$(CC_PREFIX) $(CC),$(CC))
PREFIXED_CXX := $(if $(CXX_PREFIX),$(CXX_PREFIX) $(CXX),$(CXX))

# helps
builder.verbose.help = "show complete command line and more details"
builder.depend_makefile.help = ""
options += builder.verbose builder.depend_makefile

project.targets.help = "all targets to build of this project"
project.libpaths.help = ""

options += project.targets project.libpaths \
           project.includes project.c_includes project.cxx_includes \
           project.warning project.c_warning project.cxx_warning \
           project.extra_warning project.extra_c_warning project.extra_cxx_warning \

###############################################################################
# Default option value and flags

# pipe
project.pipe ?= 1
project.pipe.help = "Whether using pipe instead of temporary file in compiling"

project.pipe_flags ?= -pipe
project.pipe_flags.help = "using pipe instead tempory file in compiling, current value is $(project.pipe_flags)"

options += project.pipe project.pipe_flags


# project.profile = ?
project.profile.help = "compiling and linking for profile"

project.profile_flags ?= -pg
project.profile_flags.help = "compiling and linking flags for profile, current value is $(project.profile_flags)"

options += project.profile project.profile_flags

# coverage test
project.coverage_test_flags ?= -pg -fprofile-arcs -ftest-coverage
#project.optimize_flags ?= -O2

# default warning flags for both C and C++
project.warning ?= \
    -Wall \
    -Wno-invalid-offsetof
#	-Wall \
#	-Wextra \
#	-Wformat=2 \
#	-Wno-unused-parameter \
#	-Wno-missing-field-initializers \
#	-Wmissing-include-dirs \
#	-Wfloat-equal \
#	-Wpointer-arith \
#	-Wwrite-strings

#	-Wconversion
#	-Wswitch-default

# warning flags for C only
# project.c_warning =

# warning flags for C++ only
#project.cxx_warning ?= \
#	-Woverloaded-virtual \
#	-Wnon-virtual-dtor \
#	-Wno-invalid-offsetof

ifneq ($(builder.gxx_version), "4.1.2")
	-Werror=return-type
endif

# using extra_ prefixed options if you don't want to override default WARNING
# project.extra_warning ?=
# project.extra_c_warning ?=
# project.extra_cxx_warning ?= -Weffc++

# target bits, 32 or 64 or null for default
# project.target_bits ?= 

machine_type = $(shell uname -m)

ifeq ($(machine_type),x86_64)
	project.default_target_bits := 64
else
	project.default_target_bits := 32
endif

# Handle all *FLAGSs
# CC, CXX, LD common flags
OPTION_FLAGS := \
	$(if $(project.pipe),$(project.pipe_flags)) \
	$(if $(project.profile),$(project.profile_flags)) \
	$(if $(project.coverage_test),$(project.coverage_test_flags))

# CPPFLAGS from options
OPTION_CPPFLAGS := \
	$(OPTION_FLAGS) \
	$(project.warning) \
	$(project.extra_warning)

OPTION_LDFLAGS := $(OPTION_FLAGS)

COMMON_LDFLAGS := $(strip $(OPTION_LDFLAGS) $(LDFLAGS))

# default ARFLAGS is not silent, override it
ARFLAGS = crus

###############################################################################
# Functions
###############################################################################

# $(1) target
is_debug = $(project.debug)$($(1).debug)

# debug/release mode
get_target_build_type_flags = \
	$(if $(call is_debug,$(1)),\
		-ggdb3 -fstack-protector,\
		-g $(if $($(1).optimize_flags),$($(1).optimize_flags),$(project.optimize_flags))\
	)

get_target_build_type_defines = $(if $(call is_debug,$(1)),,-D NDEBUG)

# function to normalize gcc option prefix, such as -I -L -D 
normalize_prefix = $(addprefix $(1),$(strip $(patsubst $(1)%,%,$(2))))

# $(1) target name
# return -m32, -m64, or empty.
get_target_bits_flag = \
	$(strip\
		$(if $($(1).target_bits),\
			-m$($(1).target_bits),\
			$(if $(project.target_bits),-m$(project.target_bits))\
		)\
	)

# $(1) target name
# return 32 or 64.
get_target_bits = \
	$(strip\
		$(if $($(1).target_bits),\
			$($(1).target_bits),\
			$(if $(project.target_bits),$(project.target_bits),$(project.default_target_bits))\
		)\
	)

target_bit_is_32 = $(findstring 32,$(call get_target_bits,$(1)))
target_bit_is_64 = $(findstring 64,$(call get_target_bits,$(1)))

# generate i386/debug, x86_64/release, etc
get_build_type_mode_dir_part = $(if $(call target_bit_is_32,$1),i386,x86_64)/$(if $(call is_debug,$(1)),debug,release)

# force add -fPIC for x86_64 or so target
# $(1) target name
get_target_pic_flag = \
	$(if $(call get_target_bits_flag,$(1)),\
		$(if $(findstring 64,$(call get_target_bits_flag,$(1))),-fPIC),\
		$(if $(findstring 64,$(project.default_target_bits)),-fPIC)\
	)

get_common_compile_flags = \
	$(call get_target_bits_flag,$(1)) \
	$(call get_target_pic_flag,$(1)) \
	$(call get_target_build_type_flags,$(1)) \

# same as compile flags currently, may be difference
get_common_link_flags = $(get_common_compile_flags)

#set CXXFLAGS
CXXFLAGS := -std=c++0x

# Get CXXFLAGS of one source file
# $(1) target
# $(2) source file fullname
get_flags.cpp = \
	$(strip \
		$(OPTION_CPPFLAGS) $(project.cxx_warning) $(project.extra_cxx_warning) \
		$(call get_common_compile_flags,$(1)) \
		$(CPPFLAGS) $(CXXFLAGS) \
		$(call normalize_prefix,-D,$(project.auto_defines) $(project.defines) $(project.cxx_defines)) \
		$(call normalize_prefix,-D,$($(1).defines) $($(1).cxx_defines)) \
		$(call get_target_build_type_defines,$(1)) \
		$(call normalize_prefix,-I,$(project.predefined_includes)) \
		$(call normalize_prefix,-I,$(project.includes) $(project.cxx_includes)) \
		$(call normalize_prefix,-I,$($(1).includes) $($(1).cxx_includes)) \
		$(project.extra_cppflags) $(project.extra_cxxflags) \
		$($(2).extra_flags) \
	)

# alias of cpp
get_flags.cxx = $(get_flags.cpp)
get_flags.cc = $(get_flags.cpp)

# Get CFLAGS of one source file
# $(1) target
# $(2) source file fullname
get_flags.c = \
	$(strip \
		$(OPTION_CPPFLAGS) \
		$(call get_common_compile_flags,$(1)) \
		$(project.c_warning) $(project.extra_c_warning) \
		$(CPPFLAGS) $(CFLAGS) \
		$(call normalize_prefix,-D,$(project.auto_defines) $(project.defines) $(project.c_defines)) \
		$(call normalize_prefix,-D,$($(1).defines) $($(1).c_defines)) \
		$(call get_target_build_type_defines,$(1)) \
		$(call normalize_prefix,-I,$(project.predefined_includes)) \
		$(call normalize_prefix,-I,$(project.includes) $(project.c_includes)) \
		$(call normalize_prefix,-I,$($(1).includes) $($(1).c_includes)) \
		$(project.extra_cppflags) $(project.extra_cflags) \
		$($(2).extra_flags) \
	)

# $(1) target name
# $(2) source file fullname
get_flags_of = \
	$(if $(get_flags$(suffix $(2))),\
		$(get_flags$(suffix $(2))),\
		$(call color_error,Unknown suffix of source '$(2)' for target '$(1)'))

# Get include dirs for one source file
# $(1) target
# $(2) source file fullname
get_includes.cpp = $(call normalize_prefix,-I,$(project.includes) $(project.cxx_includes) $($(1).includes) $($(1).cxx_includes))
get_includes.cxx = $(get_includes.cpp)
get_includes.cc = $(get_includes.cpp)
get_includes.c = $(call normalize_prefix,-I,$(project.includes) $(project.c_includes) $($(1).includes) $($(1).c_includes))
get_includes_of = \
	$(if $(get_includes$(suffix $(2))),\
		$(get_includes$(suffix $(2))),\
		$(call color_error,Unknown suffix of source '$(2)' for target '$(1)'))
# param $(1) target name
# param $(2) strings
# replace:
#   @libroot@ -> library root include system/arch/buildtype
#   @libroot_debug@ -> library root include system/arch/debug
#   @libroot_release@ -> library root include system/arch/release
#   @target_bits@ -> 32 or 64
#   @64@ -> 64 or empty
#   @32@ -> 32 or empty
ldadd_macro_subst = $(strip \
	$(subst @libroot_debug@,$(if $(call target_bit_is_32,$1),i386,x86_64)/debug,\
		$(subst @libroot_release@,$(if $(call target_bit_is_32,$1),i386,x86_64)/release,\
			$(subst @libroot@,$(project.lib_dir)/$(call get_build_type_mode_dir_part,$1),\
				$(subst @32@,$(call target_bit_is_32),\
					$(subst @64@,$(call target_bit_is_64),\
						$(subst @target_bits@,$(call get_target_bits,$(1)),$(2))))))))

# Get LDFLAGS from of one target
get_ld_flags = \
	$(if $($(1).flags),$($(1).flags),$(COMMON_LDFLAGS))\
	$(call get_common_link_flags,$(1))\
	$($(1).extra_flags) $(addprefix -L,$(project.libpaths))

get_target_user_ldadd = $(call ldadd_macro_subst,$(1),$(project.ldadd) $($(1).ldadd))
get_target_user_force_ldadd = $(call ldadd_macro_subst,$(1),$(project.force_ldadd) $($(1).force_ldadd))

get_target_ldadd = $(get_target_user_ldadd)
get_target_force_ldadd = $(get_target_user_force_ldadd)
# $(if $(call is_debug,$(1)),-lmudflap)

get_target_libs = $(filter -l% %.a %.so, $(call get_target_user_ldadd,$(1)) $(call get_target_user_force_ldadd,$(1)))

# Get ARFLAGS from of one library target
get_ar_flags = $(if $($(1).flags),$($(1).flags),$(ARFLAGS))

# get object dir for target, eg. server -> .objects/server
# param
# $(1) target
get_target_objdir = $(project.obj_dir)/$(call get_build_type_mode_dir_part,$1)/$(this_dir_part)$(1)

# get object directory of source file
# param
# $(1) target
# $(2) source file fullname
# subst special path:
#   ../ -> __/
#   ./ -> _/
#   // -> /
get_source_objdir = $(call get_target_objdir,$(1))/$(subst //,/,$(subst ./,,$(subst ../,__/,$(dir $(2)))))

# $(1) target
# $(2) source full path
get_object_path_prefix = $(call get_source_objdir,$(1),$(2))$(notdir $(2))

get_object = $(call get_object_path_prefix,$(1),$(2)).o
get_depend = $(call get_object_path_prefix,$(1),$(2)).d
get_flagfile = $(call get_object_path_prefix,$(1),$(2)).flags

# $(1) target name
get_target_objects = $(foreach source,$($(1).sources),$(call get_object,$(1),$(source)))
get_target_depends = $(foreach source,$($(1).sources),$(call get_depend,$(1),$(source)))

# for all sources of target
get_target_flagfiles = $(foreach source,$($(1).sources),$(call get_flagfile,$(1),$(source)))

# for target self
get_target_flagfile = $(call get_target_objdir,$(1)).flags

# get_linker from target sources types
# return $(CC) for C, $(CXX) for C++ or mixed C/C++
get_linker = \
	$(if $(filter %.cpp %.cxx %.cc,$($(notdir $(1)).sources)),$(CXX),$(CC))


###############################################################################
# Rules
###############################################################################

all:

###############################################################################
# process files

$(if $(project.targets) $(project.subdirs),,\
	$(call color_error,Neither project.targets nor project.subdirs found in\
		$(firstword $(MAKEFILE_LIST))))

# check whether each target has sources
$(foreach target,$(project.targets),\
	$(if $($(target).sources),,\
		$(call color_error,Missing 'sources' for target '$(target)')))

# check source file existance
#$(foreach target,$(project.targets),\
#	$(foreach source,$($(target).sources),\
#		$(if $(shell test -f $(source) || echo nofile),\
#			$(if $(findstring $(source),$(project.generated_files)),,\
#				$(call color_error,Source file '$(source)' of '$(target)' does not exist)))))

all_objects := $(foreach target,$(project.targets),$(call get_target_objects,$(target)))
all_depends := $(foreach target,$(project.targets),$(call get_target_depends,$(target)))

# Whenever makefiles changed, rebuild
ifneq ($(strip $(builder.depend_makefile)),)
$(all_depends): $(MAKEFILE_LIST)
$(all_objects): $(MAKEFILE_LIST)
endif

all_flagfiles := $(foreach target,$(project.targets),$(call get_target_flagfiles,$(target))$(call get_target_flagfile,$(target)))

# define command to update flags file
# $(1) file name
# $(2) flags
define update_flags_file
	@$(call mkdir_if_not_exist,$$(dir $(1))); \
	new_flags="$(2)"; \
	if [ -f $(1) ]; then \
		old_flags="`cat $$@ 2>/dev/null`"; \
		if [ x"$$$$new_flags" != x"$$$$old_flags" ] ; then \
			echo "$$$$new_flags" > $(1); \
			$(if $(builder.debug),echo "$(1) updated";) \
		else \
		$(if $(builder.debug),echo "$(1) nochange";,:;) \
		fi \
	else \
		echo "$$$$new_flags" > $(1); \
		$(if $(builder.debug),echo "$(1) created";) \
	fi
endef

# define rule to generate compile flags file
# $(1) target
# $(2) source full filename
define define_flagfile_rule
$(call get_flagfile,$(1),$(2)): FORCE
	$(call update_flags_file,$$@,$(strip $(call get_flags_of,$(1),$(2))))
endef

# expand flagfile rules
$(foreach target,$(project.targets),\
	$(foreach source,$($(target).sources),\
		$(eval $(call define_flagfile_rule,$(target),$(source)))\
	)\
)

# Rules to generate depends
# $(1) target
# $(2) source full filename
define define_depend_rule.cpp
$(call get_depend,$(1),$(2)): $(call get_flagfile,$(1),$(2))
	@$(if $(builder.debug),[ -f $$@ ] && echo "Updating dependancy for $(2) (imacted by $$?)" || \
        echo "Generating dependancy for $(2)";) \
	o=`$(PREFIXED_CPP) -c -MP -MM -MT $(call get_object,$(1),$(2)) $(call get_flags_of,$(1),$(2)) $(2) 2>&1`; \
	if [ $$$$? -eq 0 ]; then \
		echo "$$$$o" | sed 's|\($(call get_object,$(1),$(2))\):|\1 $$@:|' > $$@; \
	else \
	$(if $(builder.debug),$(echo-e) "\033[1;33mWarning: Can't generate dependancy file $$@ for $(2):\n$$$$o\033[m";,:;) \
	fi
endef

# alias of cpp
define_depend_rule.cxx = $(define_depend_rule.cpp)
define_depend_rule.cc = $(define_depend_rule.cpp)
define_depend_rule.c = $(define_depend_rule.cpp)

# expand depend rules
$(foreach target,$(project.targets),\
	$(foreach source,$($(target).sources),\
		$(if $(call define_depend_rule$(suffix $(source)),$(target),$(source)),\
		 	$(eval $(call define_depend_rule$(suffix $(source)),$(target),$(source))),\
			$(call color_error,Unknown suffix '$(suffix $(source))' of $(source) to make dependancy)\
		)\
	)\
)

# Rules to compile

# $(1) target
# $(2) source file
define define_compile_rule.cpp
$(call get_object,$(1),$(2)) : $(2) $(call get_flagfile,$(1),$(2)) $(call get_depend,$(1),$(2))
	@$(call mkdir_if_not_exist,$(call get_source_objdir,$(1),$(2))); \
		$$(call color_run,[$1] ,$(PREFIXED_CXX) -c $(call get_flags_of,$(1),$(2)) -o $$@ $$<, $(PREFIXED_CXX) compile: $$<)
endef

# alias of cpp
define_compile_rule.cc = $(define_compile_rule.cpp)
define_compile_rule.cxx = $(define_compile_rule.cpp)

# $(1) target
# $(2) source file
define define_compile_rule.c
$(call get_object,$(1),$(2)) : $(2) $(call get_flagfile,$(1),$(2)) $(call get_depend,$(1),$(2))
	@$(call mkdir_if_not_exist,$(call get_source_objdir,$(1),$(2))); \
		$$(call color_run,[$1] ,$(PREFIXED_CC) -c $(call get_flags_of,$(1),$(2)) -o $$@ $$<, $(PREFIXED_CC) compile: $$<)
endef

# expand source rules
$(foreach target,$(project.targets),\
	$(foreach source,$($(target).sources),\
		$(if $(call define_compile_rule$(suffix $(source)),$(target),$(source)),\
		 	$(eval $(call define_compile_rule$(suffix $(source)),$(target),$(source))),\
			$(call color_error, Unknown suffix '$(suffix $(source))' of $(source) to compile)\
		)\
	)\
)

###############################################################################
# define target rules

# get target dir
# $(1) target name
ifneq ($(project.use_unique_output_dir),)
# program
get_target_default_dir_of_ = $(project.bin_dir)/$(call get_build_type_mode_dir_part,$1)/$(this_dir_part)
get_target_default_dir_of_exe = $(get_target_default_dir_of_)
get_target_default_dir_of_program = $(get_target_default_dir_of_)

get_target_default_dir_of_library = $(project.lib_dir)/$(call get_build_type_mode_dir_part,$1)/$(this_dir_part)
get_target_default_dir_of_lib = $(get_target_default_dir_of_library)
get_target_default_dir_of_a = $(get_target_default_dir_of_library)
get_target_default_dir_of_dynamic = $(get_target_default_dir_of_library)
get_target_default_dir_of_shared = $(get_target_default_dir_of_library)
get_target_default_dir_of_dll = $(get_target_default_dir_of_library)
get_target_default_dir_of_so = $(get_target_default_dir_of_library)

get_target_default_dir = $(get_target_default_dir_of_$($(1).type))

get_target_dir = $(call remove_duplicated_slash,$(strip $(if $($(1).path),$(strip $($(1).path))/,$(call get_target_default_dir,$1))))
else
get_target_dir = $(call remove_duplicated_slash,$(strip $(if $($(1).path),$(strip $($(1).path))/)))
endif

convert_so_path = $(shell echo $1 | sed 's|\([^ -]*/\)lib\([A-Za-z0-9._+-]*\)\.so|-L\1 -l\2|g')

# get path and name of target
# $(1) target name
#
# rule:
# 	target.path
# 	project.path
# 	project.root_path
get_target_full_path = $(call get_target_dir,$(1))$(if $($(1).name),$($(1).name),$(1))

whole_archive := -Wl,-whole-archive
no_whole_archive := -Wl,-no-whole-archive
# define link rule for program
define define_target_program
$(call get_target_full_path,$(1)): $($(1).meta) $(call get_target_objects,$(1)) $(call get_target_libs,$(1)) $(call get_target_flagfile,$(1))
	@$(call mkdir_if_not_exist,$$(@D)) && \
	$$(call color_run_link,[$(1)] ,\
		$(call get_linker,$(1)) -o $$@ $(call get_ld_flags,$(1)) $(call get_target_objects,$(1)) \
			$(whole_archive) $(call convert_so_path,$(call get_target_force_ldadd,$(1))) $(no_whole_archive) -Xlinker -( $(call convert_so_path,$(call get_target_ldadd,$(1))) -Xlinker -),\
			Success in linking program $$@,$(emphasize))
$(call get_target_flagfile,$(1)): FORCE
	$(call update_flags_file,$$@,$(strip $(call get_ld_flags,$(1)) $(call get_target_objects,$(1)) $(call get_target_ldadd,$(1)) $(call get_target_force_ldadd,$(1))))
endef

# aliases of program
define_target_ = $(define_target_program)
define_target_exe = $(define_target_program)

# define link rule for shared library
define define_target_shared
$(call get_target_full_path,$(1)): $(call get_target_objects,$(1)) $(call get_target_libs,$(1)) $(call get_target_flagfile,$(1))
	@$(call mkdir_if_not_exist,$$(@D)) && \
	$$(call color_run_link,[$(1)] ,\
		$(call get_linker,$(1)) -o $$@ -shared $(if $($(1).allow_undefined),,-Xlinker --no-undefined) \
			$(call get_ld_flags,$(1)) $(call get_target_objects,$(1)) \
			$(whole_archive) $(call convert_so_path,$(call get_target_force_ldadd,$(1))) $(no_whole_archive) \
			-Xlinker -( $(call convert_so_path,$(call get_target_ldadd,$(1))) -Xlinker -),\
			Success in linking shared library $$@,$(emphasize))
$(call get_target_flagfile,$(1)): FORCE
	$(call update_flags_file,$$@,$(strip $(call get_target_objects,$(1)) $(call get_ld_flags,$(1)) $(if $($(1).allow_undefined),,-Xlinker --no-undefined) $(call get_target_ldadd,$(1)) $(call get_target_force_ldadd,$(1))))
endef

# aliases of shared
define_target_dynamic = $(define_target_shared)
define_target_so = $(define_target_shared)
define_target_dll = $(define_target_shared)

# define archive rule for static library
define define_target_library
$(call get_target_full_path,$(1)): $(call get_target_objects,$(1))
	@$(RM) $$@; \
	$$(call color_run,[$(1)] ,\
		$(AR) $(call get_ar_flags,$(1)) $$@ $$^,Success in updating static library $$@,$(emphasize))
endef

# aliases of library
define_target_static = $(define_target_library)
define_target_a = $(define_target_library)
define_target_lib = $(define_target_library)

# if different target.name and/or target.path are specified:
define define_target_mkdir
ifneq ($(call get_target_full_path,$(1)),$(1))
PHONY += $(1) $(1).mktargetdir $(1)
$(1): $(1).mktargetdir $(call get_target_full_path,$(1))
$(1).mktargetdir:
	@$(call mkdir_if_not_exist,$(call get_target_dir,$(1)));
endif
endef

# expand targets
$(foreach target,$(project.targets),\
	$(if $(call define_target_$($(target).type),$(target)),\
		$(eval $(call define_target_$($(target).type),$(target)))\
		$(eval $(call define_target_mkdir,$(target))),\
		$(call color_error,Unknown target type '$($(target).type)' of '$(target)')))

# add all libpath to library search path
ifneq ($(project.libpaths),)
vpath lib%.a $(project.libpaths)
vpath lib%.so $(project.libpaths)
endif

# define libname phony to avoid missing libary error
define define_libname_phony
$(1):
endef

all_libs := $(strip $(sort $(filter -l%,$(foreach target,$(project.targets),$($(target).ldadd)))))
all_libs += $(strip $(sort $(filter -l%,$(foreach target,$(project.targets),$($(target).force_ldadd)))))
$(foreach lib,$(all_libs),$(eval $(call define_libname_phony,$(lib))))

#############################################################################
# PHONY targets for . dir
# the first '.' means current dir, the second '.' is just a separator

PHONY += ..all ..clean ..distclean ..realclean ..codecheck ..check ..test

targets += ..all ..clean ..distclean ..realclean ..codecheck ..check ..test
..all.help = "'all' of this directory only"
..clean.help = "'clean' of this directory only"
..distclean.help = "'distclean' of this directory only"
..realclean.help = "'realclean' of this directory only"
..codecheck.help = "'codecheck' of this directory only"
..check.help = "'check' of this directory only"
..test.help = "'test' of this directory only"

..all: $(project.prebuild) $(project.targets) $(project.postbuild)

define clean_target
clean_$(1):
	$(if $(builder.verbose),,@)$(RM) $$(call get_target_full_path,$(1)) $$(call get_target_objects,$(1))
endef

$(foreach target,$(project.targets),$(eval $(call clean_target,$(target))))

clean_all_targets: $(foreach target,$(project.targets), clean_$(target))
	$(if $(builder.verbose),,@)$(foreach target,$(project.targets),$(call rm_target_objects,$(target)))

..clean: $(project.extra_clean) clean_all_targets
	$(if $(project.extra_clean_files),$(if $(builder.verbose),,@)$(RM) -r $(project.extra_clean_files))

..distclean: ..clean $(project.extra_distclean)
	$(if $(builder.verbose),,@)$(RM) -r $(foreach target,$(project.targets),\
		$(call get_target_objdir,$(target)) $(call get_target_objdir,$(target)).flags) .objects/

..realclean: ..distclean

..check : ..test

#############################################################################
# code check
get_cppcheck_includes = $(if $(builder.full_codecheck),$(call get_includes_of,$(1),$(2)))
# $(1) target
# $(2) source
define define_cppcheck_rule
PHONY += codecheck/$(1)/$(2)
codechecks += codecheck/$(1)/$(2)
codecheck/$(1)/$(2):
	@echo Checking $(2); \
	cppcheck -q --force --template 'gcc' --style $(call get_cppcheck_includes,$(1),$(2)) $(2) 2>&1 |\
	while read line; do \
		if $(call shell_regex_match,"$$$$line",': style: '); then \
			$(echo-e) "\033[1;33m$$$${line}\033[m" >&2; \
		elif $(call shell_regex_match,"$$$$line",': error: '); then \
			$(echo-e) "\033[1;31m$$$${line}\033[m" >&2; \
		else \
			if :; then $(echo-e) "$$$${line}" >&2; fi \
		fi \
	done
endef

builder.cppcheck_detected = $(shell cppcheck >/dev/null 2>&1; error=$$?; if [ $$error != 127 ]; then echo y; fi)

ifeq ($(builder.cppcheck_detected),)
error_if_no_cppcheck = \
	$(error $(shell $(echo-e) \
	"\033[1;31mcppcheck is not installed, you can get it from \033[1;4;34mhttp://sourceforge.net/projects/cppcheck/\033[m"))
endif

PHONY += check_cppcheck

check_cppcheck:
	$(error_if_no_cppcheck)

# expand check rules
$(foreach target,$(project.targets),\
	$(foreach source,$($(target).sources),\
		$(eval $(call define_cppcheck_rule,$(target),$(source)))\
	)\
)

..codecheck: check_cppcheck $(codechecks)

#############################################################################
# test
builder.test_output_xml ?= yes

get_target_test_argv0 = $(if $($(1).path),$(strip $($(1).path))/,./)$(strip $(if $($(1).name),$($(1).name),$(1)))

# target.test_path
# target.test_args

# $(1) test program name
define define_test_rule
$(1).test:
	@echo Running $(1)
	@$(call mkdir_if_not_exist, $(project.test_result_dir)/$(this_dir_part))
	$(call get_target_test_argv0,$(1)) $($(1).test_args) $(if $(call unify_bool_value,builder.test_output_xml),--gtest_output=xml:$(project.test_result_dir)/$(this_dir_part)/$(1).result.xml)
endef

# expand all test rules
$(foreach test,$(project.tests),\
	$(eval $(call define_test_rule,$(test)))\
)

..test: ..all $(addsuffix .test,$(project.tests))

#############################################################################
# include all depends if not cleaning
ifneq ($(all_depends),)
ifneq ($(MAKECMDGOALS),distclean)
ifneq ($(MAKECMDGOALS),realclean)
ifneq ($(MAKECMDGOALS),clean)
-include $(all_depends)
endif
endif
endif
endif

#############################################################################
# recursive targets, to support recursive building subdir

# all supportted goals
goals = all clean distclean realclean codecheck check test
PHONY += $(goals)

targets += $(goals)
all.help = "build all targets"
clean.help = "clean targets and building intermedia files"
distclean.help = "clean targets and all intermedia files for distribution"
realclean.help = "clean targets and all intermedia files for distribution, same as distclean"
check.help = "run test"

.DEFAULT_GOAL := all

# if subdirs does not include current dir, append it to the tail
ifneq ($(filter .,$(project.subdirs)),.)
  project.subdirs += .
endif

# ignored error for these goals during recursive make
ignore_error_goals := codecheck distclean
get_goal_ignore_error_flag = $(if $(findstring $(1),$(ignore_error_goals)),-)

# define goal dependancy, eg all: a.all b.all ..all
define define_goal
$(1):$$(foreach dir,$$(project.subdirs),$$(dir).$(1))
endef

# generate all goals' dependancy
$(foreach goal,$(goals),$(eval $(call define_goal,$(goal))))

# define subdir's goal, eg a.all b.all a.clean b.clean
# $(1) dir
# $(2) goal
define define_dir_goal
# rules for '.' has been defined previously, so skip
PHONY += $(1).$(2)
$(1).$(2):
	$(call get_goal_ignore_error_flag,$(2))@+$(MAKE) -C $(1) $(2)
targets += $(1).$(2)
$(1).$(2).help = "'$(2)' of $(1)"
endef

# generate all dirs' goals
$(foreach dir,$(filter-out .,$(project.subdirs)),\
	$(foreach goal,$(goals),\
		$(eval $(call define_dir_goal,$(dir),$(goal)))))

define define_help
PHONY += $(1).help
ifneq ($($(1).help),)
$(1).help:
	@printf "  \033[1m%-15s\033[m -- %s\n" $(1) $($(1).help)
endif
endef

# generate help details
$(foreach target,$(targets),$(eval $(call define_help,$(target))))
$(foreach option,$(options),$(eval $(call define_help,$(option))))
$(foreach command_line_variable,$(command_line_variables),$(eval $(call define_help,$(command_line_variable))))

help_targets_header:
	@$(echo-e) "Supported targets:"

help_targets: help_targets_header $(addsuffix .help,$(targets))
	@$(echo-e) "type $(MAKE) \033[4mtarget-name\033[m.help ... for details.\n"

PHONY += help_targets help_targets_header

help_options_header:
	@$(echo-e) "Supported options:"

help_options: help_options_header $(addsuffix .help,$(options))
	@$(echo-e) "type $(MAKE) \033[4mtarget-name\033[m.help ... for details.\n"

PHONY += help_options

PHONY += help

# command line help
help_commands_header:
	@$(echo-e) "Supported command line variables:"

help_commands: help_commands_header $(addsuffix .help,$(command_line_variables))
	@$(echo-e) "type $(MAKE) \033[4mvariable-name\033[m.help ... for details.\n"

help_prologue:
	@$(echo-e) "Smart Makefile, driven by Generic.mak $(builder.version), powered by phongchen.\n"

PHONY += help_prologue

help: help_prologue help_targets help_options help_commands

helps += help
help.help = "show this message"

PHONY += FORCE
FORCE: ;

.PHONY: $(PHONY)

# Tell versions [3.59,3.63) of GNU make not to export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
.NOEXPORT:

# for debug and test
dumptargets:
	@$(foreach target,$(project.targets),echo "$(target).sources = $($(target).sources)";)
