#include <algorithm>
#include <array>
#include <gtest/gtest.h>
#include <glog/logging.h>
#include <initializer_list>
#include <sys/signal.h>
#include <type_traits>
#include <utility>
#include <vector>

class BigMemoryPool {
public:
    static const size_t mem_size = 4096;
    BigMemoryPool():pool(new char[mem_size]) {}
    ~BigMemoryPool() {
        if (pool != nullptr) delete [] pool;
    }

    BigMemoryPool(const BigMemoryPool & other) : pool(new char[mem_size]) {
        if (this == &other) return;
        std::cout << "copy\n";
        memcpy(pool, other.pool, mem_size);
    }

    // 要避免多余的拷贝，移动构造必须要在类中实现
    BigMemoryPool(BigMemoryPool && other){
        std::cout << "move\n";
        pool = other.pool;
        other.pool = nullptr;
    }

private:
    char *pool;
};

BigMemoryPool getBigMemoryPool() {
    BigMemoryPool pool;
    return pool;
}

int gx = 10;
int getx() {
    return gx;
}


TEST(chapter1, base_const) {
    // 顶层const接近变量名称，底层const被指针或者引用隔开隔开
    // 在变量赋值时，顶层const没有什么影响，但如果有底层const必须保持一致
    const int a = 10;
    int b = a;  // 赋值，const属性没有赋予给b

    const int * const p = new int(10);
    // int *p1 = p;
    // int *const p2 = p; 这两种都会出错 ， 因为这里如果给你赋值成功，由于指针可以改动指定地址的值，所以这一相当于p指针指向的内容可以被改动了
    const int *p3 = p;

    // int *p4 = &a; 也不行，相当于强制去const了，可以根据p4指针修改a的值了
    const int *p4 = &a; // ok!

    // int &r1 = a; 同理不行
    const int &r1 = a; // ok

    const int &r2 = 20;
    // int &r2 = r1; 错误
    const int &r3 = r2; // ok
}

TEST(chapter1, base_right) {

    // 如果关闭返回值优化和自定义的移动拷贝构造函数，这里会出现两次copy，否则是两次move
    auto pool = getBigMemoryPool();

    // &getx(); 即使返回的是全局变量gx，但是函数返回的是临时值
    int x = 10;
    // int *p1 = &x++; // 这种会出错，因为内部的具体实现是先将x暂存（参数是引用）到一个temp，再自增，最后返回temp，temp是一个临时值
    int *p2 = &++x; // 这种正常因为返回的就是引用

    auto p = &"hello world"; // 字符串常量是左值

    /*
        C++中的表达式类型氛围 泛左值 和 右值
        左值属于泛左值
        纯右值属于右值
        将亡值同时属于泛左值和右值

        左值引用不能接受一个右值
        右值引用则可以接受右值
        **右值引用仍是左值**
    */

    BigMemoryPool aaa;
    BigMemoryPool &bbb = aaa; // 引用操作，相当于 共享 大脑
    BigMemoryPool ccc = aaa; // 拷贝操作，相当于 复制 一份大脑给另一个人
    BigMemoryPool ddd = getBigMemoryPool(); // 触发两次移动构造 相当于 移植 一份大脑给另一个人
    BigMemoryPool eee = std::move(aaa); // 此是aaa变成了将亡值
}

bool fun1(int a, int b) {
    return true;
}


TEST(chapter1, base_ptr) {
    // 数组指针和函数指针
    int array[5] = {0, 1, 2, 3, 4}; // array是什么类型？ array的数据类似是int [5]
    int *a_ptr = array; // 数组名退化为指针 int [5] -> int * ，数组名作为参数传递会发生退化
    int (*ptr3)[5] = &array; // ptr3是一个数组指针 int (*) [5] ，指向一个长度为5的int数组，即指向int [5]
    int (&ref)[5] = array; // ref是数组的引用 int (&) [5] 

    char str[12] = "hello world";
    const char *strr = "hello world"; // 字符串常量不可修改，不能去掉const

    bool (*func1)(int a, int b);
    bool (&func2)(int a, int b) = fun1; // 函数引用
    func1 = fun1; // 这里省略了&取值符号，发生了退化函数名退化为函数指针
    bool res = func1(1, 2); // 这里没有用 （*func1）也能成功

    // 函数别名
    typedef bool Func(int, int); // 这个表示func是一个函数类型，不是指针
    typedef bool (*Func1)(int);
    using Func2 = bool(int, int); // 这个只是表示func2是一个函数类型，不是指针
    using Func3 = bool (*) (int, int); 
}

int func(int) {return 0;};
bool func1(int, int) {return 0;};
using F = int(int);

