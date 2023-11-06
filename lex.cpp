#include<iostream>
#include<string>
#include<stack>
#include<vector>
#include<algorithm>
using namespace std;

typedef struct Treenode {
	char character;
	Treenode*leftchild, *rightchild;
}Treenode, *Tree;
typedef struct NFAnode
{
	char inputStatus1;
	char inputStatus2;
	NFAnode*Status1;
	NFAnode*Status2;
}NFAnode;
typedef struct TransformState
{
	NFAnode* LastStatus;
	char InputCharacter;
	NFAnode* NextStatus;
}TransformState;

typedef struct TransformStateDFA
{
	int LastStatus;
	char InputCharacter;
	int NextStatus;
}TransformStateDFA;//用数组下标唯一标记一个DFA的状态切换
typedef struct StatusSetLine
{
	vector<NFAnode*>Status;
	bool flag;
	bool isFinal;
}StatusSetLine;
typedef struct UpdateStatus
{
	vector<int>nextstatus;//存放输入字符串后的变化状态
	vector<int>nowstatusindex;//存放当前对应状态的下标
}UpdateStatus;

vector<TransformState>TransformTable;//用于存储所有状态转化函数
vector<TransformStateDFA>TransformTableDFA;//用于存储所有状态转化函数
vector<StatusSetLine>StatusSets;//用与存放状态集
vector<NFAnode*> StatusLine;//存放当前扫描得到的未标记状态
vector<vector<int>>DFAState;//所有状态
int StatusLineIndex = 0;//标记状态集里现在有几行

