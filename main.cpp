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
    int val; //当前携带货物的价值
    vector<pair<int,int>> predpath;
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
    int goods_val; //泊位中货物的价值
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
    int stay_time; //在泊位内停留的时间 
    int arrive; 
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
queue<int> gds_in_berth[berth_num + 10];

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
bool IsOkRobotPath(int robotid, int x, int y,int dis) {
    if (x < 0 || x > 199 || y < 0 || y >199) return false;
    for(int i = 0 ; i < robot_num ; i++){
        if(i==robotid||robot[robotid].pathid+dis>=robot[robotid].path.size()) continue;
        int rx = robot[i].predpath[robot[robotid].pathid+dis-1].first;
        int ry = robot[i].predpath[robot[robotid].pathid+dis-1].second;
        if(rx==x&&ry==y) return false;
    }
    for(int i = 0 ; i < robot_num ; i ++){
        if(i != robotid && robot[i].x == x&&robot[i].y==y) return false;
    }
    if (MAP[x+1][y+1] == '#' || MAP[x+1][y+1] == '*' ) return false;
    return true;
}

void Findmbgds(int robotid)
{
    double max_pro = -1.0;
    for(auto g : gds)
    {
        int d = abs(robot[robotid].x - g.second.x) + abs(robot[robotid].y - g.second.y);
        if (g.second.targeted == 1 || g.second.left_time <= d){
            	continue;
        }
        double pro = (double)g.second.val/((double)d*(double)d*(double)d); 
        if(pro > max_pro){
            max_pro = pro;
            robot[robotid].target_gds = g.first;
        }
    }
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

        //for (auto& g_pair : gds) {
            //Goods& g = g_pair.second;
            
            //if (g.targeted == 1 || g.left_time <= dis[x][y]){
            	//continue;
            //}
            int gx = gds[robot[robotid].target_gds].x;
            int gy = gds[robot[robotid].target_gds].y;
            if (x == gx && y == gy) {
                //double priority =(double)g.val/((double)dis[x][y]*(double)dis[x][y]*(double)dis[x][y]); 
                //double priority = (double)g.val/(2.0*dis[x][y]*dis[x][y]); 
                //if (priority >= max_priority) {
                    //max_priority = priority;
                    //robot[robotid].target_gds = g_pair.first;
                    // 回溯路径
                    robot[robotid].path.clear();
                    robot[robotid].predpath.clear();
                    robot[robotid].pathid = 0;
                    int px = x;
                    int py = y;
                    while (px != sX || py != sY) {
                        robot[robotid].predpath.push_back(make_pair(px,py));
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
                    reverse(robot[robotid].predpath.begin(), robot[robotid].predpath.end());
                    //printf("Robotid: %d path:",robotid);
                    //for(auto pp : robot[robotid].path) cout<<pp<<" ";
                    //printf("\n");
                }
            //}
        //}
        
        for (int i = 0; i < 4; ++i) {
            int nx = x + dx[i];
            int ny = y + dy[i];
            if (IsOkRobotPath(robotid, nx, ny,dis[x][y]+1) && dis[nx][ny] == INF) {
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

        for (int i = 0 ; i < berth_num ; i++) {
            int bx = berth[i].x;
            int by = berth[i].y;
            if(x>=bx&&x<=bx+3&&y>=by&&y<=by+3){
            //if (x==bx + robotid/3&&y == by +robotid%3) {
                double priority = (double)berth[robotid].loading_speed / ((double)dis[x][y]*(double)dis[x][y]);
                    if (priority > max_priority || i == robotid) {
                        max_priority = priority;
                        robot[robotid].target_berth = i;
                        // 回溯路径
                        robot[robotid].path.clear();
                        robot[robotid].predpath.clear();
                        robot[robotid].pathid = 0;
                        int px = x;
                        int py = y;
                        while (px != sX || py != sY) {
                            robot[robotid].predpath.push_back(make_pair(px,py));
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
                        reverse(robot[robotid].predpath.begin(), robot[robotid].predpath.end());
                    }
            }
        }
        
        for (int i = 0; i < 4; ++i) {
            int nx = x + dx[i];
            int ny = y + dy[i];
            if (IsOkRobotPath(robotid, nx, ny,dis[x][y]+1) && dis[nx][ny] == INF) {
                q.push(make_pair(nx, ny));
                dis[nx][ny] = dis[x][y] + 1;
                prev_step[nx][ny] = make_pair(x, y);
            }
        }
    }
}

int find_berth_vir(int boat_id, int time_now){//从虚拟点出发找泊位 
	/*以下为按照权重进行港口的选取*/ 
	//printf("%d ",boat_id); 
	int wgt = 300;
	int finall = 99;
	for(int berth_id = 0; berth_id < 10; berth_id++){
        // f << time_now << " " << " berth_id:" << berth_id << " goods num: " << berth[berth_id].goods_num << " goods val: " << berth[berth_id].goods_val << " busy " << berth[berth_id].busy;
        if(berth_id == boat[boat_id].pos) continue;
        if(time_now + berth[berth_id].transport_time + 550 <= 15000){
            if(berth[berth_id].busy == 0){
                int tmp = berth[berth_id].goods_val;
                if(tmp > wgt){
                    wgt = tmp;
                    finall = berth_id;
                }
            }
            else if(berth[berth_id].busy == 1){
                int tmp = berth[berth_id].goods_val - 3000;
                if(tmp > wgt){
                    wgt = tmp;
                    finall = berth_id;
                }
            }
        }
	}
	if(finall == 99) return -1;//实在找不到泊位就先不动 
	printf("ship %d %d\n", boat_id, finall);//让船往前往耗时最少的泊位 
	berth[finall].busy = 1;//将该泊位设置为忙碌状态
	return finall; 
}

void Boat_Init(){ 
	for(int boat_id = 0; boat_id < 5; boat_id++){
		boat[boat_id].pos = boat_id * 2;
        printf("ship %d %d\n", boat_id, boat[boat_id].pos);
	}
}

double check_boat_goods(int boat_id){ //若装满了货物，让轮船出发 
	return (double) boat[boat_id].num / boat_capacity;
}

int check_go(int boat_id){//若装满了货物，让轮船出发 
	if(boat[boat_id].num >= boat_capacity * 0.9) return 1;
    else return 0;
}

void count_load_goods(int boat_id){//检查船上装了多少的货，以及更新港口中剩余货物的价值 
	int berthid = boat[boat_id].pos;
	int goods_in_berth = berth[berthid].goods_num; //港口中货物数量 
	if(goods_in_berth <= berth[berthid].loading_speed){//若装货速度大于等于货物数量 
		if(boat_capacity >= boat[boat_id].num + goods_in_berth){//能一次性全部装上船 
			boat[boat_id].num += goods_in_berth;
			berth[berthid].goods_num = 0; 
			for(int i = 1 ;i <= goods_in_berth ;i ++){
				berth[berthid].goods_val -= gds_in_berth[berthid].front();
				gds_in_berth[berthid].pop();
			}
		}else{//不能全部装上船 
			berth[berthid].goods_num -= boat_capacity - boat[boat_id].num; 
			boat[boat_id].num = boat_capacity;
			for(int i = 1 ;i <= boat_capacity - boat[boat_id].num ;i ++){
				berth[berthid].goods_val -= gds_in_berth[berthid].front();
				gds_in_berth[berthid].pop();
			}
		}
	}
    else{//装货速度小于货物数量 
		if(boat_capacity >= boat[boat_id].num + berth[berthid].loading_speed){//能一次性装一次运载极限的货物装上船 
			boat[boat_id].num += berth[berthid].loading_speed;
			berth[berthid].goods_num -= berth[berthid].loading_speed;
			for(int i = 1 ;i <= berth[berthid].loading_speed ;i ++){
				berth[berthid].goods_val -= gds_in_berth[berthid].front();
				gds_in_berth[berthid].pop();
			}
		}else{//船中装不下一次运载量的货 
			berth[berthid].goods_num -= boat_capacity - boat[boat_id].num;
			boat[boat_id].num = boat_capacity;
			for(int i = 1 ;i <= boat_capacity - boat[boat_id].num ;i ++){
				berth[berthid].goods_val -= gds_in_berth[berthid].front();
				gds_in_berth[berthid].pop();
			}
		}
	}
}

int go_permisson[11];
void Boat_action(int time_now){//传入现在是第几帧 
	//printf("第%d帧：\n", time_now);
    // fstream f;
    // f.open("data.txt", ios::out | ios::app);
	for(int boat_id = 0; boat_id < 5; boat_id++){
        // if(time_now % 100 == 0) f << time_now << " boat_id: " << boat_id << " status:" << boat[boat_id].status << " num: " << boat[boat_id].num << " pos " << boat[boat_id].pos << endl;
		if(boat[boat_id].status == 0) continue;//运输中的船直接不管  
		if(go_permisson[boat_id] == 1) {//因为装货最后处理，所以做了一点特殊处理，在下一帧开始的时候检查船只是否可以从港口出发 
			berth[boat[boat_id].pos].busy = 0;
			go_permisson[boat_id] = 0;
			boat[boat_id].num = 0;
			boat[boat_id].stay_time = 0;
			printf("go %d\n", boat_id);
			continue;//如果这条船可以出发，切换到下一条船 
		}
		if(boat[boat_id].status == 1){
			if(boat[boat_id].pos != -1){
                if(time_now + berth[boat[boat_id].pos].transport_time + 30 >= 15000){ // 剩余时间到达15000
                    berth[boat[boat_id].pos].busy = 0;
                    go_permisson[boat_id] = 0;
                    boat[boat_id].num = 0;
                    boat[boat_id].stay_time = 0;
                    printf("go %d\n", boat_id);
                    continue;//如果这条船可以出发，切换到下一条船 
                }
				if(boat[boat_id].stay_time == 0) {
                    boat[boat_id].stay_time++;
                    boat[boat_id].arrive = time_now;
                }
				boat[boat_id].stay_time += time_now - boat[boat_id].arrive;
				boat[boat_id].arrive = time_now;
                if(boat_id % 2 == 0 && check_boat_goods(boat_id) >= 0.98) go_permisson[boat_id] = 1;
                else if(boat_id % 2 == 1 && check_boat_goods(boat_id) >= 0.75) go_permisson[boat_id] = 1;
                // f << "frame: " << time_now << " Arrive: " << boat[boat_id].arrive << " stay_time: " << boat[boat_id].stay_time << endl;
				if(boat[boat_id].stay_time >= 30 || berth[boat[boat_id].pos].goods_num <= 0){
                    int before_berth = boat[boat_id].pos;
                    int res = find_berth_vir(boat_id, time_now);
                    if(res != -1){
                        berth[before_berth].busy = 0;
                        boat[boat_id].stay_time = 0;
                    }
				}
                count_load_goods(boat_id);
			}
            else{ // 在虚拟点
				find_berth_vir(boat_id, time_now);
				continue;
			}
		}
		else if(boat[boat_id].status == 2){ //在泊位外等待时看有没有空闲的其他泊位可以去
			int res = find_berth_vir(boat_id, time_now);
            if(res == -1){
                if(time_now + berth[boat[boat_id].pos].transport_time + 75 >= 15000){ // 剩余时间到达15000
                    berth[boat[boat_id].pos].busy = 0;
                    go_permisson[boat_id] = 0;
                    boat[boat_id].num = 0;
                    boat[boat_id].stay_time = 0;
                    printf("go %d\n", boat_id);
                    continue;//如果这条船可以出发，切换到下一条船 
                }
            }
		}
	}
    //f.close();
} 

//机器人运动
void Robot_Control(int robotid,int zhen)
{

    if(robot[robotid].status == 0){
        if(robot[robotid].goods == 0&&robot[robotid].target_gds!=-1){
            robot[robotid].target_gds = -1;
            robot[robotid].path.clear();
            robot[robotid].predpath.clear();
            robot[robotid].pathid = 0;
        }
        else if(robot[robotid].goods == 1&&robot[robotid].target_berth!=-1){
            robot[robotid].target_berth = -1;
            robot[robotid].path.clear();
            robot[robotid].predpath.clear();
            robot[robotid].pathid = 0;
        }    
    }

    if(robot[robotid].status==1&&robot[robotid].target_gds == -1&&robot[robotid].goods == 0){
        //如果没有目标，寻找货物目标
        int nowx = robot[robotid].x;
        int nowy = robot[robotid].y;
        Findmbgds(robotid);
        if(robot[robotid].target_gds!= -1){
            FindPath(robotid,nowx, nowy);
            gds[robot[robotid].target_gds].targeted = 1;
        }
        //.....
    }

    if(robot[robotid].status == 1 ) robot_move(robotid);

    if(robot[robotid].status==1&&robot[robotid].target_gds != -1&&gds[robot[robotid].target_gds].x == robot[robotid].x && gds[robot[robotid].target_gds].y == robot[robotid].y) 
    {
        if(gds.find(robot[robotid].target_gds) == gds.end()){
            
            robot[robotid].target_gds = -1;
            robot[robotid].path.clear();
            robot[robotid].predpath.clear();
            robot[robotid].pathid = 0;
            int nowx = robot[robotid].x;
            int nowy = robot[robotid].y;
            FindPath(robotid,nowx, nowy);
            if(robot[robotid].target_gds!= -1){
                gds[robot[robotid].target_gds].targeted = 1;
            }
        }

        if(robot[robotid].goods == 0){
            robot[robotid].goods == 1;
            robot[robotid].val = gds[robot[robotid].target_gds].val;
            gds.erase(robot[robotid].target_gds);
            robot[robotid].target_gds = -1;
            robot[robotid].pathid = 0;
            robot[robotid].path.clear();
            robot[robotid].predpath.clear();
            printf("get %d\n", robotid);
        }
    }   //若处于目标货物位置
    
    if(robot[robotid].status==1&&robot[robotid].goods == 1){
        if(robot[robotid].target_berth == -1){
            Robot_to_Berth(robotid); //前往泊位
        }

        int bx = berth[robot[robotid].target_berth].x;
        int by = berth[robot[robotid].target_berth].y;

        if(robot[robotid].x>=bx&&robot[robotid].x<=bx+3&&robot[robotid].y>=by&&robot[robotid].y<=by+3){
            printf("pull %d\n",robotid);
            berth[robot[robotid].target_berth].goods_val += robot[robotid].val;
            gds_in_berth[robot[robotid].target_berth].push(robot[robotid].val);
            berth[robot[robotid].target_berth].goods_num++;
            robot[robotid].target_berth = -1;
            robot[robotid].goods = 0;
            robot[robotid].pathid = 0;
            robot[robotid].path.clear();
            robot[robotid].predpath.clear();
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
    for(int zhen = 1; zhen <= 15000; zhen ++)
    {
        int id = Input();
        for(int i = 0; i < robot_num; i ++){
            Robot_Control(i, id);
        }
        if(id == 1) Boat_Init();
        if(id >= 20) Boat_action(id);
        puts("OK");
        fflush(stdout);
        dec_gdstime();
    }

    return 0;
}
