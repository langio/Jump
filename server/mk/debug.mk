#使用方法：make -f 目标makefile -f debug.mk 想查看的目标变量名

%:
	@echo '$*=$($*)'

d-%:
	@echo '$*=$($*)'
	@echo '  origin = $(origin $*)'
	@echo '  value = $(value  $*)'
	@echo '  flavor = $(flavor $*)'
