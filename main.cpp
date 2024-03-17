#include <bits/stdc++.h>

using namespace std;

const int n = 200;  //鍦板浘澶у皬n*n
const int robot_num = 10;  //鏈哄櫒浜烘暟閲?
const int berth_num = 10;  //娉婁綅鏁伴噺
const int N = 210; 
const int goods_time = 1000; //璐х墿瀛樺湪鏃堕棿涓?000甯?
const int INF = 1e9 + 10; //鏃犵┓澶?
const int dx[] = {1, -1, 0, 0};
const int dy[] = {0, 0, 1, -1}; //鏂瑰悜鏁扮粍

//鏈哄櫒浜?
struct Robot
{
    int x, y, goods; //鏈哄櫒浜哄潗鏍囦负(x, y), goods == 1琛ㄧず鎼哄甫鐗╁搧锛?鍚﹀垯鏈惡甯?
    int status; //0琛ㄧず鎭㈠鐘舵€侊紝 1琛ㄧず姝ｅ父杩愯鐘舵€?
    int target_gds = -1;  //鐩爣鐗╁搧
    int target_berth = -1;  //鐩爣娉婁綅
    int mbx, mby; //棰勮绉诲姩鍚庝笅涓€涓綅缃紝 鍚庣画鍙敤浜庨伩鍏嶇鎾?
    int pathid = 0; //涓嬩竴姝ヨ矾寰勭储寮?
    vector<int> path;//鏈哄櫒浜鸿矾寰?
    Robot() {}
    Robot(int startX, int startY) {
        x = startX;
        y = startY;
    }
}robot[robot_num + 10];

//娉婁綅
struct Berth
{
    int x, y; //娉婁綅宸︿笂瑙掑潗鏍囦负(x, y)
    int transport_time; //娉婁綅鍒拌櫄鎷熺偣鐨勬椂闂?
    int loading_speed; //姣忓抚鍙互瑁呰浇鐨勭墿鍝佹暟
    Berth(){}
    Berth(int x, int y, int transport_time, int loading_speed) {
        this -> x = x;
        this -> y = y;
        this -> transport_time = transport_time;
        this -> loading_speed = loading_speed;
    }
}berth[berth_num + 10];

//杞埞
struct Boat
{
    int num;
    int pos;    //鐩爣娉婁綅锛?1琛ㄧず铏氭嫙鐐?
    int status; //0琛ㄧず绉诲姩涓紝1琛ㄧず瑁呰揣鐘舵€佹垨杩愯緭瀹屾垚鐘舵€侊紝2琛ㄧず娉婁綅澶栫瓑寰呯姸鎬?
}boat[10];

int money, boat_capacity, id; //褰撳墠閲戦挶鏁帮紝杞埞瀹圭Н锛屽抚鍙?

char MAP[N][N]; //鍦板浘

//鐗╁搧
struct Goods{
	int x,y;    //鐗╁搧浣嶇疆
    int val;    //鐗╁搧浠峰€?
    int left_time;  //鍓╀綑瀛樺湪鏃堕棿
    int targeted; //琚摢涓満鍣ㄤ汉閿佸畾锛屾病鏈夎閿佸畾鏃朵负-1
};

map<int,Goods> gds;
int dis[N][N];
pair<int, int> prev_step[N][N];

//璐х墿瀛樺湪鏃堕棿鍑忓皯
void dec_gdstime()
{
	vector<int> tmp;
    for(auto & g :gds)
    {
        g.second.left_time -= 1;
        if(g.second.left_time <= 0) tmp.push_back(g.first);
    }
    for(auto id : tmp) gds.erase(id);
}
//鍒ゆ柇(x,y)鏄惁鏄満鍣ㄤ汉涓嬩竴涓彲浠ュ幓寰€鐨勭偣
bool IsOkRobotPath(int robotid, int x, int y) {
    if (x < 0 || x > 199 || y < 0 || y >199) return false;
    for (int i = 0; i < robot_num; i++) {
        if(i==robotid) continue;
        if (x == robot[i].x && y == robot[i].y) return false;
    }
    if (MAP[x+1][y+1] == '#' || MAP[x+1][y+1] == '*') return false;
    return true;
}



//鏈哄櫒浜哄崟姝ョЩ鍔ㄦ寚浠?
void robot_move(int robotid)
{
    if(robot[robotid].pathid<robot[robotid].path.size()){
    	printf("move %d %d\n",robotid, robot[robotid].path[robot[robotid].pathid]);
    	robot[robotid].pathid++;
    }
}

