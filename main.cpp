#include <bits/stdc++.h>

using namespace std;

const int n = 200;  //地图大小n*n
const int robot_num = 10;  //机器人数量
const int berth_num = 10;  //泊位数量
const int N = 210; 
const int goods_time = 1000; //货物存在时间为1000帧
const int INF = 1e9 + 10; //无穷大
const int dx[] = {1, -1, 0, 0};
const int dy[] = {0, 0, 1, -1}; //方向数组

//机器人
struct Robot
{
    int x, y, goods; //机器人坐标为(x, y), goods == 1表示携带物品， 否则未携带
    int status; //0表示恢复状态， 1表示正常运行状态
    int target_gds = -1;  //目标物品
    int target_berth = -1;  //目标泊位
    int mbx, mby; //预计移动后下一个位置， 后续可用于避免碰撞
    int pathid = 0; //下一步路径索引
    vector<int> path;
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
    int targeted; //被哪个机器人锁定
};

vector<Goods> gds;
int dis[N + 1][N + 1];
pair<int, int> prev_step[N + 1][N + 1];

//判断(x,y)是否是机器人下一个可以去往的点
bool IsOkRobotPath(int x, int y)
{
    if( x < 0  || x > 199 || y < 0 || y > 199 ) return false;
    for(int i = 0 ; i < 10 ; i ++){
        if(x == robot[i].x && x == robot[i].y == y) return false;
    }
    if(MAP[x][y] == '#' || MAP[x][y] == 'B' || MAP[x][y] == '*') return false;
    return true;
}


//机器人单步移动指令
void robot_move(int robotid)
{
    if(robot[robotid].pathid<robot[robotid].path.size()){
        printf("move %d %d\n",robotid, robot[robotid].path[robot[robotid].pathid]);
        robot[robotid].pathid++;
        
        /*加上mbx,mby的计算*/
    }
}

// BFS寻找从(startX, startY)到最近货物的最短路径
pair<vector<int>, int> FindPath(int startX, int startY) {
    queue<pair<int, int>> q;
    vector<int> path;
    int target_index = -1;

    for (int i = 0; i <= N; ++i) {
        for (int j = 0; j <= N; ++j) {
            dis[i][j] = INF;
            prev_step[i][j] = {-1, -1};
        }
    }

    q.push({startX, startY});
    dis[startX][startY] = 0;

    while (!q.empty()) {
        auto [x, y] = q.front();
        q.pop();

        for (int i = 0; i < gds.size(); ++i) {
            if (x == gds[i].x && y == gds[i].y) {
                // 找到最近的货物，回溯路径
                while (x != startX || y != startY) {
                    int pathtmp = -1;
                    if(x == 1 && y == 0) pathtmp = 0;
                    else if(x == -1 && y == 0) pathtmp = 1;
                    else if(x == 0 && y == -1) pathtmp = 2;
                    else if(x == 0 && y == 1) pathtmp = 3;
                    path.push_back(pathtmp);
                    tie(x, y) = prev_step[x][y];
                }
                reverse(path.begin(), path.end());
                target_index = i;
                return {path, target_index};
            }
        }

        for (int i = 0; i < 4; ++i) {
            int nx = x + dx[i];
            int ny = y + dy[i];
            if (IsOkRobotPath(nx,ny)) {
                q.push({nx, ny});
                dis[nx][ny] = dis[x][y] + 1;
                prev_step[nx][ny] = {x, y};
            }
        }
    }

    return {path, target_index}; // 如果没有找到路径，返回空路径和-1
}

//机器人运动
void Robot_Control(int robotid)
{
    /*待完成
    if(robot[robotid].status == 0){
        Robot_recover(robotid); //机器人恢复,待码
        break;
    }

    if(robot[robotid].goods == 1){
        Robot_to_Berth(robotid); //前往泊位，待码
        break;
    }*/

    if(robot[robotid].target_gds == -1){
        int nowx = robot[robotid].x;
        int nowy = robot[robotid].y;
        pair<vector<int>, int> tmp = FindPath(nowx, nowy);
        robot[robotid].path = tmp.first; 
        robot[robotid].target_gds = tmp.second;
        robot_move(robotid);
        //.....
    }
}
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
        gds.push_back(*tmp);
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
        
        for(int i = 0; i < robot_num; i ++){
            Robot_Control(robot_num);
            printf("move %d %d\n", i, rand() % 4);
        }
        puts("OK");
        fflush(stdout);
    }

    return 0;
}
