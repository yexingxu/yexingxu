<!--
 * @Description:
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2023-11-07 13:26:35
 * @LastEditors: chen, hua
 * @LastEditTime: 2023-11-07 14:45:20
-->
# Allocator

## 首先什么是Allocator？Allocator有什么用？

分配器是负责封装堆内存管理的对象，它们在整个标准库中使用，特别是STL容器使用它们来管理容器内部的所有内存分配,大部份情况下,程序员不用理会,标准容器使用默认的分配器称为std :: allocator,例如当你声明一个简单的vector对象时，C++编译器默认已经使用了内置的std::allocator,在标准库的vector模板当中，第二个模板参数_Alloc就是std::allocator，实际上，std::allocator也是一个类模板

自定义allocator有很多现实的原因。

- 有些嵌入式平台没有提供默认的malloc/free等底层内存管理函数，你需要继承std::allocator,并封装自定义版本的malloc/free等更底层的堆内存管理函数。
- 使用C++实现自己的数据结构，有时我们需要扩展(继承)std::allocator。
- 大部分用C++写的游戏程序都有自己重新实现的allocator。

## alignof alignas
C++11引入的关键字alignof，可直接获取类型T的内存对齐要求。alignof的返回值类型是size_t，用法类似于sizeof。
```cpp
sizeof(char):   1,      alignof(char):  1
sizeof(int):    4,      alignof(int):   4
sizeof(long):   8,      alignof(long):  8
sizeof(float):  4,      alignof(float): 4
sizeof(double): 8,      alignof(double):8

struct Foo {
  char c;
  int i1;
  int i2;
  long l;
};
sizeof(Foo):    24,     alignof(Foo):   8

0   # c 的偏移量为 0
4   # i1 的偏移量为 4， c  -> i1 中间填充了 3个字节，才满足 4 字节的内存对齐要求
8   # i2 的偏移量为 8,  i1 -> i2 无填充
16  # l 的偏移量为 16， i2 -> l  中间填充了4个字节，才满足8字节的内存对齐要求
```

上面讲述的内存对齐要求都是默认情况下的，有时候考虑到cacheline、以及向量化操作，可能会需要改变一个类的alignof值。

怎么办？

在C++11之前，需要依赖靠编译器的扩展指令，C++11之后可以借助alignas关键字。

> 比如，在C++11之前，gcc实现 alignas(alignment) 效果的方式为  __attribute__((__aligned__((alignment)))

```cpp
struct alignas(32) Foo {
  Foo() { std::cout << this << std::endl; }
  char c;
  int i1;
  int i2;
  long l;
};
sizeof(Foo):    32,     alignof(Foo):   32
```

对于类型T，需要满足如下两个条件。

1. alignment >= alignof(T)
仍然以Foo为例，在没有alignas修饰时，默认的Foo的内存对齐要求alignof(Foo)为8，现在尝试使用alignas让Foo的对齐要求为4，操作如下：
```cpp
struct alignas(4) Foo {
  char c;
  int i1;
  int i2;
  long l;
};

sizeof(Foo):    24,     alignof(Foo):   8
```

可以看出，此时的alignas是失效的，在其他编译器下也许直接编译失败。
2. alignment == pow(2, N)
```cpp
struct alignas(9) Foo {
  char c;
  int i1;
  int i2;
  long l;
};

$ g++ main.cc -o main && ./main
main.cc:20:19: error: requested alignment '9' is not a positive power of 2
   20 | struct alignas(9) Foo {
      |                   ^~~
```

### std::aligned_storage
在C++11中，也引入了一个满足内存对齐要求的静态内存分配类std::aligned_storage，其类模板原型如下：
```cpp
// in <type_traits>
template< std::size_t Len,
          std::size_t Align = /*default-alignment*/ >
struct aligned_storage;
```
类std::aligned_storage对象构造完成时，即分配了长度为Len个字节的内存，且该内存满足大小为 Align 的对齐要求。

下面，我们先来看看 cpprefernece[2] 给的一个demo，来熟悉下怎么使用std::aligned_storage。

类 StaticVector ，是一个满足内存对齐要求的静态数组，模板参数T是元素类型，N是数组元素个数。

```cpp
template<typename T, size_t N>
class StaticVector {
public:
    StaticVector() {
      std::cout << alignof(T) << "/" << sizeof(T)<< std::endl;
      for (int idx = 0; idx < N; ++idx) {
        std::cout << &data[idx] << std::endl;
      }
    }

    ~StaticVector() {
      for(size_t pos = 0; pos < m_size; ++pos) {
        reinterpret_cast<T*>(data+pos)->~T();
      }
    }

    template<typename ...Args>
    void emplace_back(Args&&... args) {
      if(m_size >= N) {
        throw std::bad_alloc{};
      }
      new(data+m_size) T(std::forward<Args>(args)...);
      ++m_size;
    }

    const T& operator[](size_t pos "") const {
      return *reinterpret_cast<const T*>(data+pos);
    }

private:
  // std::aligned_storage<sizeof(T), alignof(T)>::type data[N]; // C++11
  std::aligned_storage_t<sizeof(T), alignof(T)> data[N];        // c++14
  size_t m_size = 0;
};
```