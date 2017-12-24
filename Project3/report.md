# Project2 实验报告
###### 151220136 许卓尔
###### 151220168 赵文昊

## 实现功能

1. **源代码翻译为中间代码**
2. **优化中间代码的生成**
3. **以线性结构输出中间代码**


## 实现方法

#### 源代码翻译为中间代码

- 中间代码数据结构

    中间代码使用双向链表存储在内存中，待全部生成完毕后进行优化；

    链表以增加实现的复杂性为代价换得了极大的灵活性，可以进行高效的插入、删除以及调换位置操作，并且几乎不存在代码最大行数的限制；


- 中间代码翻译

    在实验二语义分析部分添加中间代码生成的内容，使编译器一边进行语义检查一遍生成中间代码，并插入双向链表中；

    根据相应文法，进行翻译，生成相应中间代码，由于语义检查的过程中伴随符号表插入等操作，可以直接get到变量信息，更为快捷；


```
typedef struct Operand_{
    enum{TEMPVAR,VARIABLE,CONSTANT... }kind;
    union{
        int var_no;
        char* value;
        ...
    }u;
    Operand next;
} Operand_;

typedef struct InterCode_{
    enum{ASSIGN_N,ADD_N,SUB_N,MUL_N,DIV_N,...}kind;
    union{
        struct{ Operand op; }sinop;
        struct{ Operand left,right; }assign;
        struct{ Operand op1,op2,result; }binop;
        ...
    }u;
    InterCode pre;      // 实现双向链表
    InterCode next;
} InterCode_;
```


#### 优化中间代码的生成

- 消除冗余的跳转

- 删除未被指向的label 

    对中间代码双向链表进行遍历；

    对于Label类型的代码，判断是否会跳转到此Label；

    若无，则为未被指向的label，删除；

- 直接计算常数
    
    对中间代码双向链表进行遍历；

    对于计算类型的操作，判断左手端、右手端是否为常量；

    若均为常量，则将该代码替换为lhs、rhs计算结果的常量；


#### 以线性结构输出中间代码

- 输出中间代码

    对优化后的双向链表进行遍历；
    
    根据实验要求中的规则，输出中间代码到指定文件流中；


## 编译介绍

##### Makefile文件

make parser | make
> 对词法分析器、语法分析器进行编译并生成相应的parser程序；

make clean
> 对生成文件进行清理；

./parser ./testcase/TestFile.txt OutFileName.ir
> 对当前路径下pretest文件夹中的TestFile.txt文件进行中间代码翻译与优化，并以线性结构输出到自定义的OutFileName.ir文件中；
