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
};

map<int,Goods> gds;
int dis[N][N];
pair<int, int> prev_step[N][N];

//货物存在时间减少
void dec_gdstime()
{
	vector<int> tmp;
    for(auto & g :gds)
    {
        g.second.left_time -= 1;
        if(g.second.left_time <= 0) tmp.push_back(g.first);
    }
    for(auto id : tmp){
        if(gds[id].targeted == 1){
            for(int i = 0;i<robot_num;i++)
                if(robot[i].target_gds == id) robot[i].target_gds == -1;
        }
        gds.erase(id);
    }
}
//判断(x,y)是否是机器人下一个可以去往的点
bool IsOkRobotPath(int robotid, int x, int y) {
    if (x < 0 || x > 199 || y < 0 || y >199) return false;
    for(int i = 0 ; i < robot_num ; i++){
        if(i!=robotid&&robot[i].x==robot[robotid].x&&robot[i].y==robot[robotid].y) return false;
    }
    if (MAP[x+1][y+1] == '#' || MAP[x+1][y+1] == '*' ) return false;
    return true;
}



//机器人单步移动指令
void robot_move(int robotid)
{
    if(robot[robotid].pathid<robot[robotid].path.size()){
    	printf("move %d %d\n",robotid, robot[robotid].path[robot[robotid].pathid]);
    	robot[robotid].pathid++;
    }
}

// BFS寻找从(startX, startY)到最优货物的最短路径
void FindPath(int robotid, int sX, int sY) {
	
    queue<pair<int, int>> q;
    double max_priority = -1.0;
	//printf("Robotid:%d queue: %d",robotid ,q.empty());
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
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

        for (auto& g_pair : gds) {
            Goods& g = g_pair.second;
            
            if (g.targeted == 1 || g.left_time <= dis[x][y]){
            	continue;
            }
            if (x == g.x && y == g.y) {
                //double priority = (double(g.val))/( (double)dis[x][y]*(double)dis[x][y]*(double)dis[x][y]); //40000多
                double priority = (double(g.val))/exp((double)dis[x][y]);
                if (priority > max_priority) {
                    max_priority = priority;
                    robot[robotid].target_gds = g_pair.first;
                    // 回溯路径
                    robot[robotid].path.clear();
                    int px = x;
                    int py = y;
                    while (px != sX || py != sY) {
                        int dir = -1;
                        int prev_x = prev_step[px][py].first;
                        int prev_y = prev_step[px][py].second;
                        if (prev_x - px == 1) dir = 2;
                        else if (px - prev_x == 1) dir = 3;
                        else if (prev_y - py == 1) dir = 1;
                        else if (py - prev_y == 1) dir = 0;
                        robot[robotid].path.push_back(dir);
                        px = prev_x;
                        py = prev_y;
                    }
                    reverse(robot[robotid].path.begin(), robot[robotid].path.end());
                    //printf("Robotid: %d path:",robotid);
                    //for(auto pp : robot[robotid].path) cout<<pp<<" ";
                    //printf("\n");
                }
            }
        }
        
        for (int i = 0; i < 4; ++i) {
            int nx = x + dx[i];
            int ny = y + dy[i];
            if (IsOkRobotPath(robotid, nx, ny) && dis[nx][ny] == INF) {
                q.push(make_pair(nx, ny));
                dis[nx][ny] = dis[x][y] + 1;
                prev_step[nx][ny] = make_pair(x, y);
            }
        }
    }

}

void Robot_to_Berth(int robotid)
{
    int sX = robot[robotid].x;
    int sY = robot[robotid].y;
    queue<pair<int, int>> q;
    double max_priority = -1.0;
	//printf("Robotid:%d queue: %d",robotid ,q.empty());
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
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

        //for (int i = 0 ; i < berth_num ; i++) {
            int bx = berth[robotid].x;
            int by = berth[robotid].y;
            if(x>=bx&&x<=bx+3&&y>=by&&y<=by+3){
            //if (x==bx + robotid/3&&y == by +robotid%3) {
                double priority = (double)berth[robotid].loading_speed/((double)dis[x][y]*(double)dis[x][y]);
                if (priority > max_priority) {
                    max_priority = priority;
                    robot[robotid].target_berth = robotid;
                    // 回溯路径
                    robot[robotid].path.clear();
                    int px = x;
                    int py = y;
                    while (px != sX || py != sY) {
                        int dir = -1;
                        int prev_x = prev_step[px][py].first;
                        int prev_y = prev_step[px][py].second;
                        if (prev_x - px == 1) dir = 2;
                        else if (px - prev_x == 1) dir = 3;
                        else if (prev_y - py == 1) dir = 1;
                        else if (py - prev_y == 1) dir = 0;
                        robot[robotid].path.push_back(dir);
                        px = prev_x;
                        py = prev_y;
                    }
                    reverse(robot[robotid].path.begin(), robot[robotid].path.end());
                    //printf("Robotid: %d path:",robotid);
                    //for(auto pp : robot[robotid].path) cout<<pp<<" ";
                    //printf("\n");
                }
            }
        //}
        
        for (int i = 0; i < 4; ++i) {
            int nx = x + dx[i];
            int ny = y + dy[i];
            if (IsOkRobotPath(robotid, nx, ny) && dis[nx][ny] == INF) {
                q.push(make_pair(nx, ny));
                dis[nx][ny] = dis[x][y] + 1;
                prev_step[nx][ny] = make_pair(x, y);
            }
        }
    }
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
			printf("go %d\n", boat_id);
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
		if(boat[boat_id].status == 2){//在泊位外等待时看有没有空闲的其他泊位可以去 
			find_berth(boat_id);
		}
	}
} 

