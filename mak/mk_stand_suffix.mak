
##===========================================================

###################
### make target ###
###################
##===========================================================
# 使用.PHONY可以指明其后是伪文件,解决与文件名冲突的问题,其后目标进行make总是执行
.PHONY     : all bin lib dll

all        : bin lib dll
bin        : $(BIN_TARG) 
lib        : $(LIB_TARG)
dll        : $(DLL_TARG)
##----------------------------------------------------------
# 在生成可执行程序时,需要指定所有动态静态库路径,动态库顺序是从旧到新（越底层越前面）,静态库顺序是从新到旧（越底层越后面）,总的来说动态库放到静态库前面
# 生成可执行程序时，直接用到的头文件必须包含，涉及的所有库必须包含
$(BIN_TARG): $(BIN_OBJS)  
	@echo "Linking   [bin   ] file:[$@] ..."
	@if [ ! -d $(BIN) ]; then mkdir -p $(BIN) ; fi
	@$(GXX) $(BIN_FLAGS) $(INCPATH) $(LDPATH) -o $@ $(BIN_OBJS) $(USERLIBS)

##----------------------------------------------------------
# 编译动态库时，不需要指定其他库的库文件路径，只需要指定其他库的头文件路径即可
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

# 编译出.o文件
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
    
# 生成.d依赖文件
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

# 是否使用.d依赖文件
##===========================================================
ifdef DEP    
  sinclude $(ALLDEPS)    
endif
##===========================================================

# 清理
##=========================================================== 
clean:
	-rm -f $(BIN_TARG) $(LIB_TARG) $(DLL_TARG)
	-rm -f $(ALLDEPS) $(ALLOBJS)
#	-find ./ -type f -name "*.[o|d]" -delete
# 是否使用.d依赖文件
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

# 切换目录进行编译
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

# make使用解释
##===========================================================
.PHONY: help
help: 
	@echo 
	@echo '-------------------------------------------------'    
	@echo '  通用版本Makefile!'  
	@echo '  qbyao <15006181953@163.com>'  
	@echo  
	@echo '  Usage: make [TARGET]'  
	@echo '  TARGETS:'  
	@echo '  all       编译链接.'  
	@echo '  DEP=yes   生成.d依赖文件,按照依赖文件进行编译.'  
	@echo '  bin       生成指定bin文件.'  
	@echo '  lib       生成指定.a文件.'  
	@echo '  dll       生成指定.so文件.'  
	@echo '  clean     清理[.o] [.d] [bin] [lib] [dll]文件.'  
	@echo '  show      显示变量.'  
	@echo '  help      打印帮助信息.'
	@echo '-------------------------------------------------'       
	@echo   
##===========================================================  
 
# 变量展示
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
   


    
    
## 编译出.a或者.so的方法    
## :=覆盖以前值
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

## 列出目录DIR_SRC下的所有.c后缀文件,相对带路径  
# SRC := $(wildcard ${DIR_SRC}/*.c)
## 将所有文件.c后缀用.o代替
## 在$(patsubst %.c,%.o,$(dir) )中，patsubst把$(dir)中的变量符合后缀是.c的全部替换成.o,等价于obj=$(dir:%.c=%.o) 
## notdir去掉目录
# OBJ := $(patsubst %.c, ${DIR_OBJ}/%.o, $(notdir ${SRC}))  
  
# SHARE_LIB   := $(LIB_NAME).so  
# DYNAMIC_LIB := $(LIB_NAME).a  
  
# INC := -I${DIR_INC}  
   
# all:$(OBJ)
    ## 编.so
    # $(LD) $(LDFLAGS) -o $(DIR_LIB)/$(SHARE_LIB) $(OBJ)
    ## 编.a
    # #${AR} ${ARFLAGS} $(DIR_LIB)/$(DYNAMIC_LIB) $(OBJ) 
    ## 编.o    
    # ${DIR_OBJ}/%.o:${DIR_SRC}/%.c  
    # @echo Compiling $(OBJ) ...  
    # $(GCC) $(CFLAGS) -o $@ -c $< ${INC} -g  
# clean:  
    # rm -rf $(DIR_OBJ)/*.o $(DIR_LIB)/*.so $(DIR_LIB)/*.a

    
    
# $@  表示目标文件
# $^  表示所有的依赖文件
# $<  表示第一个依赖文件
# $?  表示比目标还要新的依赖文件列表
# $% 仅当目标是函数库文件中，表示规则中的目标成员名。例如，如果一个目标是“foo.a(bar.o)”，那么，“$%”就是“bar.o”，“$@”就是“foo.a”。如果目标不是函数库文件（Unix下是[.a]，Windows下是[.lib]），那么，其值为空。
# $+ 这个变量很像“$^”，也是所有依赖目标的集合。只是它不去除重复的依赖目标。
# $* 这个变量表示目标模式中“%”及其之前的部分。如果目标是“dir/a.foo.b”，并且目标的模式是“a.%.b”，那么，“$*”的值就是“dir/a.foo”。这个变量对于构造有关联的文件名是比较有较。如果目标中没有模式的定义，那么“$*”也就不能被推导出，但是，如果目标文件的后缀是make所识别的，那么“$*”就是除了后缀的那一部分。例如：如果目标是“foo.c”，因为“.c”是make所能识别的后缀名，所以，“$*”的值就是“foo”。这个特性是GNU make的，很有可能不兼容于其它版本的make，所以，你应该尽量避免使用“$*”，除非是在隐含规则或是静态模式中。如果目标中的后缀是make所不能识别的，那么“$*”就是空值。



# 第一个命令@set -e。@关键字告诉make不输出该行命令；set -e的作用是，当后面的命令的返回值非0时，立即退出。
# 那么为什么要把几个命令写在”同一行“（是对于make来说，因为\的作用就是连接行），并用分号隔开每个命令？因为在Makefile这样做才能使上一个命令作用于下一个命令。这里是想要set -e作用于后面的命令。
# 第二个命令gcc -MM $< > $@.$$$$, 作用是根据源文件生成依赖关系，并保存到临时文件中。内建变量$<的值为第一个依赖文件（那seq.c)，$$$$为字符串"$$"，由于makefile中所有的$字符都是特殊字符（即使在单引号之中！），要得到普通字符$，需要用$$来转义; 而$$是shell的特殊变量，它的值为当前进程号；使用进程号为后缀的名称创建临时文件，是shell编程常用做法，这样可保证文件唯一性。
# 第三个命令作用是将目标文件加入依赖关系的目录列表中，并保存到目标文件。关于正则表达式部分就不说了，唯一要注意的是内建变量$*，$*的值为第一个依赖文件去掉后缀的名称（这里即是seq)。
# 第四个命令是将该临时文件删除。
