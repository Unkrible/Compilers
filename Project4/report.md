# Project4 实验报告
###### 151220136 许卓尔
###### 151220168 赵文昊

## 实现功能

1. **指令选择**(151220136)
2. **寄存器选择**(151220136)
3. **栈管理**(151220168)


## 实现方法

#### 指令选择

- 指令选择

    通过遍历IR线性表，根据IR->kind，参照指导攻略中的IR-MIPS指令表进行翻译，输出到out.s中；
    


#### 寄存器选择

- 寄存器数据结构

    定义了全局变量为寄存器的名字，在初始化过程中进行赋值，这样可以通过下标访问寄存器；

    寄存器中存储了当前寄存器中变量的指针；

- 选择算法

    由于时间原因，直接在t0~t7中进行了队列式寄存器分配——先来先分配；

    通过void swReg(int)函数，在使用完寄存器后马上把相应的变量存入内存；

- 变量管理

    由于变量不重名、采用名等价的形式，因此将所有的变量记录在Var_t的链表中；

    对于新的变量，即认为是当前函数的临时变量，存储到函数的栈空间中；

```
typedef struct Var_t {
    char *name;
    int reg;
    int offset;
    struct Var_t *next;
} Var_t;

typedef struct Register_ {
    char *name;
    Var_t *var;
} Register_;

Register_ regs[];
char* regName[];
Var_t *varList;
```

#### 栈管理

- 参数传入

    前4个参数，存入a0~a3寄存器中；

    对于其他参数，按序存入栈中，在函数中，利用fp拿出实参存入栈中作为临时变量；

- 活动记录

    只需在调用前将$fp、$ra压栈，在RETURN后将$ra、$fp弹出即可



> 因为没有写寄存器分配，于是都作为栈中临时变量处理了，在Var_t中加入了offset确定相对于fp的偏移；


## 编译介绍

##### Makefile文件

make parser | make
> 对词法分析器、语法分析器进行编译并生成相应的parser程序；

make clean
> 对生成文件进行清理；

./parser ./testcase/TestFile.txt OutFileName.s
> 对当前路径下pretest文件夹中的TestFile.txt文件进行编译，生成MIPS32汇编代码到OutFileName.s文件中；
