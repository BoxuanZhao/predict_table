/*
1. 代码中包含中文注释，在Microsoft Visual Studio 2017环境下编译通过，请在相同环境下测试或使用支持UTF-16编码的编辑器。
2. 在此项目中，~为空字epsilon，$为输入右端结束标记。
3. 从in.txt中读入不含有直接左递归的上下文无关文法。为简化字符串处理：
1）第一条产生式的左部为开始符号；
2）将右部带有|的产生式拆分成若干个不含|的产生式；
3）文法符号(字符串)之间用空格分隔，以便分词；
4）产生式右部空字~只会独立出现一次（即只存在A->~，不存在A->~~~B~~b~~~）
4. 输出结果显示在out.txt中。
*/

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <string>
#include <map>
#include <set>

using namespace std;

const int MAX = 500; //最大产生式数

ofstream fout("out.txt");
ifstream fin("in.txt");

//产生式类
class production
{
public:
	string left; //产生式左部
	vector<string> right; //产生式右部(->右侧部分)
	production(char s[])
	{
		left = s;
	}

	void insert(char s[])
	{	
		int j = 0;
		right.push_back("");
		for (unsigned int i = 0; i < strlen(s); i++)
		{
			if (s[i] != ' ') //读入用空格分隔的文法符号
				right[j] += s[i];
			else
			{
				right.push_back("");
				j++;
			}			
		}
	}
}; 

set<string> nonterminal; //非终结符号集合
map<string, set<string> > first;	//FIRST集合
map<string, set<string> > follow;   //FOLLOW集合
map<string, map<string, string> > predict_table; //预测分析表
vector<production> productions;	//产生式集合
bool traversed[MAX];	//遍历标记

//深度优先得到FIRST集合
void get_first(int x)
{
	if (traversed[x]) return;
	traversed[x] = true;
	string& left = productions[x].left;
	vector<string>& right = productions[x].right;
	for (vector<string>::iterator it = right.begin(); it != right.end(); it++)
	{	
		if (!nonterminal.count(*it) || *it == "~") //文法符号是终结符号
		{
			first[left].insert(*it);
			break;
		}
		if (nonterminal.count(*it)) //文法符号是非终结符号
		{
			bool flag = true;
			for (unsigned int i = 0; i < productions.size(); i++)
			{
				if (productions[i].left == *it)
				{
					get_first(i);
					set<string>& temp = first[productions[i].left];			
					for (set<string>::iterator it1 = temp.begin(); it1 != temp.end(); it1++)
					{
						if (*it1 == "~") flag = false; //前一个文法符号的FIRST集合存在空字epsilon，继续考察下一个
						first[left].insert(*it1);
					}
				}
			}	
			if (flag) break; //前一个文法符号的FIRST集合不存在空字epsilon，迭代结束
		}
		else continue;
	}
}

//构造FIRST集合
void make_first()
{
	memset(traversed, 0, sizeof(traversed)); //将DFS标记清零，初始化值false
	for (unsigned int i = 0; i < productions.size(); i++)
		get_first(i);
	//将FIRST集合以文本形式输出
	fout << "FIRST集合:" << endl;
	for (map<string, set<string> >::iterator it = first.begin(); it != first.end(); it++)
	{
		fout << "FIRST(" << it->first << ")={";
		set<string> & temp = it->second;
		bool flag = false;
		for (set<string>::iterator it1 = temp.begin(); it1 != temp.end(); it1++)
		{
			if (flag) fout << ",";
			fout << *it1;
			flag = true;
		}
		fout << "}" << endl;
	}
}

//构造FOLLOW集合
void make_follow()
{
	follow[productions[0].left].insert("$"); //将结束符$放入开始符号的FOLLOW集合中
	while (true)
	{
		bool new_follow = false;
		for (unsigned int i = 0; i < productions.size(); i++)
		{
			set<string> temp; //temp中存储前一个产生式中得到的应加入到这一个产生式对应FOLLOW集合的元素，
							  //即对于产生式A->aBCD，当it指向C时，假设FIRST(D)不包含空字，则temp存储FIRST(D)。
			string& left = productions[i].left;
			vector<string>& right = productions[i].right;
			if (*right.begin() == "~") //产生式右部为空字
				continue;
			for (vector<string>::reverse_iterator it = right.rbegin(); it != right.rend(); it++)
			{
				if (!nonterminal.count(*it)) //文法符号是终结符号
				{
					temp.insert(*it);
				}
				else if (nonterminal.count(*it)) //文法符号是非终结符号
				{
					if (it == right.rbegin()) 
					{
						//对于产生式A->aB，将FOLLOW(A)中的所有符号加入到FOLLOW(B）
						int old_size = follow[*it].size();
						for (set<string>::iterator it1 = follow[left].begin(); it1 != follow[left].end(); it1++)
						{
							follow[*it].insert(*it1);
						}
						int new_size = follow[*it].size();
						if (new_size > old_size)
							new_follow = true;

						temp.clear();
						
						for (set<string>::iterator it1 = first[*it].begin(); it1 != first[*it].end(); it1++)
						{
							if (*it1 != "~")
								temp.insert(*it1);
						}
						if (first[*it].count("~"))
							for (set<string>::iterator it1 = follow[left].begin(); it1 != follow[left].end(); it1++)
								temp.insert(*it1);
					}
					else 
					{
						//对于产生式A->aBb，将FIRST(b)中除~之外的所有符号，以及FOLLOW(A)中的所有符号加入到FOLLOW(B）
						int old_size = follow[*it].size();
						for (set<string>::iterator it1 = temp.begin(); it1 != temp.end(); it1++)
						{			
							follow[*it].insert(*it1);	
						}
						int new_size = follow[*it].size();
						if (new_size > old_size)
							new_follow = true;

						temp.clear();
						
						for (set<string>::iterator it1 = first[*it].begin(); it1 != first[*it].end(); it1++)
						{
							if (*it1 != "~")
								temp.insert(*it1);
						}
						if (first[*it].count("~"))
							for (set<string>::iterator it1 = follow[left].begin(); it1 != follow[left].end(); it1++)
								temp.insert(*it1);
					}				
				}
			}
		}
		if (!new_follow) break; //直到没有任何新终结符号加入FOLLOW集合，循环结束
	}
	//将FOLLOW集合以文本形式输出
	fout << "FOLLOW集合:" << endl;
	for (map<string, set<string> >::iterator it = follow.begin(); it != follow.end(); it++)
	{
		fout << "FOLLOW(" << it->first<< ")={";
		set<string> & temp = it->second;
		bool flag = false;
		for (set<string>::iterator it1 = temp.begin(); it1 != temp.end(); it1++)
		{
			if (flag) fout << ",";
			fout << *it1;
			flag = true;
		}
		fout << "}" << endl;
	}
}

