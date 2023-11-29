# Muti Thread

## memory order
### 为什么有 memory order？

因为代码执行的顺序确实和写的顺序不一样，有两个因素会导致重排，

一是编译器为了优化性能，生成汇编的内存读写顺序会与原代码不一样，当然是在保证正确的
基础上，但是比如两个独立的变量编译器没有保证他们读写顺序的义务；

二是cpu执行的复杂性，cpu指令的执行是流水线式的，多个指令是同时执行的，多个cpu有
独立的缓存，甚至有内部的 store buffer，cpu能保证对一个变量读写操作是遵从代码顺序的，
但是多个变量读写之间顺序可能是重排的，自然多个cpu之间看到memory被读写的顺序可能
不一样。

c++的memory order就是对这些行为做限制，阻止编译器和cpu对部分内存读写重排，
这会以牺牲部分性能为代价来保证多核程序执行的正确性，当然比起常见的基于mutex的实现
性能有希望更好。

### 常用的 memory order
常用的c++ memory_order分三个等级，relaxed，acquire/release，seq_cst。

他们的定义非常学术，这里只谈应用场景：

- seq_cst是默认的模式，保证正确，如果不追求极致的性能就用这个模式；
- relaxed由于只保护一个变量的读写顺序，适合统计类变量，比如多线程计数；
- acquire/release常用于atomic变量是另外一些非atomic变量的index或者指针之类的情况，

acquire/release其实就是字面意思，比如先对队列一个元素写入(non atomic)，然后增加队列的header index(atomic)，
将对队列的操作release给别的核心，相对应的，其他核心想要获取(acquire)队列的元素，先获取
index(atomic)，然后读取index指向的元素的值(non atomic)。

## atomic


## future/promise