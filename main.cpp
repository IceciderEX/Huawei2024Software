#include <bits/stdc++.h>
#include <iostream>
using namespace std;

const int n = 200;//地图大小 
const int robot_num = 10;//机器人数量 
const int berth_num = 10;//泊位数量 
const int N = 210;

struct Robot//机器人 
{
    int x, y, goods;//x,y为机器人的位置。goods表示机器人拿货物没有 ，1表示拿了货物 
    int status;//状态，1表示正在运行，0表示正在恢复 
    int mbx, mby;//暂时未发现作用 
    Robot() {}
    Robot(int startX, int startY) {
        x = startX;
        y = startY;
    }
}robot[robot_num + 10];

struct Berth
{
    int x;
    int y;
    int transport_time;//虚拟点移动到泊位以及泊位移动到虚拟点的泊位 
    int loading_speed;
    int busy;//泊位中是否有船或者即将有船 
    int goods_num;//泊位中的货物数量 
    Berth(){}
    Berth(int x, int y, int transport_time, int loading_speed) {
        this -> x = x;
        this -> y = y;
        this -> transport_time = transport_time;
        this -> loading_speed = loading_speed;
    }
}berth[berth_num + 10];
 
struct Boat
{
    int num, pos, status;//num为装货数量，pos为轮船的位置，status为轮船在哪 
}boat[10];

int money, boat_capacity, id;
char ch[N][N];//地图 
int goods_time=1000; 
struct Goods{
	int x,y,val,left_time= goods_time;
};
set<Goods>gds;
void Init()
{
    for(int i = 1; i <= n; i ++)
        scanf("%s", ch[i] + 1);//读入地图 
    for(int i = 0; i < berth_num; i ++)
    {
        int id;
        scanf("%d", &id);
        scanf("%d%d%d%d", &berth[id].x, &berth[id].y, &berth[id].transport_time, &berth[id].loading_speed);
    }
    scanf("%d", &boat_capacity);
    char okk[100];
    scanf("%s", okk);
    printf("OK\n");
    fflush(stdout);
}
int Input()
{
    scanf("%d%d", &id, &money);
    int num;
    scanf("%d", &num);//新增货物数量 
    for(int i = 1; i <= num; i ++)
    {
        Goods *tmp=(Goods*)malloc(sizeof(Goods));
        scanf("%d%d%d", &tmp->x, &tmp->y, &tmp->val);
    }
    for(int i = 0; i < robot_num; i ++)
    {
        int sts;
        scanf("%d%d%d%d", &robot[i].goods, &robot[i].x, &robot[i].y, &sts);//1表示携带物品，1表示正常运行 
    }
    for(int i = 0; i < 5; i ++)
        scanf("%d%d\n", &boat[i].status, &boat[i].pos);
    char okk[100];
    scanf("%s", okk);
    return id;
} 
void find_berth(int boat_id){//从虚拟点出发找泊位 
	int tmp=2000000;
	int finall=99;
	for(int berth_id = 0; berth_id < 10 ;berth_id ++){
		if(berth[berth_id].transport_time < tmp&& berth[berth_id].busy == 0){
			tmp = berth[berth_id].transport_time;
			finall = berth_id; 
		}
	} 
	if(finall == 99) return;//实在找不到泊位就先不动 
	printf("ship %d %d\n", boat_id, finall);//让船往前往耗时最少的泊位 
	berth[finall].busy = 1;//将该泊位设置为忙碌状态 
	return; 
}
void Boat_Init(){ 
	for(int boat_id = 0; boat_id < 5; boat_id ++){
		boat[boat_id].pos = -1;
		find_berth(boat_id);
	}
}
int check_go(int boat_id){//若装满了货物，让轮船出发 
	if(boat[boat_id].num == boat_capacity) return 1;
	return 0; 
}
void count_load_goods(int boat_id){//检查船上装了多少的货，这个位置需要机器人在pull的时候往berth的goods_num加1 
	int goods_in_berth = berth[boat[boat_id].pos].goods_num; //港口中货物数量 
	if(goods_in_berth <= berth[boat[boat_id].pos].loading_speed){//若装货速度大于等于货物数量 
		if(boat_capacity >= boat[boat_id].num + goods_in_berth){//能一次性全部装上船 
			boat[boat_id].num += goods_in_berth;
			berth[boat[boat_id].pos].goods_num = 0; 
		}else{//不能全部装上船 
			berth[boat[boat_id].pos].goods_num -= boat_capacity - boat[boat_id].num; 
			boat[boat_id].num = boat_capacity;
		}
	}else{//装货速度小于货物数量 
		if(boat_capacity >= boat[boat_id].num + berth[boat[boat_id].pos].loading_speed){//能一次性装一次运载极限的货物装上船 
			boat[boat_id].num += berth[boat[boat_id].pos].loading_speed;
			berth[boat[boat_id].pos].goods_num -= berth[boat[boat_id].pos].loading_speed; 
		}else{//船中装不下一次运载量的货 
			berth[boat[boat_id].pos].goods_num -= boat_capacity - boat[boat_id].num;
			boat[boat_id].num = boat_capacity;
		}
	}
}
int go_permisson[11];
void Boat_action(){
	for(int boat_id = 0; boat_id < 5; boat_id ++){
		if(boat[boat_id].status == 0) continue;//运输中的船直接不管 
		if(go_permisson[boat_id] == 1) {//因为装货最后处理，所以做了一点特殊处理，在下一帧开始的时候检查船只是否可以从港口出发 
			berth[boat[boat_id].pos].busy = 0;
			go_permisson[boat_id] = 0;
			printf("go %d", boat_id);
			continue;//如果这条船可以出发，切换到下一条船 
		}
		if(boat[boat_id].status == 1){
			if(boat[boat_id].pos != -1){
				count_load_goods(boat_id); 
				go_permisson[boat_id] = check_go(boat_id);
			}else{
				find_berth(boat_id);
			}
		}
		if(boat[boat_id].pos == 2){//在泊位外等待时看有没有空闲的其他泊位可以去 
			find_berth(boat_id);
		}
	}
} 
int main()
{
    Init();
    for(int zhen = 1; zhen <= 15000; zhen ++)
    {
        int id = Input();
        //每个货物只能停留1000帧，如果在1000帧之内无法抵达，放弃该选项
		if(zhen == 1) Boat_Init();
		else Boat_action();//轮船的所有行动 
        for(int i = 0; i < robot_num; i ++)
            printf("move %d %d\n", i, rand() % 4);
        puts("OK");
        fflush(stdout);
    }

    return 0;
}