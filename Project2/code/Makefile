# GNU make手册：http://www.gnu.org/software/make/manual/make.html
# ************ 遇到不明白的地方请google以及阅读手册 *************

# 编译器设定和编译选项
CC = gcc
FLEX = flex
BISON = bison
CFLAGS = -std=c99

# 编译目标：code目录下的所有.c文件
CFILES = $(shell find ./ -name "*.c")
OBJS = $(CFILES:.c=.o)
LFILE = $(shell find ./ -name "*.l")
YFILE = $(shell find ./ -name "*.y")
LFC = $(shell find ./ -name "*.l" | sed s/[^/]*\\.l/lex.yy.c/)
YFC = $(shell find ./ -name "*.y" | sed s/[^/]*\\.y/syntax.tab.c/)
LFO = $(LFC:.c=.o)
YFO = $(YFC:.c=.o)

#scanner:
#	$(CC) -o scanner $(CFILE) -lfl

parser: syntax $(filter-out $(LFO),$(OBJS))
	$(CC) -o parser $(filter-out $(LFO),$(OBJS)) -lfl -ly

syntax: lexical syntax-c
	$(CC) -c $(YFC) -o $(YFO)

lexical: $(LFILE)
	$(FLEX) -o $(LFC) $(LFILE)

syntax-c: $(YFILE)
	$(BISON) -o $(YFC) -d -v $(YFILE)

-include $(patsubst %.o, %.d, $(OBJS))

# 定义的一些伪目标
.PHONY: clean test
test:
	#./parser ../pretest/1.txt
	#./parser ../pretest/2.txt
	#./parser ../pretest/3.txt
	#./parser ../pretest/4.txt
	#./parser ../pretest/5.txt
	#./parser ../pretest/6.txt
	#./parser ../pretest/7.txt
	#./parser ../pretest/8.txt
	#./parser ../pretest/9.txt
	#./parser ../pretest/10.txt
	#./parser ../pretest/11.txt
	#./parser ../pretest/12.txt
	#./parser ../pretest/13.txt
	#./parser ../pretest/14.txt
	#./parser ../pretest/15.txt
	#./parser ../pretest/16.txt
	#./parser ../pretest/17.txt
	#./parser ../pretest/18.txt
	#./parser ../pretest/19.txt
	./parser ../pretest/20.txt
	./parser ../pretest/21.txt
	#./parser ../pretest/22.txt
	#./parser ../pretest/23.txt

clean:
	rm -f parser lex.yy.c syntax.tab.c syntax.tab.h syntax.output
	rm -f $(OBJS) $(OBJS:.o=.d)
	rm -f $(LFC) $(YFC) $(YFC:.c=.h)
	rm -f *~