//机器人运动
void Robot_Control(int robotid)
{
    /*待完成
    if(robot[robotid].status == 0){
        Robot_recover(robotid); //机器人恢复,待码
        break;
    }*/
    if(robot[robotid].status == 0){
        if(robot[robotid].goods == 0){
            robot[robotid].target_gds = -1;
            robot[robotid].path.clear();
            robot[robotid].pathid = 0;
        }
        else{
            robot[robotid].target_berth = -1;
            robot[robotid].path.clear();
            robot[robotid].pathid = 0;
        }
        int x = robot[robotid].x;
        int y = robot[robotid].y;
        int nx,ny;
        int step = 0;
        bool f = false;
        while(!f){
            step = rand()%4;
            if(step == 0) nx = x, ny = y + 1;
            else if(step== 1) nx = x , ny = y - 1;
            else if(step==2) nx = x - 1 , ny = y;
            else nx = x + 1 , ny = y;
            if(IsOkRobotPath(robotid,nx,ny)) f=true; 
        }
        printf("move %d %d\n",robotid,step);
    }
    
    if(robot[robotid].target_gds == -1&&robot[robotid].goods == 0){
        //如果没有目标，寻找货物目标
        int nowx = robot[robotid].x;
        int nowy = robot[robotid].y;
        FindPath(robotid,nowx, nowy);
        if(robot[robotid].target_gds!= -1){
            gds[robot[robotid].target_gds].targeted = 1;
        }
        //.....
    }

    if(robot[robotid].status == 1 ) robot_move(robotid);

    if(robot[robotid].target_gds != -1&&gds[robot[robotid].target_gds].x == robot[robotid].x && gds[robot[robotid].target_gds].y == robot[robotid].y) 
    {
        if(robot[robotid].goods == 0){
            robot[robotid].goods == 1;
            gds.erase(robot[robotid].target_gds);
            robot[robotid].target_gds = -1;
            robot[robotid].pathid = 0;
            robot[robotid].path.clear();
            printf("get %d\n", robotid);
        }
    }   //若处于目标货物位置
    
    if(robot[robotid].goods == 1){
        if(robot[robotid].target_berth == -1){
            Robot_to_Berth(robotid); //前往泊位
        }

        int bx = berth[robot[robotid].target_berth].x;
        int by = berth[robot[robotid].target_berth].y;

        if(robot[robotid].x>=bx&&robot[robotid].x<=bx+3&&robot[robotid].y>=by&&robot[robotid].y<=by+3){
        //if(robot[robotid].x==bx + robotid/3&&robot[robotid].y==by+robotid%3){
            printf("pull %d\n",robotid);
            berth[robot[robotid].target_berth].goods_num ++;
            robot[robotid].target_berth = -1;
            robot[robotid].goods = 0;
            robot[robotid].pathid = 0;
            robot[robotid].path.clear();
        }
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
        int x, y, val;
        scanf("%d%d%d", &x, &y, &val);
        Goods new_goods{x, y, val,goods_time,0};  
        gds[id*1000 + i] = new_goods;
    }

    //机器人信息
    for (int i = 0; i < robot_num; i++)
    {
        int sts;
        scanf("%d%d%d%d", &robot[i].goods, &robot[i].x, &robot[i].y, &sts);
        robot[i].status = sts;
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
//    freopen("output.txt", "r" ,stdin);
//    freopen("test1.txt",  "w" ,stdout);
    Init();
    Boat_Init();
    for(int zhen = 1; zhen <= 15000; zhen ++)
    {
        int id = Input();
        //for(auto g : gds) cout<<g.first<<" ";
        for(int i = 0; i < robot_num; i ++){
            Robot_Control(i);
            //printf("move %d %d\n", i, rand() % 4);
        }
        Boat_action();
        puts("OK");
        fflush(stdout);
        dec_gdstime();
    }

    return 0;
}
