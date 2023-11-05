#include<iostream>
#include<string>
#include<stack>
#include<vector>
#include<algorithm>
using namespace std;

typedef struct Treenode {
	char character;
	Treenode*leftchild, *rightchild;
}Treenode,*Tree;
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
}TransformStateDFA;//�������±�Ψһ���һ��DFA��״̬�л�
typedef struct StatusSetLine
{
	vector<NFAnode*>Status;
	bool flag;
	bool isFinal;
}StatusSetLine;
vector<TransformState>TransformTable;//���ڴ洢����״̬ת������
vector<TransformStateDFA>TransformTableDFA;//���ڴ洢����״̬ת������
vector<StatusSetLine>StatusSets;//������״̬��
vector<NFAnode*> StatusLine;//��ŵ�ǰɨ��õ���δ���״̬
int StatusLineIndex = 0;//���״̬���������м���

char TEMP;//��ǵ�ǰɨ����ַ�
vector<char>Operands;
stack<char>OperatorStack;
stack<Tree>OperandsStack;
stack<pair<NFAnode*, NFAnode*>>StatusStack;//NFAջ
char OperatorStackTop;
Treenode *OperandsStackTop;
int isoperator(char temp)//�жϼ�����Ƿ�Ϊ�����
{
	if (temp == '(' || temp == ')' || temp == '|' || temp == '*'||temp=='&')
	{
		return 1;
	}
	else{
		return 0;
	}
}
//�ж���������ȼ�
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
//�������ѹջ��������
void caculate_operator(char TEMP)
{
	if (TEMP == '(')
	{  
		OperatorStack.push(TEMP);//������ֱ��ѹջ
	}
	else if (TEMP == ')')
	{
		while ((OperatorStackTop=OperatorStack.top()) != '(')
		{
			Treenode* OperatorNode = new Treenode;
			if (OperatorStackTop == '*')//����ֻ��һ����ֻ��Ҫһ�������
			{
				OperatorNode->character = '*';
				OperatorNode->leftchild = OperandsStack.top();
				OperatorNode->rightchild = NULL;
				OperandsStack.pop();
				OperandsStack.push(OperatorNode);
			}
			else//��Ҫ���������
			{
				OperatorNode->character = OperatorStackTop;
				OperatorNode->rightchild = OperandsStack.top();
				OperandsStack.pop();
				OperatorNode->leftchild = OperandsStack.top();
				OperandsStack.pop();
				OperandsStack.push(OperatorNode);
			}
			OperatorStack.pop();//��ջ
		}
		OperatorStack.pop();//��ջ������
	}
	else //����������*&|
	{
		while ((!OperatorStack.empty())&&(getOperatorPrecedence(OperatorStack.top()) >= getOperatorPrecedence(TEMP)))//ջ��Ϊ����ջ�����ŵ����ȼ��������Ҫ����ջ����ջ
		{
			OperatorStackTop = OperatorStack.top();
			Treenode* OperatorNode = new Treenode;
			if (OperatorStackTop == '*')//����ֻ��һ����ֻ��Ҫһ�������
			{
				OperatorNode->character = '*';
				OperatorNode->leftchild = OperandsStack.top();
				OperatorNode->rightchild = NULL;
				OperandsStack.pop();
				OperandsStack.push(OperatorNode);
			}
			else//��Ҫ���������
			{
				OperatorNode->character = OperatorStackTop;
				OperatorNode->rightchild= OperandsStack.top();//��һ��������Ϊ�Һ���
				OperandsStack.pop();
				OperatorNode->leftchild = OperandsStack.top();
				OperandsStack.pop();
				OperandsStack.push(OperatorNode);
			}
			OperatorStack.pop();//��ջ
			
		}
		OperatorStack.push(TEMP);
	}
}
//�������ʵ�ּ��
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
		//����ӵ��һ���µĿ�ʼ״̬�ͽ���״̬
		NFAnode*StartStatus = new NFAnode;
		NFAnode*EndStatus = new NFAnode;
		EndStatus->inputStatus1 = '$';
		EndStatus->Status1 = NULL;
		EndStatus->inputStatus2 = '$';//����ֻҪռλ���ɣ�ֻҪ�ǿ����״̬�治���ڣ����ڲŻῴ���룡����
		EndStatus->Status2 = NULL;
		if (!isoperator(root->character))//��������������״̬�ӵ�1��
		{
			StartStatus->inputStatus1 = root->character;
			StartStatus->Status1 = EndStatus;
			StartStatus->inputStatus2 ='$';//����ֻҪռλ���ɣ�ֻҪ�ǿ����״̬�治���ڣ����ڲŻῴ���룡����
			StartStatus->Status2 = NULL;
			//�����ҵ��б�
			TransformTable.push_back({ StartStatus,root->character,EndStatus });
		}
		else//�������|&*������&ʹ�úϲ�����
		{
			if (root->character == '*')
			{
				pair<NFAnode*, NFAnode*> LastStatusPair = StatusStack.top();//ȡ��ջ��
				NFAnode*LastStartStatus = LastStatusPair.first;
				NFAnode*LastEndStatus = LastStatusPair.second;
				LastEndStatus->inputStatus1 = '@';//״̬1����������
				LastEndStatus->Status1 = EndStatus;
				LastEndStatus->inputStatus2 = '@';
				LastEndStatus->Status2 = LastStartStatus;//״̬2ѭ������
				StartStatus->inputStatus1 = '@';
				StartStatus->Status1 = LastStartStatus;//״̬1������
				StartStatus->inputStatus2 = '@';
				StartStatus->Status2 = EndStatus;//״̬2ֱ�ӵ�����
				StatusStack.pop();//ջ��Ԫ��ѹ��ջ
				//�����ҵ��б�
				TransformTable.push_back({ StartStatus,'@',LastStartStatus });
				TransformTable.push_back({ StartStatus,'@',EndStatus });
				TransformTable.push_back({ LastEndStatus,'@',EndStatus });
				TransformTable.push_back({ LastEndStatus,'@',LastStartStatus });
			}
			else if (root->character == '&')//״̬֮�䶼��״̬1����
			{//��Ҫ����״̬�������и����ϵ��������ں����
				pair<NFAnode*, NFAnode*> LastAfterStatusPair = StatusStack.top();//ȡ��ջ��
				NFAnode*LastAfterStartStatus = LastAfterStatusPair.first;
				NFAnode*LastAfterEndStatus = LastAfterStatusPair.second;
				StatusStack.pop();//ȡ�������ں����״̬
				pair<NFAnode*, NFAnode*> LastBeforeStatusPair = StatusStack.top();//ȡ��ջ��
				NFAnode*LastBeforeStartStatus = LastBeforeStatusPair.first;
				NFAnode*LastBeforeEndStatus = LastBeforeStatusPair.second;
				StatusStack.pop();//ȡ��������ǰ���״̬
				StartStatus->inputStatus1 = '@';
				StartStatus->Status1 = LastBeforeStartStatus;
				StartStatus->inputStatus2 = '$';
				StartStatus->Status2 = NULL;
				LastBeforeEndStatus->inputStatus1 = '@';
				LastBeforeEndStatus->Status1 = LastAfterStartStatus;
				LastAfterEndStatus->inputStatus1 = '@';
				LastAfterEndStatus->Status1 = EndStatus;
				//�����ҵ��б�
				TransformTable.push_back({ StartStatus,'@',LastBeforeStartStatus });
				TransformTable.push_back({ LastBeforeEndStatus ,'@',LastAfterStartStatus });
				TransformTable.push_back({ LastAfterEndStatus ,'@',EndStatus });
			}
			else//�����|
			{
				pair<NFAnode*, NFAnode*> LastStatusPair1 = StatusStack.top();//ȡ��ջ��
				NFAnode*LastStartStatus1 = LastStatusPair1.first;
				NFAnode*LastEndStatus1 = LastStatusPair1.second;
				StatusStack.pop();//ȡ�������ں����״̬
				pair<NFAnode*, NFAnode*> LastStatusPair2 = StatusStack.top();//ȡ��ջ��
				NFAnode*LastStartStatus2 = LastStatusPair2.first;
				NFAnode*LastEndStatus2 = LastStatusPair2.second;
				StatusStack.pop();//ȡ��������ǰ���״̬
				StartStatus->inputStatus1 = '@';
				StartStatus->Status1 = LastStartStatus1;
				StartStatus->inputStatus2 = '@';
				StartStatus->Status2 = LastStartStatus2;
				//ԭ������̬����һ��״̬������״̬1�ϼ���
				LastEndStatus1->inputStatus1 = '@';
				LastEndStatus1->Status1 = EndStatus;
				LastEndStatus2->inputStatus1 = '@';
				LastEndStatus2->Status1 = EndStatus;
				//�����ҵ��б�
				TransformTable.push_back({ StartStatus,'@',LastStartStatus1 });
				TransformTable.push_back({ StartStatus,'@',LastStartStatus2 });
				TransformTable.push_back({ LastEndStatus1,'@',EndStatus });
				TransformTable.push_back({ LastEndStatus2,'@',EndStatus });
			}
		}
		//״̬���������״̬��ջ
		pair<NFAnode*, NFAnode*>StartEndPair(StartStatus, EndStatus);
		StatusStack.push(StartEndPair);
	}
}
vector<NFAnode*> CalculateClosure(vector<NFAnode*>node)//����һ���ڵ�ıհ�״̬������Ĭ�Ͻڵ��Ȼ���е�
{	
	stack<NFAnode*>tempNodeStack;//�����ʱ״̬
	NFAnode* tempNode;
	for (int i = 0; i < node.size(); i++)
	{
		tempNodeStack.push(node[i]);//node��ջ
	}
	
	while (!tempNodeStack.empty())//ֻҪջ�����ǿյ�
	{
		tempNode = tempNodeStack.top();//ȡ
		tempNodeStack.pop();
		for (const TransformState& row : TransformTable) {
			if ((row.LastStatus == tempNode) && (row.InputCharacter == '@'))
			{
				//�ҵ�һ��״̬,�ж��ǲ����Ѿ�����������
				auto it = find(node.begin(), node.end(), row.NextStatus);
				if (it == node.end())//û�����״̬
				{
					node.push_back(row.NextStatus);//���뵽���鵱��
					tempNodeStack.push(row.NextStatus);//ѹ��ջ��
				}
			}
		}

	}
	sort(node.begin(), node.end());//����ַ��˳�����з����ж��ǲ���ͬһ��״̬����
	for (int i = 0; i < node.size(); i++)
	{
		cout << node[i] << " ";
	}
	cout << "״̬��Ӧ�ıհ��Ĵ�С:" << node.size() << endl;
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
	for (int i = 0; i < StatusSets.size(); i++)//�����е�״̬���������Ƿ����һ��������״̬һ����״̬��
	{
		StatusInSet = StatusSets[i].Status;
		if(StatusSet.size()!= StatusInSet.size())//�����С������ȱ�Ȼ����һ��״̬��
		{
			continue;
		}
		else//��С���
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
				return i;//��״̬�����Ѿ����ֹ�
			}
		}
	}
	return -1;
}
//�ж��Ƿ�Ϊ��̬
bool IsFinal(vector<NFAnode*>StatusSet)
{
	for (int i = 0; i < StatusSet.size(); i++)
	{
		if (StatusSet[i] == StatusStack.top().second)//������̬
		{
			return 1;
		}
	}
	return 0;
}
void BuildDFA()
{
	while (ifExistNotmarked(StatusSets))//����δ��ǵ�
	{
		StatusSets[StatusLineIndex].flag = 1;//�Ƚ��б��
		for (int i = 0; i < Operands.size(); i++)
		{
			char input = Operands[i];
			vector<NFAnode*>NextStatus;
			for (int j = 0; j < StatusLine.size(); j++)//������ǰ״̬
			{
				for (const TransformState& row : TransformTable) {
					if ((row.LastStatus == StatusLine[j]) && (row.InputCharacter == input))
					{
						//�ҵ�һ��״̬,�ж��ǲ����Ѿ�����������
						auto it = find(NextStatus.begin(), NextStatus.end(), row.NextStatus);
						if (it == NextStatus.end())//û�����״̬
						{
							NextStatus.push_back(row.NextStatus);//���뵽���鵱��
						}
					}
				}
			}
			if (!NextStatus.empty())//ת�����Ը��³��µ�״̬
			{
				vector<NFAnode*>ClosureStatus = CalculateClosure(NextStatus);
				//�ж����״̬�ڲ�������״̬����
				int found = isInStatusSet(ClosureStatus);//�ҵ�����״̬�±꣬û�ҵ�����-1
				if (found == -1)
				{
					bool FinalLabel = IsFinal(ClosureStatus);
					StatusSets.push_back({ ClosureStatus,0,FinalLabel});//����һ����״̬
					TransformTableDFA.push_back({ StatusLineIndex,input,int(StatusSets.size()) - 1 });//��ǰ״̬�������ַ����¼����״̬�±�������һ��
				}
				else
				{
					TransformTableDFA.push_back({ StatusLineIndex,input,found });
				}
				
			}
		}
	}
}
int main()
{
	string regex;
	cin >> regex;
	for (int i = 0; i < regex.length();i++)
	{
		TEMP = regex[i];
		
		if (isoperator(TEMP))//��һ�������
		{
			if ((TEMP == '(') && (i != 0) && (regex[i - 1] != '|'))
			{
			caculate_operator('&');
		}
			caculate_operator(TEMP);
		}
		else
		{
			auto it = find(Operands.begin(), Operands.end(), TEMP );
			if (it == Operands.end())//û������ַ�
			{
				Operands.push_back(TEMP);
			}
			if (i == 0)
			{
				//���������ջ
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
					//��Ҫ�ȼ������ص����������
					caculate_operator('&');
				}
				//���������ջ
				Treenode* OperandNode = new Treenode;
				OperandNode->character = TEMP;
				OperandNode->leftchild = NULL;
				OperandNode->rightchild = NULL;
				OperandsStack.push(OperandNode);
			}	
		}
	}
	while (!OperatorStack.empty())//�ǿգ���Ҫ��շ���ջ
	{
		OperatorStackTop = OperatorStack.top();//�����ջ��
		Treenode* OperatorNode = new Treenode;
		if (OperatorStackTop == '*')//����ֻ��һ����ֻ��Ҫһ�������
		{
			OperatorNode->character = '*';
			OperatorNode->leftchild = OperandsStack.top();
			OperatorNode->rightchild = NULL;
			OperandsStack.pop();
			OperandsStack.push(OperatorNode);
		}
		else//��Ҫ���������
		{
			OperatorNode->character = OperatorStackTop;
			OperatorNode->rightchild = OperandsStack.top();
			OperandsStack.pop();
			OperatorNode->leftchild = OperandsStack.top();
			OperandsStack.pop();
			OperandsStack.push(OperatorNode);
		}
		OperatorStack.pop();//��ջ
	}
	//�����������
	postorderTraversal(OperandsStack.top());
	cout << endl;
	BuildNFA(OperandsStack.top());
	for (const TransformState& row : TransformTable) {
		cout << row.LastStatus << "\t\t" << row.InputCharacter << "\t\t" << row.NextStatus << endl;
	}
	cout << "TransformTable has " << TransformTable.size() << " rows." << endl;

	//NFA�Ѿ�������ϣ����ǿ����ڱ��еõ����бߵ���Ϣ����������Ҫ��NFA->DFA,��NFAջ��ȡ����ʼ״̬
    //step1:��ȷ����ʼ״̬����ʱ��Ȼ�ǿ����ҵ�������û�������Ҳ������ص�Ĭ��ֵ
	vector<NFAnode*>Status;
	Status.push_back(StatusStack.top().first);//�����Ѿ�����������
	vector<NFAnode*>ClosureStatus=CalculateClosure(Status);
	bool FinalLabel = IsFinal(ClosureStatus);
	StatusSets.push_back({ ClosureStatus,0,FinalLabel });//����һ����״̬
	//�����˳�״̬��ǰ���¿�ʼ����DFA
	BuildDFA();
	//��ӡ���յ�״̬��
	for (const StatusSetLine& row : StatusSets) {
		for (int i = 0; i < row.Status.size(); i++)
		{
			cout << row.Status[i] << " ";
		}
		cout <<row.isFinal<<endl;
	}
	//��ӡ״̬����ת����ϵ
	for (const TransformStateDFA& row : TransformTableDFA) {
		cout << row.LastStatus << "\t" << row.InputCharacter << "\t" << row.NextStatus << endl;
	}
	return 0;	
}