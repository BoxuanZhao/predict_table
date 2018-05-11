# predict_table

1. 代码中包含中文注释，在Microsoft Visual Studio 2017环境下编译通过，请在相同环境下测试或使用支持UTF-16编码的编辑器。同时提供UTF-8编码的代码版本以便参考。
2. 在此项目中，~为空字epsilon，$为输入右端结束标记。
3. 从in.txt中读入不含有直接左递归的上下文无关文法。为简化字符串处理：
   1）第一条产生式的左部为开始符号；
   2）将右部带有|的产生式拆分成若干个不含|的产生式；
   3）文法符号(字符串)之间用空格分隔，以便分词；
   4）产生式右部空字~只会独立出现一次（即只存在A->~，不存在A->~B~b~）
4. 输出结果显示在out.txt中。
