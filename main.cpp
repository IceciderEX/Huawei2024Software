#include <bits/stdc++.h>
#include <iostream>
using namespace std;

const int n = 200;  //地图大小n*n
const int robot_num = 10; //机器人数量
const int berth_num = 10; //泊位数量
const int N = 210; 
const int goods_time = 1000; //货物存在时间为1000帧

//机器人
struct Robot
{
    int x, y, goods; //机器人坐标为(x, y), goods == 1表示携带物品， 否则未携带
    int status; //0表示恢复状态， 1表示正常运行状态
    int mbx, mby;
    Robot() {}
    Robot(int startX, int startY) {
        x = startX;
        y = startY;
    }
}robot[robot_num + 10];

//泊位
struct Berth
{
    int x, y; //泊位左上角坐标为(x, y)
    int transport_time; //泊位到虚拟点的时间
    int loading_speed; //每帧可以装载的物品数
    Berth(){}
    Berth(int x, int y, int transport_time, int loading_speed) {
        this -> x = x;
        this -> y = y;
        this -> transport_time = transport_time;
        this -> loading_speed = loading_speed;
    }
}berth[berth_num + 10];

//轮船
struct Boat
{
    int num;
    int pos;    //目标泊位，-1表示虚拟点
    int status; //0表示移动中，1表示装货状态或运输完成状态，2表示泊位外等待状态
}boat[10];

int money, boat_capacity, id; //当前金钱数，轮船容积，帧号

char MAP[N][N]; //地图

//物品
struct Goods{
	int x,y;    //物品位置
    int val;    //物品价值
    int left_time= goods_time;  //剩余存在时间
};

set<Goods>gds;
void Init()
{
    //读入地图
    for(int i = 1; i <= n; i ++)
        scanf("%s", MAP[i] + 1);

    //初始化泊位信息
    for(int i = 0; i < berth_num; i ++)
    {
        int id;
        scanf("%d", &id);
        scanf("%d%d%d%d", &berth[id].x, &berth[id].y, &berth[id].transport_time, &berth[id].loading_speed);
    }

    //轮船容积
    scanf("%d", &boat_capacity);

    char okk[100];
    scanf("%s", okk);
    printf("OK\n");
    fflush(stdout);
}

int Input()
{
    scanf("%d%d", &id, &money);

    //新增货物
    int num;
    scanf("%d", &num);

    for(int i = 1; i <= num; i ++)
    {
        Goods *tmp=(Goods*)malloc(sizeof(Goods));
        scanf("%d%d%d", &tmp->x, &tmp->y, &tmp->val);
    }

    //机器人信息
    for(int i = 0; i < robot_num; i ++)
    {
        scanf("%d%d%d%d", &robot[i].goods, &robot[i].x, &robot[i].y, &robot[i].status);
    }

    //轮船信息
    for(int i = 0; i < 5; i ++)
        scanf("%d%d\n", &boat[i].status, &boat[i].pos);
        
    char okk[100];
    scanf("%s", okk);
    return id;
}

int main()
{
    Init();
    for(int zhen = 1; zhen <= 15000; zhen ++)
    {
        int id = Input();
        
        for(int i = 0; i < robot_num; i ++)
            printf("move %d %d\n", i, rand() % 4);
        puts("OK");
        fflush(stdout);
    }

    return 0;
}
