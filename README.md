# 南南正则表达式库
一个清爽的正则表达式库，完全参照《编译器设计》第二版中的算法实现。
从正则表达式的到**非确定性**有限状态机的_thompson_算法，再通过_子集构造算法_从**NFA**(非确定性有限状态机)到**DFA**(确定性有限状态机)。最后使用_hopcroft_最小化*DFA*。

## 使用例子
```c++
nanan::nan_regular reg("ab(a|b)[1234][a-z][^A-Z]*");
//nanan::nan_regular reg("a(b|c)*");
std::string test = "aba3a123ca12312321aba3ac^321312352aba3aca";
reg.match_strict(test) == true ? printf("true\n") : printf("false\n");
std::vector<std::pair<size_t, size_t> > res = reg.match_long(test);
for (auto i : res) {
  printf("%s\n", test.substr(i.first, i.second).c_str());
}
```

## 简单的使用说明

|名称|作用|
|---|---|
|match_strict|进行严格匹配也就是要**匹配的字符串**当消耗完毕字符串时，正则表达式也处于**接收状态**，返回一个布尔值|
|match_short|进行最短匹配，返回一个数组，其中纪录了偏移与长度|
|match_long|进行最长匹配，返回一个数组，其中纪录了偏移与长度|

**最短**与**最长**的意思是当正则表达式匹配到**接受状态**时是重写匹配，还是继续匹配直到出现不匹配。

## 目前功能
只有最核心的：“闭包，连接，选择” 三种操作。
