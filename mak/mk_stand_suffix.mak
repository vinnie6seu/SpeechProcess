
##===========================================================

###################
### make target ###
###################
##===========================================================
# ʹ��.PHONY����ָ�������α�ļ�,������ļ�����ͻ������,���Ŀ�����make����ִ��
.PHONY     : all bin lib dll

all        : bin lib dll
bin        : $(BIN_TARG) 
lib        : $(LIB_TARG)
dll        : $(DLL_TARG)
##----------------------------------------------------------
# �����ɿ�ִ�г���ʱ,��Ҫָ�����ж�̬��̬��·��,��̬��˳���ǴӾɵ��£�Խ�ײ�Խǰ�棩,��̬��˳���Ǵ��µ��ɣ�Խ�ײ�Խ���棩,�ܵ���˵��̬��ŵ���̬��ǰ��
# ���ɿ�ִ�г���ʱ��ֱ���õ���ͷ�ļ�����������漰�����п�������
$(BIN_TARG): $(BIN_OBJS)  
	@echo "Linking   [bin   ] file:[$@] ..."
	@if [ ! -d $(BIN) ]; then mkdir -p $(BIN) ; fi
	@$(GXX) $(BIN_FLAGS) $(INCPATH) $(LDPATH) -o $@ $(BIN_OBJS) $(USERLIBS)

##----------------------------------------------------------
# ���붯̬��ʱ������Ҫָ��������Ŀ��ļ�·����ֻ��Ҫָ���������ͷ�ļ�·������
$(DLL_TARG): $(DLL_OBJS) 
	@echo "Linking   [dll   ] file:[$@] ..."
	@if [ ! -d $(LIB) ]; \
         then \
             mkdir -p $(LIB) ; \
         fi
	@$(GXX) $(DLL_FLAGS) $(INCPATH) -o $@ $(DLL_OBJS) $(USERLIBS)

##----------------------------------------------------------
$(LIB_TARG): $(LIB_OBJS) 
	@echo "Archiving [lib   ] file:[$@] ..."
	@if [ ! -d $(LIB) ]; \
         then \
             mkdir -p $(LIB) ; \
         fi
	@$(AR) $(LIB_FLAGS) $@ $(LIB_OBJS) $(USERLIBS)
##===========================================================

# �����.o�ļ�
##===========================================================
# .SUFFIXES:
# .SUFFIXES: .c .cpp .cc .C .o
COMPILE.c   = $(GCC) $(CFLAGS) $(INCPATH) -O0 -c  
COMPILE.cxx = $(GXX) $(CXXFLAGS) $(INCPATH) -c  
##----------------------------------------------------------
.c.o     :
	@echo "Compiling [c     ] file:[$*.o] ..."
	@$(COMPILE.c) $*.c -o $@
    
.C.o     :
	@echo "Compiling [C     ] file:[$*.o] ..."
	@$(COMPILE.c) $*.C -o $@

.cpp.o   :
	@echo "Compiling [cpp     ] file:[$*.o] ..."
	@$(COMPILE.cxx) $*.cpp -o $@
	
.cc.o    :
	@echo "Compiling [cc     ] file:[$*.o] ..."
	@$(COMPILE.cxx) $*.cc -o $@
    
.CPP.o   :
	@echo "Compiling [CPP     ] file:[$*.o] ..."
	@$(COMPILE.cxx) $*.CPP -o $@
    
.c++.o   :
	@echo "Compiling [c++     ] file:[$*.o] ..."
	@$(COMPILE.cxx) $*.c++ -o $@
 
.cxx.o   :
	@echo "Compiling [cxx     ] file:[$*.o] ..."
	@$(COMPILE.cxx) $*.cxx -o $@ 
    
.cp.o   :
	@echo "Compiling [cp     ] file:[$*.o] ..."
	@$(COMPILE.cxx) $*.cp -o $@ 
##===========================================================
    
# ����.d�����ļ�
##===========================================================
DEP_OPT     = $(shell if `$(GCC) --version | grep "GCC" >/dev/null`; then echo "-MM -MP"; else echo "-M"; fi ) 
DEPEND      = $(GCC) $(DEP_OPT) $(CFLAGS) $(INCPATH)
DEPEND.d    = $(subst -g ,,$(DEPEND)) 
##----------------------------------------------------------
%.d:%.c
	@set -e; rm -f $@; echo -n $(dir $<) > $@.$$$$; $(DEPEND.d) $< >> $@.$$$$; \
    sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
    rm -f $@.$$$$

%.d:%.C
	@set -e; rm -f $@; echo -n $(dir $<) > $@.$$$$; $(DEPEND.d) $< >> $@.$$$$; \
    sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
    rm -f $@.$$$$  

%.d:%.cpp
	@set -e; rm -f $@; echo -n $(dir $<) > $@.$$$$; $(DEPEND.d) $< >> $@.$$$$; \
    sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
    rm -f $@.$$$$

%.d:%.cc
	@set -e; rm -f $@; echo -n $(dir $<) > $@.$$$$; $(DEPEND.d) $< >> $@.$$$$; \
    sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
    rm -f $@.$$$$     

