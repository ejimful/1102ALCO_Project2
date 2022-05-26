#include<iostream>
#include<fstream>
#include<string>
#include<vector>
#include<sstream>
#include <cmath>
#include<algorithm>
#include <cstdlib>
#include <iomanip>
using namespace std;

struct reg
{
	string name;
	int value;
};
struct predict
{
	string state_table[8] = { "SN","SN","SN","SN","SN","SN","SN","SN" };
	int bit[3] = { 0,0,0 };
	int mispre = 0;
};
vector<string>PC, type, inst, label;
vector <predict> predictor;
int outcome = 0;
char outcomec;
char pre;
int state;
int entry_num;
vector<int>PCdex;
vector<reg> use_reg;
string misprediction;
string table[4] = { "SN","WN","WT","ST" };
int Base2To10(string binary)
{
	int value = 0;
	reverse(binary.begin(), binary.end());
	for (int i = 0; i < binary.size(); i++)
		value += pow(2, i) * (binary[i] - '0');
	return value;
}
int Base16To10(string pc)
{
	int i = pc.size() - 1;
	int j = 0;
	int newPC = 0;
	while (pc[i] != 'x')
	{
		switch (pc[i]) {
		case 'A':
			newPC += pow(16, j) * 10;
			break;
		case 'B':
			newPC += pow(16, j) * 11;
			break;
		case 'C':
			newPC += pow(16, j) * 12;
			break;
		case 'D':
			newPC += pow(16, j) * 13;
			break;
		case 'E':
			newPC += pow(16, j) * 14;
			break;
		case 'F':
			newPC += pow(16, j) * 15;
			break;
		default:
			int num = pc[i] - '0';
			newPC += pow(16, j) * num;
			break;
		}
		i--;
		j++;
	}
	return newPC;
}
void calc_li(int num)
{
	//�N�ҨϥΪ�register push_back��use_reg���A�åB�Nimmediate�g�J.value
	int end = inst[num].find(',');
	string r = inst[num].substr(0, end);
	reg temp;
	temp.name = r;
	string imm = inst[num].substr(end + 1, inst[num].size());
	temp.value = stoi(imm);
	use_reg.push_back(temp);
}
void calc_addi(int num)
{	
	//�buse_reg�����ݭn�ܧ�ƭȪ�register�A�åB�Nimmediate+=�i�h
	int end = inst[num].find(',');
	int immediate = inst[num].find(',', end + 1);
	string temp = inst[num].substr(immediate + 1, inst[num].size() - (immediate + 1));
	string r = inst[num].substr(0, end);
	for (int i = 0; i < use_reg.size(); i++)
		if (use_reg[i].name == r)
			use_reg[i].value += stoi(temp);
}
int calc_beq(int num)
{
	//���N�n��������registers�Mlabel��X
	int n = inst[num].find(',');
	string reg1 = inst[num].substr(0, n);
	int last = inst[num].find(',', n + 1);
	string reg2 = inst[num].substr(n + 1, last - (n + 1));
	string l = inst[num].substr(last + 1, inst[num].size() - 1);
	if (reg1 == reg2)//�p�G�O�P�@��register�A�@�w�|taken
	{
		for (int i = 0; i < label.size(); i++)
		{
			if (label[i] == l)//���label
			{
				if (i + 1 == PCdex.size())//�p�Glabel�OEnd����(�̫�@��{��)�Aoutcome=1�A�^��-100
				{
					outcome = 1;
					return -100;
				}
				outcome = 1; // �p�Glabel���OEnd���ܡAoutcome = 1�A�^��label�U�@��instruction��PCdex
				return PCdex[i + 1];
			}
		}
	}
	//�H�U�����register���P��case
	int reg1num = 0, reg2num = 0;
	for (int i = 0; i < use_reg.size(); i++)//����X���register���O��value
	{
		if (use_reg[i].name == reg1)
			reg1num = i;
		if (use_reg[i].name == reg2)
			reg2num = i;
	}
	if (use_reg[reg1num].value != use_reg[reg2num].value)//value���۵��NNot taken�Aoutcome=1�B�^�Ƿ�ePC+4
	{
		outcome = 0;
		return PCdex[num] + 4;
	}
	for (int i = 0; i < label.size(); i++)//value�۵��ATaken
	{
		if (label[i] == l)
		{
			if (i + 1 == PCdex.size())//�p�Glabel�OEnd����(�̫�@��{��)�Aoutcome=1�A�^��-100
			{
				outcome = 1;
				return -100;
			}
			outcome = 1; //�p�Glabel���OEnd���ܡAoutcome = 1�A�^��label�U�@��instruction��PCdex
			return PCdex[i + 1];
		}
	}
}
void prediction(int thispredictor)
{
	//�N�w�����G(3-bit history�B8��state�����A)��X
	cout << endl << predictor[thispredictor].bit[0] << predictor[thispredictor].bit[1] << predictor[thispredictor].bit[2] << ' ';
	for (int j = 0; j < 8; j++)
		cout << predictor[thispredictor].state_table[j] << ' ';
}
void update(int thispredictor, int outcome)
{
	state = 0;//�ھ�3-bit histroy��X����predict�Ҩ̾ڪ�state
	for (int i = 2, j = 0; i >= 0; i--, j++)
		state += predictor[thispredictor].bit[i] * pow(2, j);
	int num;//��Xt=state�����A(SN, WN, WT, ST)
	for (int i = 0; i < 4; i++)
		if (table[i] == predictor[thispredictor].state_table[state])
			num = i;
	if (outcome == 0 && predictor[thispredictor].state_table[state][1] == 'N')//case: outcome=N, predict=N, �w�����T�Astate���A������
	{
		if (predictor[thispredictor].state_table[state] != "SN")//�n���P�_�A�p�G���A��SN���ܴN�����
			predictor[thispredictor].state_table[state] = table[num - 1];
		misprediction = "not miss";
		pre = 'N';
	}
	else if (outcome == 0 && predictor[thispredictor].state_table[state][1] == 'T')//case: outcome=N, predict=T, �w�����~�Astate���A�������Amispre++
	{
		predictor[thispredictor].state_table[state] = table[num - 1];
		misprediction = "miss";
		pre = 'T';
		predictor[thispredictor].mispre++;
	}
	else if (outcome == 1 && predictor[thispredictor].state_table[state][1] == 'T')//case: outcome=T, predict=T, �w�����T�Astate���A���k��
	{
		if (predictor[thispredictor].state_table[state] != "ST")//�n���P�_�A�p�G���A��ST���ܴN�����
			predictor[thispredictor].state_table[state] = table[num + 1];
		misprediction = "not miss";
		pre = 'T';
	}
	else if (outcome == 1 && predictor[thispredictor].state_table[state][1] == 'N')//case: outcome=T, predict=N, �w�����~�Astate���A���k���Amispre++
	{
		predictor[thispredictor].state_table[state] = table[num + 1];
		misprediction = "miss";
		pre = 'N';
		predictor[thispredictor].mispre++;
	}
	if (outcome == 1)//�Noutcome�ରchar
		outcomec = 'T';
	else
		outcomec = 'N';
	//��X�ҨϥΪ�enrty�s���B�o��predictor��misprediction�ƶq
	//��X�w�����G�B�u�굲�G�B�O�_miss
	cout << "entry: " << thispredictor << ' ' << "misprediction: " << predictor[thispredictor].mispre;
	cout << endl << pre << ' ' << outcomec << ' ' << misprediction << endl;

	//��s3-bit history
	predictor[thispredictor].bit[0] = predictor[thispredictor].bit[1];
	predictor[thispredictor].bit[1] = predictor[thispredictor].bit[2];
	predictor[thispredictor].bit[2] = outcome;
}
int main()
{	
	ifstream fin("input.txt");//Ū��
	string input;
	while (getline(fin, input))//�@��@��Ū
	{
		if (input.find(':') != string::npos)//��':'���label�A�npush_back PC,type,inst �Ŧr��
		{
			input.erase(remove(input.begin(), input.end(), '\t'), input.end());
			label.push_back(input.substr(0, input.find(':')));
			PC.push_back("");
			type.push_back("");
			inst.push_back("");
		}
		else//�p�G�䤣��':'��ܤ��Olabel�Apush_back label �Ŧr��
		{
			label.push_back("");
			PC.push_back(input.substr(0, 5));
			int x = 5;
			char a = input[x];
			while (a == '\t')//��Ĥ@�Ӥ���tab���r��
			{
				x += 1;
				a = input[x];
			}
			//�P�_type��addi, beq, li
			if (a == 'l')
			{
				type.push_back(input.substr(x, 2));
				x += 3;
			}
			else if (a == 'b')
			{
				type.push_back(input.substr(x, 3));
				x += 4;
			}
			else if (a == 'a')
			{
				type.push_back(input.substr(x, 4));
				x += 5;
			}
			//�N�Ѿl�����e(';'�e)�g�Jinst
			int end = input.find(';');
			string temp = input.substr(x, end - x);
			temp.erase(remove(temp.begin(), temp.end(), ' '), temp.end());
			inst.push_back(temp);
		}
	}
	for (int i = 0; i < PC.size(); i++)//�]������{���ɷ|�Ψ�10�i�PC�A�ҥH�h�]�@��PCdex�A�åB�NPC�ഫ���J�C
	{
		int temp = -1;
		PCdex.push_back(temp);
		if (PC[i] != "")
			PCdex[i] = Base16To10(PC[i]);
	}
	cout << "Please input entry (entry>0) :" << endl;
	cin >> entry_num;
	while (entry_num <= 0 || (entry_num & (entry_num - 1)))//�P�_entry�O�_>0�B��2�������
	{
		cout << "Number of entries is an error!" << endl;
		cout << "Please input entry (entry>0) :" << endl;
		cin >> entry_num;
	}
	for (int i = 0; i < entry_num; i++)//�Npredictor���ƶq�]��entry_num
	{
		predict temp;
		predictor.push_back(temp);
	}
	PCdex[PCdex.size() - 1] = -100;//��label: End��pc�]��-100�A��K�{���B��
	int next = PCdex[0];//�U�@�ӭn�������O��PC
	int thistpredictor = -1;//�o���w���ҨϥΪ�predictor�A��l�ȳ]��-1�A�j��}�l���0~(entry_num-1)���_���j
	while (next != -100)//��next������label:End���ɭԡA�i�J�j��
	{
		int num = 0;//��Xnext instruction�binput.txt�ɤ����������
		for (; num < PCdex.size(); num++)
		{
			if (PCdex[num] == next)
				break;
		}
		cout << endl << PC[num] << ' ' << type[num] << ' ' << inst[num];//����instruction��X
		thistpredictor = (thistpredictor + 1) % entry_num;//�]�w�����n�Ϊ�predictor
		prediction(thistpredictor);//�w��
		if (type[num] == "li")//�p�G�Oli�A���wnot taken�A�ҥHoutcome=0�Bnext����+=4
		{
			calc_li(num);
			outcome = 0;
			next += 4;
		}
		else if (type[num] == "addi")//�p�G�Oaddi�A���wnot taken�A�ҥHoutcome=0�Bnext����+=4
		{
			calc_addi(num);
			outcome = 0;
			next += 4;
		}
		else if (type[num] == "beq") //�p�G�Obeq�A�n�g�L�B��~�ા�DTaken or not
		{
			next = calc_beq(num);
		}
		update(thistpredictor, outcome);//�ھ�outcome�ӧ�sthispredictor��state�M3-bit history
	}
}