template<class T>
void tcf(T param) {
    std::cout << param << std::endl;
}

TEST(chapter1, item1) {
    // 类型推导
    /*
        template<class T>
        void fun(const T param);
        void fun(T const param); 这两种都是顶层const（ 接近变量）

        顶层const不能构成重载
        fun(int a)；与fun(const int a)；不能出现在同一个命名空间

        指针也存在引用
    */
    // 函数指针的底层const似乎只能用别名表示
    int (*funcptr1)(int) = func; // ok
    int (* const funcptr2)(int) = func; // 顶层const ok
    // const int (* funcptr3)(int) = func; // 非别名无法用底层const
    const F *f = func; // 别名可用底层const但没什么用
    

    /*
        模板类型 T 可以情况讨论：
        类别1： T, T*, T&, const T(❎), const T*, const T&, 
        类别2： T* const(❎), const T* const(❎)
                将上面T全换成int， 字面量
                int[10], bool(int, int)
                int(*)[10], bool(*)(int, int)
                int(&)[10], bool(&)(int, int)
        类别3： T&&, const T&&

        类别1：理解顶层与底层const， 缺什么补什么
        类别2：理解数组和函数能够退化成指针
        类别3 T &&万能引用，传入左值是左值引用，传入右值是右值引用
             const T&&只接受右值
    */

    int a = 10;
    int *aptr = &a;
    int &aref = a;
    int &&arref = std::move(a); 

    const int ca = a;
    const int *captr = &ca;
    const int &caref = ca; 
    const int &&carref = std::move(ca);
    int *const acptr = &a;
    const int *const cacptr = &ca;
    int array[2] = {0, 1};
    int(*arrayptr)[2]  = &array;
    int(&arrayref)[2] = array;
    using F = bool(int, int);
    using F1 = bool(*)(int, int);
    using F2 = bool(&)(int, int);

    F1 fff = func1;

    F2 ffff = func1;
}

class A {
public:
    // A(int a) {std::cout << "A(int a)" << std::endl;}
    A(int a, int b) {std::cout << "A(int a, int b)" << std::endl;}
    A(const A &a) {std::cout << "A(const A &a)" << std::endl;}
    A(std::initializer_list<int> p) {std::cout << "initial" << std::endl;} // 列表可以接受无限个同类型参数
};

// 聚合类
struct peo {
    int age;
    double money;
};

void funcA(A a) {
}

A funcA1() {
    return A(10, 10);
}

A funcA2() {
    return {10, 10};
}

void funcpp(double p) {
    // int i(int(p)); // 编译器有两种解析方式，它可能解析为函数声明
    int i{int(p)}; // 不会解析错误
}

struct S {
    int a, b;
};

TEST(chapter1, item2) {
    // 区别使用（）和{}创建对象
    /*
        也需要关闭自动优化返回值
        A a(..); 问题:当作为临时变量参数或返回值的时候会进行拷贝
        A a{...} 的优势就可以避免上面的问题，且不允许缩窄转换
                      大大减少聚合类的初始化，聚合类：所有成员都是public，
                                                没有定义任何构造函数
                                                没有基类和虚函数
                                          c++17允许有构造函数但必须是公有继承且非虚继承
                      对解析问题天生免疫， 类内初始化不能用 int a(10);他会解析成函数
    */  
    // A a(10, 20); 
    // A aaa = {10, 20};
    // A aaaa {10, 20};

    // funcA(A(10, 20)); // 先构造再拷贝，效率低
    // funcA({10, 20}); // 列表不会存在拷贝
    // auto p = funcA1(); // 一次构造两次拷贝
    // auto p = funcA2(); // 一次构造一次拷贝

    // A a{1, 1.2}; // 不允许缩窄类型转换 float -> int

    /*
        奇怪的array
    */
    std::array<int, 3> a1{1, 2, 3};
    S s1[3] = {{1, 2}, {2, 3}, {3, 4}}; // 聚合类数组
    S s2[3]{1, 2, 2, 3, 3, 4}; // ok

    // std::array<S, 3> s3{{1, 2}, {2, 3}, {3, 4}}; 编译出错，因为array本身就是聚合类，里面有个数组，需要多包一层
    std::array<S, 3> s3{{{1, 2}, {2, 3}, {3, 4}}}; //ok
    
    /*
        如何让容器支持列表初始化
        std::initializer_list<> 以类A为例
        注意：不允许缩窄函数
             列表初始化的优先级问题：除非万不得已(参数不同类型且怎么都无法转换)，否则优先匹配列表初始化构造函数
             A (int ,int ) 和 A(std::initializer_list<int>)无论怎么样都会优先
             匹配后者，这个很坑，可以通过()调用来避免
             空的{}不会调用列表初始化，而是无参构造

    */
    A aaaaa{1, 2};
}

