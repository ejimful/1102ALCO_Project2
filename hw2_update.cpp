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
	//將所使用的register push_back到use_reg中，並且將immediate寫入.value
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
	//在use_reg中找到需要變更數值的register，並且將immediate+=進去
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
	//先將要比較的兩個registers和label找出
	int n = inst[num].find(',');
	string reg1 = inst[num].substr(0, n);
	int last = inst[num].find(',', n + 1);
	string reg2 = inst[num].substr(n + 1, last - (n + 1));
	string l = inst[num].substr(last + 1, inst[num].size() - 1);
	if (reg1 == reg2)//如果是同一個register，一定會taken
	{
		for (int i = 0; i < label.size(); i++)
		{
			if (label[i] == l)//找到label
			{
				if (i + 1 == PCdex.size())//如果label是End的話(最後一行程式)，outcome=1，回傳-100
				{
					outcome = 1;
					return -100;
				}
				outcome = 1; // 如果label不是End的話，outcome = 1，回傳label下一行instruction的PCdex
				return PCdex[i + 1];
			}
		}
	}
	//以下為兩個register不同的case
	int reg1num = 0, reg2num = 0;
	for (int i = 0; i < use_reg.size(); i++)//先找出兩個register分別的value
	{
		if (use_reg[i].name == reg1)
			reg1num = i;
		if (use_reg[i].name == reg2)
			reg2num = i;
	}
	if (use_reg[reg1num].value != use_reg[reg2num].value)//value不相等就Not taken，outcome=1、回傳當前PC+4
	{
		outcome = 0;
		return PCdex[num] + 4;
	}
	for (int i = 0; i < label.size(); i++)//value相等，Taken
	{
		if (label[i] == l)
		{
			if (i + 1 == PCdex.size())//如果label是End的話(最後一行程式)，outcome=1，回傳-100
			{
				outcome = 1;
				return -100;
			}
			outcome = 1; //如果label不是End的話，outcome = 1，回傳label下一行instruction的PCdex
			return PCdex[i + 1];
		}
	}
}
void prediction(int thispredictor)
{
	//將預測結果(3-bit history、8個state的狀態)輸出
	cout << endl << predictor[thispredictor].bit[0] << predictor[thispredictor].bit[1] << predictor[thispredictor].bit[2] << ' ';
	for (int j = 0; j < 8; j++)
		cout << predictor[thispredictor].state_table[j] << ' ';
}
void update(int thispredictor, int outcome)
{
	state = 0;//根據3-bit histroy找出此次predict所依據的state
	for (int i = 2, j = 0; i >= 0; i--, j++)
		state += predictor[thispredictor].bit[i] * pow(2, j);
	int num;//找出t=state的狀態(SN, WN, WT, ST)
	for (int i = 0; i < 4; i++)
		if (table[i] == predictor[thispredictor].state_table[state])
			num = i;
	if (outcome == 0 && predictor[thispredictor].state_table[state][1] == 'N')//case: outcome=N, predict=N, 預測正確，state狀態往左跳
	{
		if (predictor[thispredictor].state_table[state] != "SN")//要做判斷，如果狀態為SN的話就不能跳
			predictor[thispredictor].state_table[state] = table[num - 1];
		misprediction = "not miss";
		pre = 'N';
	}
	else if (outcome == 0 && predictor[thispredictor].state_table[state][1] == 'T')//case: outcome=N, predict=T, 預測錯誤，state狀態往左跳，mispre++
	{
		predictor[thispredictor].state_table[state] = table[num - 1];
		misprediction = "miss";
		pre = 'T';
		predictor[thispredictor].mispre++;
	}
	else if (outcome == 1 && predictor[thispredictor].state_table[state][1] == 'T')//case: outcome=T, predict=T, 預測正確，state狀態往右跳
	{
		if (predictor[thispredictor].state_table[state] != "ST")//要做判斷，如果狀態為ST的話就不能跳
			predictor[thispredictor].state_table[state] = table[num + 1];
		misprediction = "not miss";
		pre = 'T';
	}
	else if (outcome == 1 && predictor[thispredictor].state_table[state][1] == 'N')//case: outcome=T, predict=N, 預測錯誤，state狀態往右跳，mispre++
	{
		predictor[thispredictor].state_table[state] = table[num + 1];
		misprediction = "miss";
		pre = 'N';
		predictor[thispredictor].mispre++;
	}
	if (outcome == 1)//將outcome轉為char
		outcomec = 'T';
	else
		outcomec = 'N';
	//輸出所使用的enrty編號、這個predictor的misprediction數量
	//輸出預測結果、真實結果、是否miss
	cout << "entry: " << thispredictor << ' ' << "misprediction: " << predictor[thispredictor].mispre;
	cout << endl << pre << ' ' << outcomec << ' ' << misprediction << endl;

	//更新3-bit history
	predictor[thispredictor].bit[0] = predictor[thispredictor].bit[1];
	predictor[thispredictor].bit[1] = predictor[thispredictor].bit[2];
	predictor[thispredictor].bit[2] = outcome;
}
int main()
{	
	ifstream fin("input.txt");//讀檔
	string input;
	while (getline(fin, input))//一行一行讀
	{
		if (input.find(':') != string::npos)//用':'找到label，要push_back PC,type,inst 空字串
		{
			input.erase(remove(input.begin(), input.end(), '\t'), input.end());
			label.push_back(input.substr(0, input.find(':')));
			PC.push_back("");
			type.push_back("");
			inst.push_back("");
		}
		else//如果找不到':'表示不是label，push_back label 空字串
		{
			label.push_back("");
			PC.push_back(input.substr(0, 5));
			int x = 5;
			char a = input[x];
			while (a == '\t')//找第一個不為tab的字元
			{
				x += 1;
				a = input[x];
			}
			//判斷type為addi, beq, li
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
			//將剩餘的內容(';'前)寫入inst
			int end = input.find(';');
			string temp = input.substr(x, end - x);
			temp.erase(remove(temp.begin(), temp.end(), ' '), temp.end());
			inst.push_back(temp);
		}
	}
	for (int i = 0; i < PC.size(); i++)//因為執行程式時會用到10進制的PC，所以多設一個PCdex，並且將PC轉換後放入。
	{
		int temp = -1;
		PCdex.push_back(temp);
		if (PC[i] != "")
			PCdex[i] = Base16To10(PC[i]);
	}
	cout << "Please input entry (entry>0) :" << endl;
	cin >> entry_num;
	while (entry_num <= 0 || (entry_num & (entry_num - 1)))//判斷entry是否>0且為2的次方數
	{
		cout << "Number of entries is an error!" << endl;
		cout << "Please input entry (entry>0) :" << endl;
		cin >> entry_num;
	}
	for (int i = 0; i < entry_num; i++)//將predictor的數量設為entry_num
	{
		predict temp;
		predictor.push_back(temp);
	}
	PCdex[PCdex.size() - 1] = -100;//把label: End的pc設為-100，方便程式運行
	int next = PCdex[0];//下一個要做的指令的PC
	int thistpredictor = -1;//這次預測所使用的predictor，初始值設為-1，迴圈開始後由0~(entry_num-1)不斷輪迴
	while (next != -100)//當next不等於label:End的時候，進入迴圈
	{
		int num = 0;//找出next instruction在input.txt檔中對應的行數
		for (; num < PCdex.size(); num++)
		{
			if (PCdex[num] == next)
				break;
		}
		cout << endl << PC[num] << ' ' << type[num] << ' ' << inst[num];//把整個instruction輸出
		thistpredictor = (thistpredictor + 1) % entry_num;//設定本次要用的predictor
		prediction(thistpredictor);//預測
		if (type[num] == "li")//如果是li，必定not taken，所以outcome=0、next直接+=4
		{
			calc_li(num);
			outcome = 0;
			next += 4;
		}
		else if (type[num] == "addi")//如果是addi，必定not taken，所以outcome=0、next直接+=4
		{
			calc_addi(num);
			outcome = 0;
			next += 4;
		}
		else if (type[num] == "beq") //如果是beq，要經過運算才能知道Taken or not
		{
			next = calc_beq(num);
		}
		update(thistpredictor, outcome);//根據outcome來更新thispredictor的state和3-bit history
	}
}