%.d:%.CPP
	@set -e; rm -f $@; echo -n $(dir $<) > $@.$$$$; $(DEPEND.d) $< >> $@.$$$$; \
    sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
    rm -f $@.$$$$

%.d:%.c++
	@set -e; rm -f $@; echo -n $(dir $<) > $@.$$$$; $(DEPEND.d) $< >> $@.$$$$; \
    sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
    rm -f $@.$$$$

%.d:%.cxx
	@set -e; rm -f $@; echo -n $(dir $<) > $@.$$$$; $(DEPEND.d) $< >> $@.$$$$; \
    sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
    rm -f $@.$$$$    
    
%.d:%.cp
	@set -e; rm -f $@; echo -n $(dir $<) > $@.$$$$; $(DEPEND.d) $< >> $@.$$$$; \
    sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
    rm -f $@.$$$$
##===========================================================

# �Ƿ�ʹ��.d�����ļ�
##===========================================================
ifdef DEP    
  sinclude $(ALLDEPS)    
endif
##===========================================================

# ����
##=========================================================== 
clean:
	-rm -f $(BIN_TARG) $(LIB_TARG) $(DLL_TARG)
	-rm -f $(ALLDEPS) $(ALLOBJS)
#	-find ./ -type f -name "*.[o|d]" -delete
# �Ƿ�ʹ��.d�����ļ�
##===========================================================
    

###########################
### get bison_parser.cpp###
###########################
##===========================================================
.PHONY: parser cleanparser

SRCPARSER = $(CURDIR)/parser

# $(SRCPARSER)/bison_parser.cpp  : parser
# $(SRCPARSER)/flex_lexer.cpp    : parser

parser:
	make -C $(SRCPARSER)/

cleanparser:
	make -C $(SRCPARSER)/ clean
##===========================================================    
    
##########################
### make sqlparsertest ###
##########################
##===========================================================
.PHONY: sqlparsertest cleansqlparsertest

SRCTEST = $(CURDIR)/sqlparsertest

# �л�Ŀ¼���б���
sqlparsertest:
	make -C $(SRCTEST)/

cleansqlparsertest:
	make -C $(SRCTEST)/ clean   
##===========================================================

###############
### install ###
###############
##===========================================================
.PHONY: install

install:
	cp $(DLL_TARG) $(INSTALL)/lib/$(notdir $(DLL_TARG))
##===========================================================   

# makeʹ�ý���
##===========================================================
.PHONY: help
help: 
	@echo 
	@echo '-------------------------------------------------'    
	@echo '  ͨ�ð汾Makefile!'  
	@echo '  qbyao <15006181953@163.com>'  
	@echo  
	@echo '  Usage: make [TARGET]'  
	@echo '  TARGETS:'  
	@echo '  all       ��������.'  
	@echo '  DEP=yes   ����.d�����ļ�,���������ļ����б���.'  
	@echo '  bin       ����ָ��bin�ļ�.'  
	@echo '  lib       ����ָ��.a�ļ�.'  
	@echo '  dll       ����ָ��.so�ļ�.'  
	@echo '  clean     ����[.o] [.d] [bin] [lib] [dll]�ļ�.'  
	@echo '  show      ��ʾ����.'  
	@echo '  help      ��ӡ������Ϣ.'
	@echo '-------------------------------------------------'       
	@echo   
##===========================================================  
 
# ����չʾ
##===========================================================
.PHONY: show
show:  
	@echo 
	@echo '-------------------------------------------------'    
	@echo 'CURDIR      :' $(CURDIR)
	@echo 'EXCLUDE_DIRS:' $(EXCLUDE_DIRS)
	@echo 'ADD_DIRS    :' $(ADD_DIRS)
	@echo 'SUBSRCDIRS  :' $(SUBSRCDIRS)
	@echo 'SRCDIRS     :' $(SRCDIRS)
	
	@echo 
	
	@echo 'ALLFILES    :' $(ALLFILES)
	@echo 'ALLDEPS     :' $(ALLDEPS) 
	@echo 'ALLOBJS     :' $(ALLOBJS)
	
	@echo 
	
	@echo 'USERLIBS    :' $(USERLIBS)   
	@echo 'INCPATH     :' $(INCPATH)   
	@echo 'LDPATH      :' $(LDPATH)   
	
	@echo 
	  
	@echo 'BIN_TARG    :' $(BIN_TARG)
	@echo 'LIB_TARG    :' $(LIB_TARG)
	@echo 'DLL_TARG    :' $(DLL_TARG) 
	@echo '-------------------------------------------------'       
	@echo       
##=========================================================== 
   


    
    
## �����.a����.so�ķ���    
## :=������ǰֵ
# GCC := gcc  
# GXX := g++  
# LD  := ld  
# AR  := ar  
  
# LDFLAGS := -shared -fpic  
# ARFLAGS := -rc  
# CFLAGS  := -g -Wall -fpic  
  