char TEMP;//标记当前扫描的字符
vector<char>Operands;
stack<char>OperatorStack;
stack<Tree>OperandsStack;
stack<pair<NFAnode*, NFAnode*>>StatusStack;//NFA栈
char OperatorStackTop;
Treenode *OperandsStackTop;
int isoperator(char temp)//判断加入的是否为运算符
{
	if (temp == '(' || temp == ')' || temp == '|' || temp == '*' || temp == '&')
	{
		return 1;
	}
	else {
		return 0;
	}
}
//判断运算符优先级
int getOperatorPrecedence(char temp)
{
	switch (temp)
	{
	case '(':
		return 0;
	case '|':
		return 1;
	case '&':
		return 2;
	case '*':
		return 3;
	}
}
//运算符的压栈流程运算
void caculate_operator(char TEMP)
{
	if (TEMP == '(')
	{
		OperatorStack.push(TEMP);//右括号直接压栈
	}
	else if (TEMP == ')')
	{
		while ((OperatorStackTop = OperatorStack.top()) != '(')
		{
			Treenode* OperatorNode = new Treenode;
			if (OperatorStackTop == '*')//孩子只有一个，只需要一个运算符
			{
				OperatorNode->character = '*';
				OperatorNode->leftchild = OperandsStack.top();
				OperatorNode->rightchild = NULL;
				OperandsStack.pop();
				OperandsStack.push(OperatorNode);
			}
			else//需要两个运算符
			{
				OperatorNode->character = OperatorStackTop;
				OperatorNode->rightchild = OperandsStack.top();
				OperandsStack.pop();
				OperatorNode->leftchild = OperandsStack.top();
				OperandsStack.pop();
				OperandsStack.push(OperatorNode);
			}
			OperatorStack.pop();//出栈
		}
		OperatorStack.pop();//出栈右括号
	}
	else //如果是运算符*&|
	{
		while ((!OperatorStack.empty()) && (getOperatorPrecedence(OperatorStack.top()) >= getOperatorPrecedence(TEMP)))//栈不为空且栈顶符号的优先级如果高需要先让栈顶出栈
		{
			OperatorStackTop = OperatorStack.top();
			Treenode* OperatorNode = new Treenode;
			if (OperatorStackTop == '*')//孩子只有一个，只需要一个运算符
			{
				OperatorNode->character = '*';
				OperatorNode->leftchild = OperandsStack.top();
				OperatorNode->rightchild = NULL;
				OperandsStack.pop();
				OperandsStack.push(OperatorNode);
			}
			else//需要两个运算符
			{
				OperatorNode->character = OperatorStackTop;
				OperatorNode->rightchild = OperandsStack.top();//第一个出来的为右孩子
				OperandsStack.pop();
				OperatorNode->leftchild = OperandsStack.top();
				OperandsStack.pop();
				OperandsStack.push(OperatorNode);
			}
			OperatorStack.pop();//出栈

		}
		OperatorStack.push(TEMP);
	}
}
//后序遍历实现检查
void postorderTraversal(Tree root)
{
	if (root)
	{
		postorderTraversal(root->leftchild);
		postorderTraversal(root->rightchild);
		cout << root->character << " ";
	}
}
void printinput(NFAnode*StartStatus)
{
	while ((StartStatus->Status1 != NULL) && (StartStatus->Status2 != NULL))
	{
		cout << StartStatus->inputStatus1 << endl;
		StartStatus = StartStatus->Status1;
	}
}
void BuildNFA(Tree root)
{
	if (root)
	{
		BuildNFA(root->leftchild);
		BuildNFA(root->rightchild);
		//都会拥有一个新的开始状态和结束状态
		NFAnode*StartStatus = new NFAnode;
		NFAnode*EndStatus = new NFAnode;
		EndStatus->inputStatus1 = '$';
		EndStatus->Status1 = NULL;
		EndStatus->inputStatus2 = '$';//这里只要占位即可，只要是看这个状态存不存在，存在才会看输入！！！
		EndStatus->Status2 = NULL;
		if (!isoperator(root->character))//如果不是运算符，状态加到1上
		{
			StartStatus->inputStatus1 = root->character;
			StartStatus->Status1 = EndStatus;
			StartStatus->inputStatus2 = '$';//这里只要占位即可，只要是看这个状态存不存在，存在才会看输入！！！
			StartStatus->Status2 = NULL;
			//更新我的列表
			TransformTable.push_back({ StartStatus,root->character,EndStatus });
		}
		else//是运算符|&*，其中&使用合并类型
		{
			if (root->character == '*')
			{
				pair<NFAnode*, NFAnode*> LastStatusPair = StatusStack.top();//取出栈顶
				NFAnode*LastStartStatus = LastStatusPair.first;
				NFAnode*LastEndStatus = LastStatusPair.second;
				LastEndStatus->inputStatus1 = '@';//状态1继续往后走
				LastEndStatus->Status1 = EndStatus;
				LastEndStatus->inputStatus2 = '@';
				LastEndStatus->Status2 = LastStartStatus;//状态2循环继续
				StartStatus->inputStatus1 = '@';
				StartStatus->Status1 = LastStartStatus;//状态1正常走
				StartStatus->inputStatus2 = '@';
				StartStatus->Status2 = EndStatus;//状态2直接到结束
				StatusStack.pop();//栈顶元素压出栈
				//更新我的列表
				TransformTable.push_back({ StartStatus,'@',LastStartStatus });
				TransformTable.push_back({ StartStatus,'@',EndStatus });
				TransformTable.push_back({ LastEndStatus,'@',EndStatus });
				TransformTable.push_back({ LastEndStatus,'@',LastStartStatus });
			}
			else if (root->character == '&')//状态之间都用状态1连接
			{//需要两个状态连接其中更靠上的是连接在后面的
				pair<NFAnode*, NFAnode*> LastAfterStatusPair = StatusStack.top();//取出栈顶
				NFAnode*LastAfterStartStatus = LastAfterStatusPair.first;
				NFAnode*LastAfterEndStatus = LastAfterStatusPair.second;
				StatusStack.pop();//取出连接在后面的状态
				pair<NFAnode*, NFAnode*> LastBeforeStatusPair = StatusStack.top();//取出栈顶
				NFAnode*LastBeforeStartStatus = LastBeforeStatusPair.first;
				NFAnode*LastBeforeEndStatus = LastBeforeStatusPair.second;
				StatusStack.pop();//取出连接在前面的状态
				StartStatus->inputStatus1 = '@';
				StartStatus->Status1 = LastBeforeStartStatus;
				StartStatus->inputStatus2 = '$';
				StartStatus->Status2 = NULL;
				LastBeforeEndStatus->inputStatus1 = '@';
				LastBeforeEndStatus->Status1 = LastAfterStartStatus;
				LastAfterEndStatus->inputStatus1 = '@';
				LastAfterEndStatus->Status1 = EndStatus;
				//更新我的列表
				TransformTable.push_back({ StartStatus,'@',LastBeforeStartStatus });
				TransformTable.push_back({ LastBeforeEndStatus ,'@',LastAfterStartStatus });
				TransformTable.push_back({ LastAfterEndStatus ,'@',EndStatus });
			}
			else//运算符|
			{
				pair<NFAnode*, NFAnode*> LastStatusPair1 = StatusStack.top();//取出栈顶
				NFAnode*LastStartStatus1 = LastStatusPair1.first;
				NFAnode*LastEndStatus1 = LastStatusPair1.second;
				StatusStack.pop();//取出连接在后面的状态
				pair<NFAnode*, NFAnode*> LastStatusPair2 = StatusStack.top();//取出栈顶
				NFAnode*LastStartStatus2 = LastStatusPair2.first;
				NFAnode*LastEndStatus2 = LastStatusPair2.second;
				StatusStack.pop();//取出连接在前面的状态
				StartStatus->inputStatus1 = '@';
				StartStatus->Status1 = LastStartStatus1;
				StartStatus->inputStatus2 = '@';
				StartStatus->Status2 = LastStartStatus2;
				//原来的终态增加一个状态，都在状态1上加入
				LastEndStatus1->inputStatus1 = '@';
				LastEndStatus1->Status1 = EndStatus;
				LastEndStatus2->inputStatus1 = '@';
				LastEndStatus2->Status1 = EndStatus;
				//更新我的列表
				TransformTable.push_back({ StartStatus,'@',LastStartStatus1 });
				TransformTable.push_back({ StartStatus,'@',LastStartStatus2 });
				TransformTable.push_back({ LastEndStatus1,'@',EndStatus });
				TransformTable.push_back({ LastEndStatus2,'@',EndStatus });
			}
		}
		//状态更新完毕新状态入栈
		pair<NFAnode*, NFAnode*>StartEndPair(StartStatus, EndStatus);
		StatusStack.push(StartEndPair);
	}
}
vector<NFAnode*> CalculateClosure(vector<NFAnode*>node)//计算一个节点的闭包状态，这里默认节点必然是有的
{
	stack<NFAnode*>tempNodeStack;//存放临时状态
	NFAnode* tempNode;
	for (int i = 0; i < node.size(); i++)
	{
		tempNodeStack.push(node[i]);//node入栈
	}

	while (!tempNodeStack.empty())//只要栈还不是空的
	{
		tempNode = tempNodeStack.top();//取
		tempNodeStack.pop();
		for (const TransformState& row : TransformTable) {
			if ((row.LastStatus == tempNode) && (row.InputCharacter == '@'))
			{
				//找到一个状态,判断是不是已经在数组里了
				auto it = find(node.begin(), node.end(), row.NextStatus);
				if (it == node.end())//没有这个状态
				{
					node.push_back(row.NextStatus);//加入到数组当中
					tempNodeStack.push(row.NextStatus);//压入栈中
				}
			}
		}

	}
	sort(node.begin(), node.end());//将地址按顺序排列方便判断是不是同一个状态集合
	for (int i = 0; i < node.size(); i++)
	{
		cout << node[i] << " ";
	}
	cout << "状态对应的闭包的大小:" << node.size() << endl;
	return node;
}
bool ifExistNotmarked(vector<StatusSetLine>StatusSet)
{
	for (int i = 0; i < StatusSet.size(); i++)
	{
		if (StatusSet[i].flag == 0)
		{
			StatusLine = StatusSet[i].Status;
			StatusLineIndex = i;
			return 1;
		}
	}
	return 0;
}
int isInStatusSet(vector<NFAnode*>StatusSet)
{
	vector<NFAnode*>StatusInSet;
	for (int i = 0; i < StatusSets.size(); i++)//在现有的状态机当中找是否存在一个和输入状态一样的状态集
	{
		StatusInSet = StatusSets[i].Status;
		if (StatusSet.size() != StatusInSet.size())//如果大小都不相等必然不是一个状态集
		{
			continue;
		}
		else//大小相等
		{
			int j = 0;
			for (; j < StatusInSet.size(); j++)
			{
				if (StatusSet[j] != StatusInSet[j])
				{
					break;
				}
			}
			if (j == StatusInSet.size())
			{
				return i;//在状态集中已经出现过
			}
		}
	}
	return -1;
}
//判断是否为终态
bool IsFinal(vector<NFAnode*>StatusSet)
{
	for (int i = 0; i < StatusSet.size(); i++)
	{
		if (StatusSet[i] == StatusStack.top().second)//包含终态
		{
			return 1;
		}
	}
	return 0;
}
#include <iostream>
#include <vector>

