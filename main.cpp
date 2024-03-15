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
    vector<int> path;//机器人路径
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
    int left_time;  //剩余存在时间
    int targeted; //被哪个机器人锁定，没有被锁定时为-1
    Goods(int x,int y, int val)
    {
        this->x = x;
        this->y = y;
        this->val = val;
        this->left_time = goods_time;
        this->targeted = -1;
    }
};

map<int,Goods> gds;
int dis[N + 1][N + 1];
pair<int, int> prev_step[N + 1][N + 1];

//货物存在时间减少
void dec_gdstime()
{
    for(auto & g :gds)
    {
        g.second.left_time -= 1;
        if(g.second.left_time <= 0) gds.erase(g.first);
    }
}
//判断(x,y)是否是机器人下一个可以去往的点
bool IsOkRobotPath(int x, int y)
{
    if( x < 1  || x > 200 || y < 1 || y > 200 ) return false;
    for(int i = 0 ; i < 10 ; i ++){
        if(x == robot[i].x && x == robot[i].y == y) return false;
    }
    if(MAP[x][y] == '#' || MAP[x][y] == 'B' || MAP[x][y] == '*') return false;
    return true;
}


//机器人单步移动指令
void robot_move(int robotid)
{
    if(robot[robotid].path.size() == 0) return;
    if(robot[robotid].pathid<robot[robotid].path.size()){
        printf("move %d %d\n",robotid, robot[robotid].path[robot[robotid].pathid]);
        robot[robotid].pathid++;
        
        /*加上mbx,mby的计算*/
    }
}

// BFS寻找从(startX, startY)到最优货物的最短路径
pair<vector<int>, int> FindPath(int sX, int sY) {
    queue<pair<int, int>> q;
    vector<int> path;
    int target_index = -1;
    double max_priority = -1.0;  // 初始化最大优先级为负数

    for (int i = 0; i <= N; ++i) {
        for (int j = 0; j <= N; ++j) {
            dis[i][j] = INF;
            prev_step[i][j] = make_pair(-1, -1);
        }
    }

    q.push(make_pair(sX, sY));
    dis[sX][sY] = 0;

    while (!q.empty()) {
        pair<int, int> current = q.front();
        q.pop();

        int x = current.first;
        int y = current.second;

        if (dis[x][y] >= goods_time) {
            continue;  // 如果当前路径已经超过了货物的最大存在时间，则跳过
        }

        for (auto& g_pair : gds) {
            Goods& g = g_pair.second;
            if (g.targeted != -1 || g.left_time <= dis[x][y]) continue;  // 如果货物已被其他机器人锁定或者无法在消失前到达，则跳过
            if (x == g.x && y == g.y) {
                double priority = double(g.val) / dis[x][y];  // 计算优先级
                if (priority > max_priority) {
                    max_priority = priority;
                    target_index = g_pair.first;
                    // 回溯路径
                    path.clear();
                    int px = x;
                    int py = y;
                    while (px != sX || py != sY) {
                        int dir = -1;
                        int prev_x = prev_step[px][py].first;
                        int prev_y = prev_step[px][py].second;
                        if (prev_x - px == 1) dir = 0;
                        else if (px - prev_x == 1) dir = 1;
                        else if (prev_y - py == 1) dir = 2;
                        else if (py - prev_y == 1) dir = 3;
                        path.push_back(dir);
                        px = prev_x;
                        py = prev_y;
                    }
                    reverse(path.begin(), path.end());
                }
            }
        }

        for (int i = 0; i < 4; ++i) {
            int nx = x + dx[i];
            int ny = y + dy[i];
            if (IsOkRobotPath(nx, ny) && dis[nx][ny] == INF) {
                q.push(make_pair(nx, ny));
                dis[nx][ny] = dis[x][y] + 1;
                prev_step[nx][ny] = make_pair(x, y);
            }
        }
    }

    return make_pair(path, target_index);  // 如果没有找到路径，返回空路径和-1
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
        //如果没有目标，寻找货物目标
        int nowx = robot[robotid].x;
        int nowy = robot[robotid].y;
        pair<vector<int>, int> tmp = FindPath(nowx, nowy);
        if(tmp.second != -1){
            robot[robotid].path = tmp.first; 
            robot[robotid].target_gds = tmp.second;
            gds[robot[robotid].target_gds].targeted = robotid;
        }
        //.....
    }

    if(gds[robot[robotid].target_gds].x == robot[robotid].x && gds[robot[robotid].target_gds].y == robot[robotid].y) 
    {
        if(robot[robotid].goods == 0){
            gds.erase(robot[robotid].target_gds);
            robot[robotid].target_gds = -1;
            robot[robotid].pathid = 0;
            robot[robotid].path.clear();
            printf("get %d\n", robotid);
        }
    }   //若处于目标货物位置

    robot_move(robotid);
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

int Input(int zhen)
{
    scanf("%d%d", &id, &money);

    //新增货物
    int num;
    scanf("%d", &num);

    for(int i = 1; i <= num; i ++)
    {
        int x, y, val;
        scanf("%d%d%d", &x, &y, &val);
        Goods new_goods{x, y, val};  
        gds[zhen*10 + num] = new_goods;
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
        int id = Input(zhen);
        
        dec_gdstime();
        for(int i = 0; i < robot_num; i ++){
            Robot_Control(i);
            printf("%d%d\n",robot[i].goods, robot[i].target_gds);
            //printf("move %d %d\n", i, rand() % 4);
        }
        puts("OK");
        fflush(stdout);
    }

    return 0;
}