# DIR_INC := ./inc  
# DIR_SRC := ./src  
# DIR_OBJ := ./obj  
# DIR_LIB := ./lib  
  
# LIB_NAME    := commonLib  

## �г�Ŀ¼DIR_SRC�µ�����.c��׺�ļ�,��Դ�·��  
# SRC := $(wildcard ${DIR_SRC}/*.c)
## �������ļ�.c��׺��.o����
## ��$(patsubst %.c,%.o,$(dir) )�У�patsubst��$(dir)�еı������Ϻ�׺��.c��ȫ���滻��.o,�ȼ���obj=$(dir:%.c=%.o) 
## notdirȥ��Ŀ¼
# OBJ := $(patsubst %.c, ${DIR_OBJ}/%.o, $(notdir ${SRC}))  
  
# SHARE_LIB   := $(LIB_NAME).so  
# DYNAMIC_LIB := $(LIB_NAME).a  
  
# INC := -I${DIR_INC}  
   
# all:$(OBJ)
    ## ��.so
    # $(LD) $(LDFLAGS) -o $(DIR_LIB)/$(SHARE_LIB) $(OBJ)
    ## ��.a
    # #${AR} ${ARFLAGS} $(DIR_LIB)/$(DYNAMIC_LIB) $(OBJ) 
    ## ��.o    
    # ${DIR_OBJ}/%.o:${DIR_SRC}/%.c  
    # @echo Compiling $(OBJ) ...  
    # $(GCC) $(CFLAGS) -o $@ -c $< ${INC} -g  
# clean:  
    # rm -rf $(DIR_OBJ)/*.o $(DIR_LIB)/*.so $(DIR_LIB)/*.a

    
    
# $@  ��ʾĿ���ļ�
# $^  ��ʾ���е������ļ�
# $<  ��ʾ��һ�������ļ�
# $?  ��ʾ��Ŀ�껹Ҫ�µ������ļ��б�
# $% ����Ŀ���Ǻ������ļ��У���ʾ�����е�Ŀ���Ա�������磬���һ��Ŀ���ǡ�foo.a(bar.o)������ô����$%�����ǡ�bar.o������$@�����ǡ�foo.a�������Ŀ�겻�Ǻ������ļ���Unix����[.a]��Windows����[.lib]������ô����ֵΪ�ա�
# $+ �����������$^����Ҳ����������Ŀ��ļ��ϡ�ֻ������ȥ���ظ�������Ŀ�ꡣ
# $* ���������ʾĿ��ģʽ�С�%������֮ǰ�Ĳ��֡����Ŀ���ǡ�dir/a.foo.b��������Ŀ���ģʽ�ǡ�a.%.b������ô����$*����ֵ���ǡ�dir/a.foo��������������ڹ����й������ļ����ǱȽ��нϡ����Ŀ����û��ģʽ�Ķ��壬��ô��$*��Ҳ�Ͳ��ܱ��Ƶ��������ǣ����Ŀ���ļ��ĺ�׺��make��ʶ��ģ���ô��$*�����ǳ��˺�׺����һ���֡����磺���Ŀ���ǡ�foo.c������Ϊ��.c����make����ʶ��ĺ�׺�������ԣ���$*����ֵ���ǡ�foo�������������GNU make�ģ����п��ܲ������������汾��make�����ԣ���Ӧ�þ�������ʹ�á�$*����������������������Ǿ�̬ģʽ�С����Ŀ���еĺ�׺��make������ʶ��ģ���ô��$*�����ǿ�ֵ��



# ��һ������@set -e��@�ؼ��ָ���make������������set -e�������ǣ������������ķ���ֵ��0ʱ�������˳���
# ��ôΪʲôҪ�Ѽ�������д�ڡ�ͬһ�С����Ƕ���make��˵����Ϊ\�����þ��������У������÷ֺŸ���ÿ�������Ϊ��Makefile����������ʹ��һ��������������һ�������������Ҫset -e�����ں�������
# �ڶ�������gcc -MM $< > $@.$$$$, �����Ǹ���Դ�ļ�����������ϵ�������浽��ʱ�ļ��С��ڽ�����$<��ֵΪ��һ�������ļ�����seq.c)��$$$$Ϊ�ַ���"$$"������makefile�����е�$�ַ����������ַ�����ʹ�ڵ�����֮�У�����Ҫ�õ���ͨ�ַ�$����Ҫ��$$��ת��; ��$$��shell���������������ֵΪ��ǰ���̺ţ�ʹ�ý��̺�Ϊ��׺�����ƴ�����ʱ�ļ�����shell��̳��������������ɱ�֤�ļ�Ψһ�ԡ�
# ���������������ǽ�Ŀ���ļ�����������ϵ��Ŀ¼�б��У������浽Ŀ���ļ�������������ʽ���־Ͳ�˵�ˣ�ΨһҪע������ڽ�����$*��$*��ֵΪ��һ�������ļ�ȥ����׺�����ƣ����Ｔ��seq)��
# ���ĸ������ǽ�����ʱ�ļ�ɾ����
