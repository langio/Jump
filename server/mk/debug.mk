#ʹ�÷�����make -f Ŀ��makefile -f debug.mk ��鿴��Ŀ�������

%:
	@echo '$*=$($*)'

d-%:
	@echo '$*=$($*)'
	@echo '  origin = $(origin $*)'
	@echo '  value = $(value  $*)'
	@echo '  flavor = $(flavor $*)'