//构造预测分析表
void make_table()
{
	set<string> terminal;	//终结符号集合，用来存储分析表的列
	for (unsigned int i = 0; i < productions.size(); i++)
	{
		vector<string>& right = productions[i].right;	
		for (vector<string>::iterator it = right.begin(); it != right.end(); it++)
			if (!nonterminal.count(*it) && *it != "~")
				terminal.insert(*it);
	}
	for (set<string>::iterator it = terminal.begin(); it != terminal.end(); it++)
	{
		first[*it].insert(*it);
	}
	terminal.insert("$");
	for (unsigned int i = 0; i < productions.size(); i++)
	{
		string& left = productions[i].left;
		vector<string>& right = productions[i].right;
		map<string, string> &temp = predict_table[left]; //终结符号map to产生式，分析表中存储结点的临时数据结构
		for (set<string>::iterator it = first[*right.begin()].begin(); it != first[*right.begin()].end(); it++)
		{
			temp[*it] = "";
			for (vector<string>::iterator it1 = right.begin(); it1 != right.end(); it1++)
				temp[*it] += *it1;
		}
		if (first[*right.begin()].count("~") || *right.begin() == "~")
		{
			if (follow[left].count("$"))
			{
				temp["$"] = "";
				for (vector<string>::iterator it = right.begin(); it != right.end(); it++)
					temp["$"] += *it;
			}
			for (set<string>::iterator it = follow[left].begin(); it != follow[left].end(); it++)
			{
				temp[*it] = "";
				for (vector<string>::iterator it1 = right.begin(); it1 != right.end(); it1++)
					temp[*it] += *it1;
			}				
		}
	}
	
	//将预测分析表以文本形式输出
	fout << "预测分析表：" << endl;
	for (unsigned int i = 0; i <= (terminal.size() + 1) * 10; i++)
		fout << "-";
	fout << endl;
	fout << setw(10) << "|";
	for (set<string>::iterator it = terminal.begin(); it != terminal.end(); it++)
		fout << setw(5) << *it << setw(5) << "|";
	fout << endl;
	for (unsigned int i = 0; i <= (terminal.size() + 1) * 10; i++)
		fout << "-";
	fout << endl;
	for (set<string>::iterator it = nonterminal.begin(); it != nonterminal.end(); it++)
	{
		fout << "|" << setw(5) << *it << setw(4) << "|";
		for (set<string>::iterator it1 = terminal.begin(); it1 != terminal.end(); it1++)
			if (predict_table[*it].count(*it1))
				fout << setw(3) << *it << "->" << predict_table[*it][*it1] << setw(3) << "|";
			else fout << setw(10) << "|"; //空条目
		fout << endl;
		for (unsigned int i = 0; i <= (terminal.size() + 1) * 10; i++)
			fout << "-";
		fout << endl;
	}
}

int main()
{
	char s[MAX] = {0};
	cout << "读入的产生式为：" << endl;
	while (fin.getline(s, MAX))
	{
		cout << s << endl;
		int len = strlen(s), j;
		for (j = 0; j < len; j++)
			if (s[j] == '-') break;
		s[j] = 0;
		nonterminal.insert(s); //将产生式左部加入非终结符号集合
		productions.push_back(s); //将产生式左部加入到productions，调用class production的构造函数
		char *right = s + j + 2; //其中2 = strlen(->)
		productions[productions.size() - 1].insert(right); //将产生式右部(->右侧部分)加入到productions
	}
	//eliminate_left_recursion(); 暂时未实现
	make_first();
	make_follow();
	make_table();
	cout << "已生成预测分析表！" << endl;
	return 0;
}