template<class T> 
void f(T a) {

}

template<class T> 
void g(std::initializer_list<T> a){
}

auto create() {
    // return {1, 2, 3}; // 模板T无法推导列表初始化结构，报错
    return 1;
}

TEST(chapter1, yauto) {
    /*
        理解auto类型推导：
        1、万能引用的第二种写法 auto &&
        2、{}的auto类型推导
        3、{}的模板类型推导
        4、c++14之后auto可以作为函数的返回值，规则却是模板的规则
    */
    auto x = 1;
    auto x1(1);
    auto x2{1};
    auto x3 = {1}; // 推导成std::init...

    // f({1, 2, 3}); // 报错， T无法推出列表初始化
    g({1, 2, 3}); // ok
}

struct uutest1 {static int testtype;};
struct uutest2 {typedef int testtype;};

template<class T>
using myvc1 = std::vector<T>;

template<class T>
// typedef定义模板的别名
// typedef myvc2 std::vector<T>; // 错误
struct myvc2{
    typedef std::vector<T> type ;
};

template<class T>
class weight1 {
public:
    myvc1<T> p; // 若用using不需要加上typename ！！！
    typename myvc2<T>::type p2; // 若用typedef必须要加上typename
};

template<class T>
class myclass {
public:
    void foo() {
        typename T::testtype *ptr; // 消除T::testtype为类型而不是uutest1中的变量
    }
};

TEST(chapter1, uusing) {
    /*
        优先考虑别名声明而非typedef
        using > typedef
        1、typename在模板类中用来澄清下一个标识符代表的是某类型，比较明显的就是作用域符号
            c++中T::xxx默认认为是作用域访问符号
        2、typedef定义模板的别名很麻烦必须要声明类
        3、using再模板类中不需要typename来澄清
        4、类型萃取器，用于添加/删除模板T的修饰
            typename std::remove_const<T>::type c++11  const T -> T // 这个需要typename
            std::remove_const_t<T>   c++14 const T -> T // C++14用using实现了，不需要typename
    */
    myclass<uutest1> mc1;
    //mc1.foo(); // 报错
    myclass<uutest2> mc2;
    mc2.foo();

    weight1<int> vec{{1, 2, 3}, {}};
}

template<class T> 
T &&mymove1(T &&param) {
    return static_cast<T &&>(param);
}

template<class T> 
std::remove_reference_t<T> && mymove2(T &&param) {
    // 做类型萃取，因为万能引用弄进来的是引用类型，不去掉后面会产生折叠
    using returntype = std::remove_reference_t<T> &&;
    return static_cast<returntype>(param);
}

void process(const int& a) {
    std::cout << "process la\n";
}

void process(int &&a) {
    std::cout << "process ra\n";
}

template<class T>
void logAndprocess(T &&param) {
    process(std::forward<T>(param)); // param本身永远是左值， 用forward能保留原变量的类型（左值还是右值）
}

TEST(chapter1, udforward) {
    /*
        理解std::move和std::forward
        1、std::move的实现（必须模板）
            可以接入任意类型所以参数是T &&万能引用
            本质是类型转换 static_cast<type &&>(...);
        2、std::move本质是将实参转化为右值
            就是告诉编译器这个对象很适合移动，但是你可以不移动，移动的操作是你手动做的事
        3、对一个const类型的对象做move结果为const T &&， const &可以匹配任何类型，此时可能
            仍然调用拷贝构造函数，如果输入值的类型是const T
    */
    int a = 10;
    // int &&p = mymove1(a); // 出大问题，因为全部引用折叠成左值引用了，返回的也是左值引用
    int &&p = mymove2(a);
    mymove1(10); // ok 

    /*
        初探std::forward 本质：有条件的move，只有实参用右值初始化才能转为右值 
        模板类型参数本身就是左值，如果没有forward，传进来的变量的左值和右值信息已经丢失
    */
    logAndprocess(std::move(a));
}

TEST(chapter1, uddecltype) { 
    /*
        decltype + 变量 所有的信息都会被保留，数组和函数也不会退化
        decltype + 表达式 会返回表达式结果对应的类型
            不是左值就是右值，左值：得到该类型的左值引用，右值：得到该类型
        想将变量作为表达式可以加括号（括号表达式包含一个变量产生的结果总是一个左值）
    */

    const int &&a = 10;
    decltype(a) b = 10;

    int c = 10;
    int *cptr = &c;
    decltype( *cptr) por = c; //因为*cptr是一个表达式，所以por推导出来的类型是int &
}