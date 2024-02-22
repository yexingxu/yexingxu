
# vector

vector的一些主要特点和应用场景：

动态大小：与传统的数组不同，vector可以根据需要动态地扩展或缩减大小。这意味着你不需要事先知道数据的数量。
随机访问：就像数组一样，vector支持随机访问，这意味着你可以通过索引直接访问任何元素，访问时间是常数时间复杂度（O(1)）。

内存管理：vector在内部管理其存储的内存。当元素被添加到vector中，并且当前分配的内存不足以容纳它们时，它会自动重新分配更多的内存。

灵活性：你可以在vector的末尾添加或删除元素，而且效率很高。但在中间或开始位置插入或删除元素可能会比较慢，因为这可能需要移动现有的元素。

应用场景

动态数据集合：当你需要一个可以根据数据量动态调整大小的数组时，vector是一个很好的选择。例如，处理用户输入的数据集，其中输入数量事先未知。

需要快速访问的数据：由于vector支持随机访问，它非常适合于需要频繁读取元素的情况，比如查找或排序算法中。

性能敏感的应用：由于其元素紧密排列在连续的内存块中，vector通常提供高效的内存访问性能，适合用于性能敏感的应用。

## vector深浅拷贝问题
首先来看看以下代码:
```cpp
vector<vector<int>> vv(3,vector<int>(5));
```
这是一个二维数组,初始化为三行五列
```cpp
vector<vector<int>> vv(3,vector<int>(5));
vector<vector<int>> x(vv);
```
这是在拷贝构造类对象 x

自我实现的拷贝构造使用的是 memcpy:
```cpp
Vector(const Vector<T>& v)
{
	assert(v._start && v._finish && v._endofsto);
	_start = new T[v.capacity()];//给size或capacity都可以
	memcpy(_start, v._start, sizeof(T) * v.size());
}
```
然而memcpy是逐个字节拷贝. 当数组是一维时,用memcpy没有问题，但是当数组是二维数组时,会出错!

解决方法

由于这种深浅拷贝问题是因为memcpy
导致的,所以这里不能使用memcpy
只需要老实的使用一个for循环就能解决:
```cpp
Vector(const Vector<T>& v)
{
	assert(v._start && v._finish && v._endofsto);
	_start = new T[v.capacity()];//给size或capacity都可以
	//memcpy(_start, v._start, sizeof(T) * v.size()); //使用memcpy时,数组是二维数组会发生问题
	for (size_t i = 0; i < size(); i++)
	{
		_start[i] = v._start[i];
		_finish = _start + v.size();
	}
	_endofsto = _start + v.capacity();
}
```

## resize()和reserve()区别

void resize(size_type n, value_type val = value_type());