// BFS瀵绘壘浠?startX, startY)鍒版渶浼樿揣鐗╃殑鏈€鐭矾寰?
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
            //printf("Goods:%d 被锁定？：%d\n",g_pair.first,g.targeted);
            if (g.targeted == 1 || g.left_time <= dis[x][y]){
            	continue;
            }
            if (x == g.x && y == g.y) {
                double priority = double(g.val) / dis[x][y];
                if (priority > max_priority) {
                    max_priority = priority;
                    robot[robotid].target_gds = g_pair.first;
                    // 鍥炴函璺緞
                    robot[robotid].path.clear();
                    int px = x;
                    int py = y;
                    while (px != sX || py != sY) {
                        int dir = -1;
                        int prev_x = prev_step[px][py].first;
                        int prev_y = prev_step[px][py].second;
                        if (prev_x - px == 1) dir = 1;
                        else if (px - prev_x == 1) dir = 0;
                        else if (prev_y - py == 1) dir = 2;
                        else if (py - prev_y == 1) dir = 3;
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
//	printf("Robotid:%d path:", robotid);
//    for(auto pp : robot[robotid].path) printf("%d ",pp);
//    printf("\n");
//    for(int i = 0 ;i < 199 ;i++){//锟斤拷锟斤拷锟斤拷锟斤拷没锟斤拷锟斤拷锟斤拷 
//		for(int j=0;j<199;j++){
//			for(auto g : gds)
//			{
//				if(i==g.second.x&&j==g.second.y){
//					//cout<< g.second.x << " " << g.second.y << " " <<g.second.val<<endl;
//					printf("Robotid:%d Robotx:%d Roboty:%d GoodsX:%d GoodsY:%d dis:%d\n",robotid ,sX ,sY ,g.second.x ,g.second.y ,dis[g.second.x][g.second.y]);
//				}
//			}
//		}
//	}
	
    /*if (target_index != -1) {
        // 鍥炴函璺緞
        path.clear();
        int px = sX;
        int py = sY;
        while (!(px == robot[robotid].x && py == robot[robotid].y)) {
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
    }*/
}


//鏈哄櫒浜鸿繍鍔?
void Robot_Control(int robotid)
{
    /*寰呭畬鎴?
    if(robot[robotid].status == 0){
        Robot_recover(robotid); //鏈哄櫒浜烘仮澶?寰呯爜
        break;
    }

    if(robot[robotid].goods == 1){
        Robot_to_Berth(robotid); //鍓嶅線娉婁綅锛屽緟鐮?
        break;
    }*/

    if(robot[robotid].target_gds == -1||gds.find(robot[robotid].target_gds)==gds.end()){
        //濡傛灉娌℃湁鐩爣锛屽鎵捐揣鐗╃洰鏍?
        int nowx = robot[robotid].x;
        int nowy = robot[robotid].y;
        FindPath(robotid,nowx, nowy);
//        printf("Robotid : %d ",robotid);
//        printf("Pathid:%d PathSize:%d ",robot[robotid].pathid,robot[robotid].path.size());
//        for(auto pp : robot[robotid].path)  cout<<pp<<" ";
//        printf("\n");
        if(robot[robotid].target_gds!= -1){
            gds[robot[robotid].target_gds].targeted = 1;
        }
        //.....
    }

    if(robot[robotid].target_gds == -1&&gds[robot[robotid].target_gds].x == robot[robotid].x && gds[robot[robotid].target_gds].y == robot[robotid].y) 
    {
        if(robot[robotid].goods == 0){
            gds.erase(robot[robotid].target_gds);
            robot[robotid].target_gds = -1;
            robot[robotid].pathid = 0;
            robot[robotid].path.clear();
            printf("get %d\n", robotid);
        }
    }   //鑻ュ浜庣洰鏍囪揣鐗╀綅缃?
    
	if(robot[robotid].status == 1 ) robot_move(robotid);
}
void Init()
{
    //璇诲叆鍦板浘
    for(int i = 1; i <= n; i ++)
        scanf("%s", MAP[i] + 1);

    //鍒濆鍖栨硦浣嶄俊鎭?
    for(int i = 0; i < berth_num; i ++)
    {
        int id;
        scanf("%d", &id);
        scanf("%d%d%d%d", &berth[id].x, &berth[id].y, &berth[id].transport_time, &berth[id].loading_speed);
    }

    //杞埞瀹圭Н
    scanf("%d", &boat_capacity);

    char okk[100];
    scanf("%s", okk);
    printf("OK\n");
    fflush(stdout);
}

int Input()
{
    scanf("%d%d", &id, &money);

    //鏂板璐х墿
    int num;
    scanf("%d", &num);

    for(int i = 1; i <= num; i ++)
    {
        int x, y, val;
        scanf("%d%d%d", &x, &y, &val);
        Goods new_goods{x, y, val,goods_time,0};  
        gds[id*1000 + i] = new_goods;
    }

    //鏈哄櫒浜轰俊鎭?
    for (int i = 0; i < robot_num; i++)
    {
        int sts;
        scanf("%d%d%d%d", &robot[i].goods, &robot[i].x, &robot[i].y, &sts);
        robot[i].status = sts;
    }

    //杞埞淇℃伅
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
        //for(auto g : gds) cout<<g.first<<" ";
        for(int i = 0; i < robot_num; i ++){
            Robot_Control(i);
            //printf("move %d %d\n", i, rand() % 4);
        }
        puts("OK");
        fflush(stdout);
        dec_gdstime();
    }

    return 0;
}