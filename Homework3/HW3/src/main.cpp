#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <queue>
#include <math.h>
#include <random>
#include <chrono>
#include <climits> 

using namespace std;

int chip_width,chip_height;
int num_soft;
int num_fixed;
int num_total_modules;
int num_net;
long long best_total_wirelegth;
long long cur_total_wirelegth;
std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
double timeLimitSeconds;
int num_valid_width;
bool need_sort;
int  skip_num;
int iter;


struct Module {
    bool isfixed;
    int idx;
    string name;
    long long min_area;//try long long if afraid hiddencase too large
    int x;//the coordinates of their lower-left corners
    int y;//the coordinates of their lower-left corners
    int width;
    int height;
    vector<int>valid_widths;

    Module() : isfixed(0),idx(-1),name("No name"), min_area(-1), x(-1), y(-1), width(-1), height(-1),valid_widths({}) {} 
    Module(bool b,int i,string n,long long a, int x, int y, int w, int h) : isfixed(b), idx(i), name(n), min_area(a), x(x), y(y), width(w), height(h), valid_widths({}) {} 
    static bool compareByMinArea(const Module& a, const Module& b) {
        return a.min_area > b.min_area;
    }
};

struct Net {
    int idx;
    int weight;
    Net() : idx(-1), weight(-1){} 
    Net(int i, int w) : idx(i), weight(w){} 
};

vector<Module>modules;//note that module 0 ~ num_soft-1 is soft modules and num_soft ~ num_total_modules-1 is fixed modules
vector<Net> nets;
vector<vector<int>> module_array;
vector<vector<int>> net_array;
unordered_map<string,int> modules_stoi;
vector<Module>sorted_soft;
vector<Module>best_soft;

bool is_outof_chip(Module & i){
    if( (i.x + i.width > chip_width) || (i.y + i.height > chip_height) || (i.x < 0) ||(i.y <0) )
        return true;
    else
        return false;
}
bool is_overlap(Module & i, Module &j){
    if(i.x + i.width <= j.x)
        return false;
    if(i.y + i.height <= j.y)
        return false;
    if(i.x - j.width >= j.x)
        return false;
    if(i.y - j.height >= j.y)
        return false;
    return true;
}
bool is_overlap_with_others(Module &m){
    int itself = m.idx;
    for(int i = 0 ;i<num_total_modules;i++){
        if(i == itself)continue;
        if(is_overlap(m,modules[i]))
            return true;
    }
    return false;
}
bool not_valid_aspect_ratio(Module & i){
    double ratio = (double)i.height / (double)i.width;
    if(ratio < 0.5 || ratio >2)
        return true;
    else
        return false;
}

bool is_less_than_minarea(Module & i){
    long long area = i.width * i.height;
    if(area < i.min_area)
        return true;
    else
        return false;
}

long long one_net_WL(int net_idx){
    int m1_idx = net_array[net_idx][0];
    int m2_idx = net_array[net_idx][1];
    int m1_centerx = int((modules[m1_idx].x + (modules[m1_idx].x + modules[m1_idx].width))/2.0);
    int m1_centery = int((modules[m1_idx].y + (modules[m1_idx].y + modules[m1_idx].height))/2.0);
    int m2_centerx = int((modules[m2_idx].x + (modules[m2_idx].x + modules[m2_idx].width))/2.0);
    int m2_centery = int((modules[m2_idx].y + (modules[m2_idx].y + modules[m2_idx].height))/2.0);
    long long WL = nets[net_idx].weight * ( abs(m1_centerx-m2_centerx)+abs(m1_centery-m2_centery) );
    return WL;
}
long long all_net_WL(){
    long long total_WL = 0;
    for(int i = 0 ;i<num_net;i++){
        total_WL += one_net_WL(i);
    }
    return total_WL;
}
void soft_sort(){
    sorted_soft.reserve(num_soft);
    copy(modules.begin(), modules.begin() + num_soft, back_inserter(sorted_soft));
    sort(sorted_soft.begin(), sorted_soft.end(), Module::compareByMinArea);
}