- 如果n<当前容器的size，则将元素减少到前n个，移除多余的元素(并销毁）
- 如果n>当前容器的size，则在容器中追加元素，如果val指定了，则追加的元素为val的拷贝，否则，默认初始化
- 如果n>当前容器容量，内存会自动重新分配


void reserve(size_type n)

- 如果n>容器的当前capacity，该函数会使得容器重新分配内存使capacity达到n
- 任何其他情况，该函数调用不会导致内存重新分配，并且容器的capacity不会改变
- 该函数不会影响向量的size而且不会改变任何元素


1、vector的reserve增加了vector的capacity，但是它的size没有改变！而resize改变了vector的capacity同时也增加了它的size！

2、reserve是容器预留空间，但在空间内不真正创建元素对象，所以在没有添加新的对象之前，不能引用容器内的元素。加入新的元素时，要调用push_back()/insert()函数。

3、resize是改变容器的大小，且在创建对象，因此，调用这个函数之后，就可以引用容器内的对象了，因此当加入新的元素时，用operator[]操作符，或者用迭代器来引用元素对象。此时再调用push_back()函数，是加在这个新的空间后面的。

4、两个函数的参数形式也有区别的，reserve函数之后一个参数，即需要预留的容器的空间；resize函数可以有两个参数，第一个参数是容器新的大小，第二个参数是要加入容器中的新元素，如果这个参数被省略，那么就调用元素对象的默认构造函数。


## vector的.at()和[ ]

最主要的区别就是.at()会额外检查访问是否越界，如果越界，会抛出exception，所以使用.at()时程序运行速度较慢。

二者优点

一般来说用[]效率更高，尤其是需要对索引值进行复杂的计算或者单纯不需要检查是否越界时。
更好的做法是默认用.at()，这样代码尽管很慢但不会产生bug；[]更适合对程序效率要求比较高的场景。

[]缺点

索引值越界时程序不会报错，但会一路莽下去，在向量非空的情况下，即使下标越界，也有可能对应的内存是可读写的，至于读到的是什么内容，或者写到什么地方，就是随机事件了。

由于[]不做边界检查，哪怕越界了也会返回一个引用，当然这个引用是错误的引用，如何不小心调用了这个引用对象的方法，会直接导致应用退出。

处理.at()越界访问

我们用try可以catch out_of_bound exception:

```cpp
try {
	cout << "Out of range element value: "
		 << v.at(container_size + 10) << "/n.";
 } catch(const out_of_range &e) {
	cout << "Ooops, out of range access detected: "
		 << e.what() << "/n."
 }
```

## push_back和emplace_back有什么区别？

vector 的 push_back 和 emplace_back 函数都是用来在 vector 的末尾添加新元素的，但它们之间有几个关键的区别：

构造方式：
push_back 函数会复制或移动已经构造好的对象到 vector 的末尾。
emplace_back 函数则是直接在 vector 的末尾构造新元素，它接受的是构造函数的参数，而不是对象本身。

性能：
使用 push_back 时，如果传入的是一个临时对象，它首先会被构造，然后再被复制或移动到 vector 中（C++11起，会尝试使用移动构造减少开销）。
emplace_back 则可以避免这些额外的复制或移动操作，因为它直接在容器的内存中构造对象，从而可能提供更好的性能。

例子：
使用 push_back 添加一个复杂对象时：myVector.push_back(MyClass(a, b, c)); 这里 a, b, c 是传递给 MyClass 构造函数的参数，首先在外部构造一个临时的 MyClass 对象，然后将其添加到 vector。
使用 emplace_back 相同的操作：myVector.emplace_back(a, b, c); 这里直接将参数 a, b, c 传递给 emplace_back，在 vector 的内存空间中直接构造对象，无需临时对象。

# 迭代器失效

## 一、序列式容器(数组式容器)

对于序列式容器(如vector,deque)，序列式容器就是数组式容器，删除当前的iterator会使后面所有元素的iterator都失效。这是因为vetor,deque使用了连续分配的内存，删除一个元素导致后面所有的元素会向前移动一个位置。所以不能使用erase(iter++)的方式，还好erase方法可以返回下一个有效的iterator。
```cpp
for (iter = cont.begin(); iter != cont.end();)
{
   (*it)->doSomething();
   if (shouldDelete(*iter))
      iter = cont.erase(iter);  //erase删除元素，返回下一个迭代器
   else
      ++iter;
}
```
## 二、关联式容器

对于关联容器(如map, set,multimap,multiset)，删除当前的iterator，仅仅会使当前的iterator失效，只要在erase时，递增当前iterator即可。这是因为map之类的容器，使用了红黑树来实现，插入、删除一个结点不会对其他结点造成影响。erase迭代器只是被删元素的迭代器失效，但是返回值为void，所以要采用erase(iter++)的方式删除迭代器。

## 链表式容器

对于链表式容器(如list)，删除当前的iterator，仅仅会使当前的iterator失效，这是因为list之类的容器，使用了链表来实现，插入、删除一个结点不会对其他结点造成影响。只要在erase时，递增当前iterator即可，并且erase方法可以返回下一个有效的iterator。

方式一:递增当前iterator

同 map `erase(iter++)`

方式二:通过erase获得下一个有效的iterator

同 vector： `iter = erase(iter)`


## 四、总结

迭代器失效分三种情况考虑，也是分三种数据结构考虑，分别为数组型，链表型，树型数据结构。

数组型数据结构：该数据结构的元素是分配在连续的内存中，insert和erase操作，都会使得删除点和插入点之后的元素挪位置，所以，插入点和删除掉之后的迭代器全部失效，也就是说insert(*iter)(或erase(*iter))，然后在iter++，是没有意义的。解决方法：erase(*iter)的返回值是下一个有效迭代器的值。 iter =cont.erase(iter);

链表型数据结构：对于list型的数据结构，使用了不连续分配的内存，删除运算使指向删除位置的迭代器失效，但是不会失效其他迭代器.解决办法两种，erase(*iter)会返回下一个有效迭代器的值，或者erase(iter++).

树形数据结构： 使用红黑树来存储数据，插入不会使得任何迭代器失效；删除运算使指向删除位置的迭代器失效，但是不会失效其他迭代器.erase迭代器只是被删元素的迭代器失效，但是返回值为void，所以要采用erase(iter++)的方式删除迭代器。

注意：经过erase(iter)之后的迭代器完全失效，该迭代器iter不能参与任何运算，包括iter++,*ite