bool areArraysEqual(const std::vector<int>& array1, const std::vector<int>& array2) {

	for (size_t i = 0; i < array1.size(); ++i) {
		if (array1[i] != array2[i]) {
			return false; // 如果有任何不匹配的元素，返回false
		}
	}

	return true; // 如果没有不匹配的元素，返回true
}

void BuildDFA()
{
	while (ifExistNotmarked(StatusSets))//存在未标记的
	{
		StatusSets[StatusLineIndex].flag = 1;//先进行标记
		for (int i = 0; i < Operands.size(); i++)
		{
			char input = Operands[i];
			vector<NFAnode*>NextStatus;
			for (int j = 0; j < StatusLine.size(); j++)//遍历当前状态
			{
				for (const TransformState& row : TransformTable) {
					if ((row.LastStatus == StatusLine[j]) && (row.InputCharacter == input))
					{
						//找到一个状态,判断是不是已经在数组里了
						auto it = find(NextStatus.begin(), NextStatus.end(), row.NextStatus);
						if (it == NextStatus.end())//没有这个状态
						{
							NextStatus.push_back(row.NextStatus);//加入到数组当中
						}
					}
				}
			}
			if (!NextStatus.empty())//转换可以更新出新的状态
			{
				vector<NFAnode*>ClosureStatus = CalculateClosure(NextStatus);
				//判断这个状态在不在现有状态集中
				int found = isInStatusSet(ClosureStatus);//找到返回状态下标，没找到返回-1
				if (found == -1)
				{
					bool FinalLabel = IsFinal(ClosureStatus);
					StatusSets.push_back({ ClosureStatus,0,FinalLabel });//加入一个新状态
					TransformTableDFA.push_back({ StatusLineIndex,input,int(StatusSets.size()) - 1 });//当前状态，输入字符，新加入的状态下标就是最后一行
				}
				else
				{
					TransformTableDFA.push_back({ StatusLineIndex,input,found });
				}

			}
		}
	}
}
void MinDFA()
{
	vector<int> Final;
	vector<int> NotFinal;
	for (int i = 0; i < StatusSets.size(); i++)
	{
		if (StatusSets[i].isFinal == 1)
		{
			Final.push_back(i);
		}
		else
		{
			NotFinal.push_back(i);
		}
	}
	DFAState.push_back(Final);//第一行放所有终态
	DFAState.push_back(NotFinal);//第二行放初始化的所有非终态
	//打印终态行和非终态行
	for (int i = 0; i < DFAState.size(); i++)
	{
		vector<int>State = DFAState[i];
		for (int j = 0; j < State.size(); j++)
		{
			cout << State[j] << " ";
		}
		cout << endl;
	}
	//开始判断非终态行是否可以再分
	bool changed = true;
	while (changed)
	{
		changed = false;
		int SIZE = DFAState.size();
		for (int index = 1; index < SIZE; index++)//合并可能的非终态,
		{
			if (DFAState[index].size() > 1)
			{
				vector<vector<int>>NextDFAStateS;
				vector<int>State = DFAState[index];
				vector<UpdateStatus>UPDATE;
				//cout << "State.size():" << State.size();
				for (int j = 0; j < State.size(); j++)//第一轮，逐个取非终态
				{
					vector<int>NextState;
					for (char a : Operands)
					{
						int NextStatus = -1;//先假设找不到下一个状态
						for (const TransformStateDFA& row : TransformTableDFA) {
							if ((row.LastStatus == State[j]) && (row.InputCharacter == a))//DFA有唯一确定的下一个状态
							{//找到下一状态当前的索引
								for (int p = 0; p < DFAState.size(); p++)
								{
									for (int q = 0; q < DFAState[p].size(); q++)
									{
										if (row.NextStatus == DFAState[p][q])
										{
											NextStatus = p;
											break;
										}

									}
								}
							}
						}
						NextState.push_back(NextStatus);
					}
					NextDFAStateS.push_back(NextState);
					//cout << "NextDFAStateS.size():" << NextDFAStateS.size() << endl;
					/*if (j == 0) {
						vector<int>initStateIndex;
						initStateIndex.push_back(0);
						UPDATE.push_back({ NextDFAStateS[0],initStateIndex });
					}*/

					//尝试打印我的Update
				}
				vector<int>initStateIndex;
				initStateIndex.push_back(0);
				UPDATE.push_back({ NextDFAStateS[0],initStateIndex });
				for (int i = 1; i < NextDFAStateS.size(); i++)
				{
					vector<int>State = NextDFAStateS[i];//得到一个状态
					bool exist = 0;
					for (int j = 0; j < UPDATE.size(); j++)
					{
						bool outcome = areArraysEqual(State, UPDATE[j].nextstatus);
						if (outcome)
						{
							UPDATE[j].nowstatusindex.push_back(i);
							exist = 1;
							break;
						}
					}
					if (!exist)//状态被更新了
					{
						vector<int>NewStateIndex;
						NewStateIndex.push_back(i);
						UPDATE.push_back({ NextDFAStateS[i],NewStateIndex });
						changed = true;
					}
				}
				//打印状态转化
				for (int p = 0; p < NextDFAStateS.size(); p++)
				{
					for (int q = 0; q < NextDFAStateS[p].size(); q++)
					{
						cout << NextDFAStateS[p][q] << " ";
					}
					cout << endl;
				}
				//状态更新DFAState
				for (int i = 0; i < UPDATE.size(); i++) {
					for (int p = 0; p < UPDATE[i].nextstatus.size(); p++)
					{
						cout << UPDATE[i].nextstatus[p] << " ";
					}
					for (int p = 0; p < UPDATE[i].nowstatusindex.size(); p++)
					{
						cout << UPDATE[i].nowstatusindex[p] << " ";
					}
					cout << endl;
				}
				if (changed == true) {
					for (int i = UPDATE.size() - 1; i >= 0; i--) {
						if (i == 0)
						{
							vector<int>newDFAState;
							for (int p = 0; p < UPDATE[i].nowstatusindex.size(); p++)
							{
								int low_index = UPDATE[i].nowstatusindex[p];

								newDFAState.push_back(DFAState[index][low_index]);
							}
							DFAState[index] = newDFAState;

						}
						else {
							vector<int>newDFAState;
							for (int p = 0; p < UPDATE[i].nowstatusindex.size(); p++)
							{
								int low_index = UPDATE[i].nowstatusindex[p];
								newDFAState.push_back(DFAState[index][low_index]);
							}
							DFAState.push_back(newDFAState);
						}
					}
				}
			}

		}

	}
}
int UpdateStatus(int status)
{
	for (int i = 0; i < DFAState.size(); i++)
	{
		for (int j = 0; j < DFAState[i].size(); j++)
		{
			if (DFAState[i][j] == status)
			{
				return i;//找到更新之后的下标表示新状态
			}
		}
	}
	return -1;//正确的情况下应该不会到这里状态必然存在
}
void UpdateTransfromTableDFA()
{
	for (int i = 0; i < TransformTableDFA.size(); i++)
	{
		TransformTableDFA[i].LastStatus = UpdateStatus(TransformTableDFA[i].LastStatus);
		TransformTableDFA[i].NextStatus = UpdateStatus(TransformTableDFA[i].NextStatus);
	}
}
int main()
{
	string regex;
	cin >> regex;
	for (int i = 0; i < regex.length(); i++)
	{
		TEMP = regex[i];

		if (isoperator(TEMP))//是一个运算符
		{
			if ((TEMP == '(') && (i != 0) && (regex[i - 1] != '|'))
			{
				caculate_operator('&');
			}
			caculate_operator(TEMP);
		}
		else
		{
			auto it = find(Operands.begin(), Operands.end(), TEMP);
			if (it == Operands.end())//没有这个字符
			{
				Operands.push_back(TEMP);
			}
			if (i == 0)
			{
				//让运算符入栈
				Treenode* OperandNode = new Treenode;
				OperandNode->character = TEMP;
				OperandNode->leftchild = NULL;
				OperandNode->rightchild = NULL;
				OperandsStack.push(OperandNode);
			}
			else
			{
				if (regex[i - 1] == '(' || regex[i - 1] == '|')
				{
					;
				}
				else
				{
					//需要先加入隐藏的连接运算符
					caculate_operator('&');
				}
				//让运算符入栈
				Treenode* OperandNode = new Treenode;
				OperandNode->character = TEMP;
				OperandNode->leftchild = NULL;
				OperandNode->rightchild = NULL;
				OperandsStack.push(OperandNode);
			}
		}
	}
	while (!OperatorStack.empty())//非空，需要清空符号栈
	{
		OperatorStackTop = OperatorStack.top();//获得新栈顶
		Treenode* OperatorNode = new Treenode;
		if (OperatorStackTop == '*')//孩子只有一个，只需要一个运算符
		{
			OperatorNode->character = '*';
			OperatorNode->leftchild = OperandsStack.top();
			OperatorNode->rightchild = NULL;
			OperandsStack.pop();
			OperandsStack.push(OperatorNode);
		}
		else//需要两个运算符
		{
			OperatorNode->character = OperatorStackTop;
			OperatorNode->rightchild = OperandsStack.top();
			OperandsStack.pop();
			OperatorNode->leftchild = OperandsStack.top();
			OperandsStack.pop();
			OperandsStack.push(OperatorNode);
		}
		OperatorStack.pop();//出栈
	}
	//后续遍历检查
	postorderTraversal(OperandsStack.top());
	cout << endl;
	BuildNFA(OperandsStack.top());
	for (const TransformState& row : TransformTable) {
		cout << row.LastStatus << "\t\t" << row.InputCharacter << "\t\t" << row.NextStatus << endl;
	}
	cout << "TransformTable has " << TransformTable.size() << " rows." << endl;

	//NFA已经建立完毕，我们可以在表中得到所有边的信息，现在我们要将NFA->DFA,从NFA栈中取出开始状态
	//step1:先确定初始状态，此时必然是可以找到的所以没有设置找不到返回的默认值
	vector<NFAnode*>Status;
	Status.push_back(StatusStack.top().first);//本身已经被包含其中
	vector<NFAnode*>ClosureStatus = CalculateClosure(Status);
	bool FinalLabel = IsFinal(ClosureStatus);
	StatusSets.push_back({ ClosureStatus,0,FinalLabel });//加入一个新状态
	//在有了初状态的前提下开始建立DFA
	BuildDFA();
	//打印最终的状态集
	for (const StatusSetLine& row : StatusSets) {
		for (int i = 0; i < row.Status.size(); i++)
		{
			cout << row.Status[i] << " ";
		}
		cout << row.isFinal << endl;
	}
	//打印状态集的转化关系
	for (const TransformStateDFA& row : TransformTableDFA) {
		cout << row.LastStatus << "\t" << row.InputCharacter << "\t" << row.NextStatus << endl;
	}
	MinDFA();
	cout << "MIN StatusSet:" << endl;
	for (int i = 0; i < DFAState.size(); i++)
	{
		vector<int>State = DFAState[i];
		for (int j = 0; j < State.size(); j++)
		{
			cout << State[j] << " ";
		}
		cout << endl;
	}
	UpdateTransfromTableDFA();
	//打印状态集的转化关系
	cout << "FINAL TRANSFREOM TABLE:" << endl;
	for (const TransformStateDFA& row : TransformTableDFA) {
		cout << row.LastStatus << "\t" << row.InputCharacter << "\t" << row.NextStatus << endl;
	}
	return 0;
}