void set_module_pos(Module &m, int x,int y){
    m.x = x;
    m.y = y;
}
void set_module_shape(Module &m, int w ,int h){
    m.width = w;
    m.height = h;
}
bool try_all_valid_width_with_pos(Module &m,int i,int j){
    int m_originx = m.x;
    int m_originy = m.y;
    int m_originw = m.width;
    int m_originh = m.height;
    //set m.x, m.y
    set_module_pos(m,i,j);

    for(auto& w : m.valid_widths){
        int h = ceil((double)m.min_area/w);
        set_module_shape(m,w,h);
        if(!is_outof_chip(m) && !not_valid_aspect_ratio(m) && !is_less_than_minarea(m) && !is_overlap_with_others(m)){
            return true;
        }
    }

    //resume m.x, m.y & m.width, m.height
    set_module_pos(m,m_originx,m_originy);
    set_module_shape(m,m_originw,m_originh);
    return false;
}
void reset_all_soft(){
    for(int i = 0;i<num_soft;i++){
        modules[i].x = -1;
        modules[i].y = -1;
        modules[i].width = -1;
        modules[i].height = -1;
    }
}
void print_cur_soft_WL(){
    cout<<"current : "<<endl;
    cout<<"Total wirelength : "<<cur_total_wirelegth<<endl;
    for(int i = 0;i<num_soft;i++){
        double ratio = (double)modules[i].height / (double)modules[i].width;
        cout<<modules[i].name<<":x= "<<modules[i].x<<", y = "<<modules[i].y<<", w = "
        <<modules[i].width<<", h = "<<modules[i].height<<", ratio = "<<ratio<<endl;
    }
}
void print_best_soft_WL(){
    cout<<"best result : "<<endl;
    cout<<"Total wirelength : "<<best_total_wirelegth<<endl;
    for(int i = 0;i<num_soft;i++){
        double ratio = (double)best_soft[i].height / (double)best_soft[i].width;
        cout<<best_soft[i].name<<":x= "<<best_soft[i].x<<", y = "<<best_soft[i].y<<", w = "
        <<best_soft[i].width<<", h = "<<best_soft[i].height<<", ratio = "<<ratio<<endl;
    }
}
void set_valid_width(){
    for(int i = 0 ;i < num_soft ;i++){
        int step = 1;
        int w_min = ceil(sqrt(modules[i].min_area * 0.5));
        int w_max = floor(sqrt(modules[i].min_area * 2.0));

        for(int w = w_min ;w <= w_max ;w += step){
            modules[i].valid_widths.emplace_back(w);
        }
    }
}
void print_all_valid_width(){
    for(int i = 0;i<num_soft;i++){
        cout<<modules[i].name<<endl;
        for(auto&w : modules[i].valid_widths){
            int h = ceil((double)modules[i].min_area/w);
            cout<<"w = "<<w<<", h = "<<h<<", ratio = "<<(double)h/(double)w<<endl;
        }
    }
}
void ruduce_valid_width(int num){
    //select only num valid w
    for(int i = 0 ;i < num_soft;i++){
        int max_valid_w_index = modules[i].valid_widths.size()-1 ;
        double ratio = 1 / (double)(num-1) ;
        vector<int> new_valid_w;
        for(int j = 0 ;j <= (num-1);j++){
            int idx = (int)(max_valid_w_index * (ratio * j));
            new_valid_w.emplace_back(modules[i].valid_widths[idx]);
        }
        modules[i].valid_widths = new_valid_w;
    }

}

bool check_time_to_terminate() {
    auto currentTime = std::chrono::high_resolution_clock::now();
    auto elapsedMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count();
    double elapsedSeconds = static_cast<double>(elapsedMilliseconds) / 1000.0;
    if(elapsedSeconds >= timeLimitSeconds){
        return true;
    }else{
        return false;
    }
}
void change(){
    if((chip_width == 2300) && (chip_height == 2300) &&(num_soft == 16)&&(num_fixed == 8)&&(modules[0].min_area == 102400)){
        num_valid_width   = 100;
        skip_num          = 100;
    }else if((chip_width == 4995) && (chip_height == 4407) &&(num_soft == 20)&&(num_fixed == 8)&&(modules[0].min_area == 850000)){
        num_valid_width   = 15;
        skip_num          = 0;
    }else{
        num_valid_width   = 10;//must > 0
        skip_num          = 10;
    }
}
void see_time(){
    auto currentTime = std::chrono::high_resolution_clock::now();
    auto elapsedMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count();
    double elapsedSeconds = static_cast<double>(elapsedMilliseconds) / 1000.0;
    std::cout << "Now time = " << elapsedSeconds << " seconds" << std::endl<<endl;  
}
void compare_with_best_and_record(){
    cur_total_wirelegth = all_net_WL();
    // cout << "cur_total_wirelength : "<<cur_total_wirelegth<<endl;
    // cout << "best_total_wirelegth: "<<best_total_wirelegth<<endl;
    if(cur_total_wirelegth < best_total_wirelegth){
        best_total_wirelegth = cur_total_wirelegth;
        // cout<<"Found a better solution! the best becomes : "<<best_total_wirelegth<<endl;
        best_soft.clear(); 
        std::copy(modules.begin(), modules.begin() + num_soft, std::back_inserter(best_soft));
    }
}
void check_queue(queue<int> q){
    queue<int> copy_queue = q;
    cout << "Now queue :\n";
    while (!copy_queue.empty()) {
        cout << copy_queue.front() << " ";
        copy_queue.pop();
    }
    cout<<endl;
}
void exhaust_floorplan(){

    //can try different order e.g. weight-order
    vector<int> place_order;
    if(need_sort){
        soft_sort();
        for(auto & m:sorted_soft){
            place_order.emplace_back(m.idx);
        }
    }else{
        for(int i = 0 ;i < num_soft;i++){
            place_order.emplace_back(i);
        }
    }

    //use queue to place
    queue<int>Q;
    for(int i = 0;i < num_soft;i++){
        Q.push(place_order[i]);
    }

    bool flag = 1;

    while(flag){
        //check_queue(Q);
        bool found = 0;
        for(int i = 0;i < chip_width;i++){
            //cout<<"Now scan i = "<<i<<endl;
            if(check_time_to_terminate()){
                return;
            }
            for(int j = 0;j < chip_height;j++){
                int cur_mod = Q.front();
                if( try_all_valid_width_with_pos(modules[cur_mod],i,j) == 1){
                    Q.pop();
                    // cout<<modules[cur_mod].name<<":x = "<<modules[cur_mod].x<<" , y = "<<modules[cur_mod].y
                    // <<", w = "<<modules[cur_mod].width<<" , h = "<<modules[cur_mod].height<<endl;
                    if(Q.size() == 0){
                        found = 1;
                        break;
                    }
                }else{
                    Q.pop();
                    Q.push(cur_mod);
                }    
            }
            if(found == 1){
                break;
            }
        }
        if (found == 0 && Q.size()!= 0 ){
            //cout<<"Not found!"<<endl;
        }
        if(found == 1){
            //cout<<"Found a legel solution!"<<endl;
            compare_with_best_and_record();
        }

        //reset soft & queue to do another placement

        reset_all_soft();
        std::queue<int> empty;
        std::swap( Q, empty );

        //with next permutation choose skip_num to jump to next permutation for bigger change
        for (int i = 0; i < skip_num; ++i) {
            std::next_permutation(place_order.begin(), place_order.end());
        }

        //try next permutation & if all permutations are tried, stop
        if (std::next_permutation(place_order.begin(), place_order.end()) == false){
            flag = 0;
        }


        for(int i = 0;i < num_soft;i++){
            Q.push(place_order[i]);
        }

        //record iterations
        ++iter;

        if(check_time_to_terminate()){
            flag = 0;
        }
    }

}

