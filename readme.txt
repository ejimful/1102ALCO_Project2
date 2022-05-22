0x000		li R3,0;
0x004		li R4,4;
	Loop:
0x008		beq R4,R3,End;
0x00C		addi R3,R3,1;
0x010		beq R0,R0,Loop;
	End:
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
struct:
	reg: 存放register name和value。
	predict: 存放predictor的state和3_bit_history，另外還存放了每個state的預測值。
vector:
	<string>PC: 存放instruction的Program counter
	<string>type: 存放instruction的種類，ex:addi, beq......
	<string>inst: 存放instruction所用到的register、immedeate
	<string>label: 存放label名稱 
	<int>PCdex: 存放instruction的Program counter，10進制的digit
	<reg> use_reg: 存放使用到的register和值
	<predict> predictor: 有幾個entry就有幾個predictor

int outcome: Taken = 1; Not taken = 0
int state: 當前的預測使用哪一個state
char out: Taken = 'T'; Not taken = 'N'
char pre: 預測結果。Taken = 'T'; Not taken = 'N'
string missprediction: 是否預測失誤。
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
將.txt檔一行一行讀進來，用':'判斷是否為label。
不管是label或instruction，PC、type、inst、label、PCdex都會push_back!!!

輸入共有幾個entry，這邊做了是否>0和是否為2的次方數的判斷，若任一條件不符就要再繼續輸入。
將Predictor增加成entry的數目，每個predictor都初始設定都是000 SN SN SN SN SN SN SN SN。

next存放下一個inst的PCdex。
nextpredictor存放下一個使用的predictor。

為了方便程式運行，我將END label的PC設為-100，再跑程式時只要nextPC為-100就結束。

將num存next對應的PC後，先cout完整的instruction。
接下來做ALU運算，我的input比較簡單，做的運算只有li、addi、beq，
分別用calc_li()、calc_addi()、calc_beq()去運算。
li和addi永遠為Not Taken，所以outcome為0、next += 4。
beq則是要去判斷是否taken，outcome可能為0或1，next可能+=4或跳去label的下一個instruction。

接下來做prediction，使用prediction()。
先將當前使用的預測器和state都cout出來，
先找出state的十進制，這樣就可以直接取出需要使用的state的狀態。
接著去判斷outcome和prediction state是否一致，並且去更新predictor的state_table、missrpediction。
如果miss的話要將predictor的mispre++。
最後cout 使用的predictor、mis數量、pre、outcome、missprediction。

最後回到main()，更新3 bit history，就可以在跑下一個迴圈。


