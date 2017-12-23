# Project2 实验报告
###### 151220136 许卓尔

## 实现功能

1. **进行语义分析且检查错误类型**
> 部分由队友赵文昊完成

2. **附加要求**

    - 要求2.1

    - 要求2.3（由队友赵文昊完成）



## 实现方法

#### 进行语义分析且检查错误类型

- 属性文法
    
    由于Bison采用了自底向上的LALR(1)分析技术，对于SDT无法实现继承属性的计算，因此需要通过Bison建立语法树，并在语法树的基础上自顶向下进行SDT；

    通过上下文无关文法，对语法分析树进行遍历，并在遍历过程中进行语义分析；

- 类型表示
    
    参考了实验指导书中取自老虎书的代码风格，对类型的数据结构进行设计；

    有所改变的是，自己设计了 Structure、 Function 的数据结构，具体可见/code/Semantic.h;

    在此基础上，由于Exp属性文法中需要进行左值、右值判断，在 strcut Type_ 的基础上加入枚举类型成员变量 
```
struct Type_{
    ...
    enum{LEFT,RIGHT,BOTH} assign; 
};
```



- 符号表

    符号表采用散列表的实现方案，设计如下数据结构；

    采用P.J.Weinberger提出的哈希函数；

    对于冲突，采用开散列的处理方式；
    > 由于基本要求中，对于大部分类型判断采用名等价的方案，于是加入 name 属性方便判断；

```
typedef struct Entry_{
    char *name;
    Type type;
    struct Entry_ *next;
}Entry;
extern Entry *hashTable[HASH_SIZE];
```


- 错误检查

    在语法制导翻译的过程中进行错误检查；

     - Error Type 1~2、17： 调用的时候于符号表查找，若为NULL报错；
     - Error Type 3~4、16： 插入符号表时进行查重；
     - Error Type 5~14： 在Exp中，对于Type lhs、Type rhs进行类型判断，不满足条件则报错；
     - Error Type 15： 结构体域名插入符号表时查重；对于赋值，判断其文法上一层的来源是否为StructSpecifier；

#### 要求2.1

##### 进行函数声明

- 添加文法 ExtDef -> Specifier FunDec SEMI；
- 对于 struct Function_ 添加变量 isDefined 判断其为声明或定义；
- 函数插入符号表时，进行判断，具体实现可见 /code/HashTable.c/funcInsertCheck()；
- Error Type 18： 在文法 Program 完成后，对符号表进行查找，若仍有函数 isDefined==0 ，则报错；
- Error Type 19： 在funcInsertCheck()时，进行返回类型、参数类型判断，若不一致则标错；
```
struct Function_{
    ...
    int isDefined;  // 判断为声明或实现；
};
```



## 编译介绍

##### Makefile文件

make parser | make
> 对词法分析器、语法分析器进行编译并生成相应的parser程序；

make clean
> 对生成文件进行清理；

./parser ./pretest/FileName.txt
> 对当前路径下pretest文件夹中的FileName.txt文件进行语义分析并检查是否有错；