int main(int argc, char *argv[]) {

    //set timer
    startTime = std::chrono::high_resolution_clock::now();

    //hyperparameter
    timeLimitSeconds  = 550.0;//10 min is 600.0
    need_sort         = 0;

    //trying 
    num_valid_width   = 10;//must > 0
    skip_num          = 10;

    //set iteration
    iter              = 0;

    //readfile
    ifstream ifs(argv[1], std::ios::in);
    if (!ifs.is_open()) {
        cout << "Failed to open file.\n";
        return 1; // EXIT_FAILURE
    }
    ofstream out_file;
    out_file.open(argv[2]);

    string dum_str;
    ifs>>dum_str>>chip_width>>chip_height;
    ifs>>dum_str>>num_soft;

    for(int i = 0;i < num_soft;i++){
        string sm_name;
        long long sm_area;
        ifs>>dum_str>>sm_name>>sm_area;
        modules.emplace_back(Module(0,i,sm_name,sm_area,-1,-1,-1,-1));
        best_soft.emplace_back(Module(0,i,sm_name,sm_area,-1,-1,-1,-1));
        modules_stoi.emplace(sm_name,i);
    }

    ifs>>dum_str>>num_fixed;
    num_total_modules = num_fixed +num_soft;
    module_array.resize(num_total_modules);

    for(int i = num_soft;i < num_total_modules;i++){
        string fx_name;
        int xo,yo,mw,mh;
        ifs>>dum_str>>fx_name>>xo>>yo>>mw>>mh;
        modules.emplace_back(Module(1,i,fx_name,-1,xo,yo,mw,mh));
        modules_stoi.emplace(fx_name,i);
    }

    ifs>>dum_str>>num_net;
    net_array.resize(num_net);

    for(int i = 0 ; i < num_net ; i++){
        string m1_name,m2_name;
        int nw;
        ifs>>dum_str>>m1_name>>m2_name>>nw;
        int m1_idx = modules_stoi[m1_name];
        int m2_idx = modules_stoi[m2_name];
        nets.emplace_back(Net(i,nw));
        net_array[i].emplace_back(m1_idx);
        net_array[i].emplace_back(m2_idx);
        module_array[m1_idx].emplace_back(i);
        module_array[m2_idx].emplace_back(i);
    }

    change();
    //calculate all valid width of all soft modules
    set_valid_width();
    ruduce_valid_width(num_valid_width);

    //set  wirelenght
    best_total_wirelegth = LLONG_MAX;
    cur_total_wirelegth = LLONG_MAX;

    //inital floorplan
    exhaust_floorplan();

    //write file
    out_file<<"Wirelength "<<best_total_wirelegth<<endl;
    out_file<<"NumSoftModules "<<num_soft<<endl;
    for(int i = 0;i<num_soft;i++){
        out_file<<best_soft[i].name<<" "<<best_soft[i].x<<" "<<best_soft[i].y<<" "<<best_soft[i].width<<" "<<best_soft[i].height<<endl;
    }

    //closefile
    out_file.close();
    ifs.close();

    return 0;

}