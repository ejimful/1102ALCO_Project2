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
	string state_table[8] = { "SN","SN","SN" ,"SN", "SN","SN","SN" ,"SN" };
	char table[8] = { 'N','N','N','N', 'N','N','N','N' };
	int bit[3] = { 0,0,0 };
	int mispre = 0;
};

vector<string>PC, type, inst, label;
int outcome = 0;
char out;
char pre;
int state;
vector<int>PCdex;
vector<reg> use_reg;
int entry_num;
string missprediction;
vector <predict> predictor;
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
	int end = inst[num].find(',');
	int immdeate = inst[num].find(',', end + 1);
	string temp = inst[num].substr(immdeate + 1, inst[num].size() - (immdeate + 1));
	string r = inst[num].substr(0, end);
	for (int i = 0; i < use_reg.size(); i++)
		if (use_reg[i].name == r)
			use_reg[i].value += stoi(temp);
}
int calc_beq(int num)
{
	int n = inst[num].find(',');
	string reg1 = inst[num].substr(0, n);
	int last = inst[num].find(',', n + 1);
	string reg2 = inst[num].substr(n + 1, last - (n + 1));
	string l = inst[num].substr(last + 1, inst[num].size() - 1);
	if (reg1 == reg2)
	{
		for (int i = 0; i < label.size(); i++)
		{
			if (label[i] == l)
			{
				if (i + 1 == PCdex.size())
				{
					outcome = 1;
					return -100;
				}
				outcome = 1;
				return PCdex[i + 1];
			}
		}
	}
	int reg1num = 0, reg2num = 0;
	for (int i = 0; i < use_reg.size(); i++)
	{
		if (use_reg[i].name == reg1)
			reg1num = i;
		if (use_reg[i].name == reg2)
			reg2num = i;
	}
	if (use_reg[reg1num].value != use_reg[reg2num].value)
	{
		outcome = 0;
		return PCdex[num] + 4;
	}
	for (int i = 0; i < label.size(); i++)
	{
		if (label[i] == l)
		{
			if (i + 1 == PCdex.size())
			{
				outcome = 1;
				return -100;
			}
			outcome = 1;
			return PCdex[i + 1];
		}
	}
}
void prediction(int outcome, int thispredictor)
{
	cout << endl << predictor[thispredictor].bit[0] << predictor[thispredictor].bit[1] << predictor[thispredictor].bit[2] << ' ';
	for (int j = 0; j < 8; j++)
		cout << predictor[thispredictor].state_table[j] << ' ';
	if (outcome == 1)
		out = 'T';
	else
		out = 'N';
	int s = 0;
	for (int i = 2, j = 0; i >= 0; i--, j++)
		s += predictor[thispredictor].bit[i] * pow(2, j);
	state = s;
	int num;
	for (int i = 0; i < 4; i++)
		if (table[i] == predictor[thispredictor].state_table[s])
			num = i;
	if (outcome == 0 && predictor[thispredictor].table[s] == 'N')
	{
		if (predictor[thispredictor].state_table[s] != "SN")
			predictor[thispredictor].state_table[s] = table[num - 1];
		missprediction = "not miss";
		pre = 'N';
	}
	else if (outcome == 0 && predictor[thispredictor].table[s] == 'T')
	{
		if (predictor[thispredictor].state_table[s] == "WT")
			predictor[thispredictor].table[s] = 'N';
		predictor[thispredictor].state_table[s] = table[num - 1];
		missprediction = "miss";
		pre = 'T';
		predictor[thispredictor].mispre ++;
	}
	else if (outcome == 1 && predictor[thispredictor].table[s] == 'T')
	{
		if (predictor[thispredictor].state_table[s] != "ST")
			predictor[thispredictor].state_table[s] = table[num + 1];
		missprediction = "not miss";
		pre = 'T';
	}
	else if (outcome == 1 && predictor[thispredictor].table[s] == 'N')
	{
		if (predictor[thispredictor].state_table[s] != "WN")
			predictor[thispredictor].table[s] = 'T';
		predictor[thispredictor].state_table[s] = table[num + 1];
		missprediction = "miss";
		pre = 'N';
		predictor[thispredictor].mispre++;
	}
	cout << "entry: " << thispredictor << ' ' << "misprediction: " << predictor[thispredictor].mispre;
	cout << endl << pre << ' ' << out << ' ' << missprediction << endl;
}
int main()
{
	ifstream fin("input.txt");
	string input;
	while (getline(fin, input))
	{
		if (input.find(':') != string::npos)//找到label
		{
			input.erase(remove(input.begin(), input.end(), '\t'), input.end());
			label.push_back(input.substr(0, input.find(':')));
			PC.push_back("");
			type.push_back("");
			inst.push_back("");
		}
		else
		{
			label.push_back("");
			PC.push_back(input.substr(0, 5));
			int x = 5;
			char a = input[x];
			while (a == '\t')
			{
				x += 1;
				a = input[x];
			}
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
			int end = input.find(';');
			string temp = input.substr(x, end - x);
			temp.erase(remove(temp.begin(), temp.end(), ' '), temp.end());
			inst.push_back(temp);
		}
	}
	for (int i = 0; i < PC.size(); i++)
	{
		int temp = -1;
		PCdex.push_back(temp);
		if (PC[i] != "")
			PCdex[i] = Base16To10(PC[i]);
	}
	cout << "Please input entry (entry>0) :" << endl;
	cin >> entry_num;
	while (entry_num <= 0 || (entry_num & (entry_num - 1)))
	{
		cout << "Number of entries is an error!" << endl;
		cout << "Please input entry (entry>0) :" << endl;
		cin >> entry_num;
	}
	for (int i = 0; i < entry_num; i++)
	{
		predict temp;
		predictor.push_back(temp);
	}
	PCdex[PCdex.size() - 1] = -100;
	int next = PCdex[0];
	int nextpredictor = -1;
	while (next != -100)
	{
		int num = 0;
		for (; num < PCdex.size(); num++)
		{
			if (PCdex[num] == next)
				break;
		}
		cout << endl << PC[num] << ' ' << type[num] << ' ' << inst[num];
		nextpredictor = (nextpredictor + 1) % entry_num;
		if (type[num] == "li")
		{
			calc_li(num);
			outcome = 0;
			next += 4;
		}
		else if (type[num] == "addi")
		{
			calc_addi(num);
			outcome = 0;
			next += 4;
		}
		else if (type[num] == "beq")
		{
			next = calc_beq(num);
		}
		prediction(outcome, nextpredictor);
		predictor[nextpredictor].bit[0] = predictor[nextpredictor].bit[1];
		predictor[nextpredictor].bit[1] = predictor[nextpredictor].bit[2];
		predictor[nextpredictor].bit[2] = outcome;
	}
}