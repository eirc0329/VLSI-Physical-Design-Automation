#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <set>
#include <chrono>
#include <queue>

using namespace std;

int NumTechs;
string dum_str,TechA,TechB;
//here
unsigned long long DieSize,DieA_area_limit,DieB_area_limit; 
int num_cell;
int num_net;
int Pmax;
int max_gain_val,max_gain_cell_index;
unordered_map<string, int> Tech_index;//Tech_index['TC'] = 1,mapping 'TC' to index 1
vector<unordered_map<string, int>> Tech;//Tech[Tech_idex['TC']]['MC1'] = 70, under TC MC1 has size 70
vector<vector<int>> cell_array;
vector<vector<int>> net_array;
vector<vector<int>> net_distri;
unordered_map<string,int>cell_index;        //cell_index['C20'] = 1, cell 'C20' = index 1
unordered_map<int,string>cell_index_inverse;//cell_index_inverse[1] = 'C20', index 1 = cell 'C20'
vector<vector<int>>cell_size;//0 is for DieA, 1 is for DieB
vector<bool>cell_group;
vector<int>cell_gain;
vector<int> chosen_cell;//index means the step i of p37
vector<int> max_gain;
//here
vector<unsigned long long> A_size;
vector<unsigned long long> B_size;
vector<bool>locked;
vector<int>DieA_cells;
vector<int>DieB_cells;

struct Cell {
    int data;
    Cell* next;
    Cell* prev;
    Cell() : data(0), next(nullptr), prev(nullptr) {} 
    Cell(int value) : data(value), next(nullptr), prev(nullptr) {}
};
int gain_exchange(int gain){
    return gain + Pmax;
}

vector<Cell*>lista;
vector<Cell*>cells_addr;
void append(int gain,Cell * cell){
    cell->next = nullptr;
    cell->prev = nullptr;
    Cell *head = lista[ gain_exchange(gain)];

    if(head->next == nullptr){
        //new
        head->prev = cell;

        head->next = cell;
        cell->prev = head;
    }else if(head->next != nullptr){
        //new
        cell->prev       = head->prev;
        head->prev->next = cell;
        head->prev       = cell;

        // Cell *cur = head;
        // while(cur -> next != nullptr){
        //     cur = cur->next;
        // }
        // cur ->next =cell;
        // cell->prev = cur;
    }

    return;
}
void del(int gain,Cell * cell){
    Cell *head = lista[ gain_exchange(gain)];

    if(cell->prev == head && cell->next == nullptr){
        //new
        head->prev = nullptr;

        head->next = nullptr;
    }else if(cell->prev == head && cell->next != nullptr){
        head->next = cell->next;
        cell->next->prev = head;
    }else if(cell->prev != head && cell->next == nullptr){
        //new
        head->prev = head->prev->prev;

        cell->prev->next = nullptr;
    }else if(cell->prev != head && cell->next != nullptr){
        cell->prev->next = cell->next;
        cell->next->prev = cell->prev;
    }
    cell->next = nullptr;
    cell->prev = nullptr;
    return;
}
void traverse(Cell *cell){
    Cell* cur = cell;
    while(cur != nullptr){
        cout<<cur->data<<" ";
        cur = cur->next;
    }
}
int total_node_lista(){
    int total = 0;
    for(int i =0;i<lista.size();i++){
        Cell* cur = lista[i];
        while(cur != nullptr){
            total++;
            cur = cur->next;
        }
        total--;
    }
    return total;
}
bool valid(int t,int cell_idx){
    int c_group = cell_group [cell_idx];

    //here
    int  seta_size = A_size[ t - 1] ;
    int  setb_size = B_size[ t - 1] ;

    if( c_group == 0){
        seta_size -= cell_size[0][cell_idx];
        setb_size += cell_size[1][cell_idx];
    }else{
        seta_size += cell_size[0][cell_idx];
        setb_size -= cell_size[1][cell_idx];
    }
    if(seta_size < DieA_area_limit && setb_size < DieB_area_limit && seta_size > 0 && setb_size > 0){
        return true;
    }

    else return false;
}
void max_update(int gain,int t,int cell_idx){
    if(valid(t,cell_idx) ){
        if(gain > max_gain_val){
            max_gain_cell_index = cell_idx;
            max_gain_val = gain;
        }   
    }
    if(cell_idx == max_gain_cell_index){
        int temp_invalid_max_gain_cell_index;
        int temp_invalid_max_gain_val;
        bool find_valid_max = 0;
        for(int i =lista.size()-1;i>=0;i--){
            if(lista[i]->next == nullptr){
                continue;
            }
            Cell * cur = lista[i]->next;
            bool flag = 0;
            while(cur != nullptr){
                if(valid(t,cur->data)){
                    max_gain_cell_index = cur->data;
                    max_gain_val = i-Pmax;
                    flag = 1;
                    find_valid_max = 1;
                    break;
                }
                temp_invalid_max_gain_cell_index = cur->data;
                temp_invalid_max_gain_val = i-Pmax;
            

                cur = cur->next;
            }

            if(flag == 1)break;
            
        }
        if(find_valid_max == 0){
            max_gain_cell_index = temp_invalid_max_gain_cell_index;
            max_gain_val = temp_invalid_max_gain_val;   
        }

    }
}
void replace_next_max(int t){
    bool find_valid_next_max = 0;
    for(int i = max_gain_val+Pmax;i >=0;i--){
        Cell* temp = lista[i]->next;
        while(temp != nullptr){
            if(valid(t,temp->data)){
                max_gain_cell_index = temp->data;
                max_gain_val = i-Pmax;  
                find_valid_next_max = 1;
                return;     
            }
            temp = temp->next;
        }
    }
    if(find_valid_next_max == 0){
        int temp_invalid_max_gain_cell_index;
        int temp_invalid_max_gain_val;
        bool find_valid_max = 0;
        for(int i =lista.size()-1;i>=0;i--){
            if(lista[i]->next == nullptr){
                continue;
            }
            Cell * cur = lista[i]->next;
            bool flag = 0;
            while(cur != nullptr){
                if(valid(t,cur->data)){
                    max_gain_cell_index = cur->data;
                    max_gain_val = i-Pmax;
                    flag = 1;
                    find_valid_max = 1;
                    break;
                }
                temp_invalid_max_gain_cell_index = cur->data;
                temp_invalid_max_gain_val = i-Pmax;
            

                cur = cur->next;
            }

            if(flag == 1)return;
            
        }
        if(find_valid_max == 0){
            max_gain_cell_index = temp_invalid_max_gain_cell_index;
            max_gain_val = temp_invalid_max_gain_val;   
            return;
        }

    }

}

//here
queue<int>Q0;
queue<int>Q1;
void fix_area_div(unsigned long long g0_size, unsigned long long g1_size){
   std::queue<int> empty0,empty1;
   std::swap( Q0, empty0 );
   std::swap( Q1, empty1 );
   for(int i = 0;i<num_cell;i++){
        if(cell_group[i] == 0)
            Q0.push(i);
        else
            Q1.push(i);
   }
   bool signal = 1;
   while(signal== 1){
        int flag = -1;
        if(g0_size >= DieA_area_limit){
            flag = 0;
            int cell = Q0.front();
            Q0.pop();
            Q1.push(cell);
            g0_size -= cell_size[0][cell];
            g1_size += cell_size[1][cell];
            cell_group[cell] = 1;

        }else if(g1_size >= DieB_area_limit){
            flag = 1;
            int cell = Q1.front();
            Q1.pop();
            Q0.push(cell);
            g0_size += cell_size[0][cell];
            g1_size -= cell_size[1][cell];
            cell_group[cell] = 0;
        }else{
            flag = 2;
            signal = 0;
        }

   }
    // cout<<"after fix"<<endl;
    // cout<<"g0_size:   "<<g0_size<<"  g1_size:   "<<g1_size<<endl;
}

void time_check(std::chrono::high_resolution_clock::time_point startTime) {
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    std::cout << "時間: " << duration.count() / 1000.0 << " 秒" << std::endl;
}

int main(int argc, char *argv[]) {

auto startTime = std::chrono::high_resolution_clock::now();
    //readfile
    ifstream ifs(argv[1], std::ios::in);
    if (!ifs.is_open()) {
        cout << "Failed to open file.\n";
        return 1; // EXIT_FAILURE
    }
    ofstream out_file;
    out_file.open(argv[2]);

    ifs>>dum_str>>NumTechs;
    Tech.resize(NumTechs);

    for(int i = 0 ;i < NumTechs;i++){
        string Tech_name;
        int num_lib_cell;
        ifs>>dum_str>>Tech_name>>num_lib_cell;
        Tech_index.emplace(Tech_name,i);

        for(int j = 0 ;j < num_lib_cell;j++){
            string lib_cell;
            int width,height;
            ifs>>dum_str>>lib_cell>>width>>height;
            Tech[i].emplace(lib_cell,width*height);
        }
    }
    double diew,dieh;
    double perA,perB;
    ifs>>dum_str>>diew>>dieh;
    DieSize = diew*dieh;
    ifs>>dum_str>>TechA>>perA;
    ifs>>dum_str>>TechB>>perB;
    DieA_area_limit = (DieSize  / 100)* perA ;
    DieB_area_limit = (DieSize  / 100)* perB;

    ifs>>dum_str>>num_cell;
    cell_size.resize(2);
    for(int i = 0;i < 2;i++){
        cell_size[i].resize(num_cell);
    }
    cell_gain.resize(num_cell);
    cells_addr.resize(num_cell);
    for(int i = 0 ;i< num_cell;i++){
        Cell* cell = new Cell(i);
        cells_addr[i] = cell;
    }

    for(int i = 0 ;i<num_cell;i++){
        string cell_name_temp, libcell_temp;
        ifs>>dum_str>>cell_name_temp>>libcell_temp;
        cell_index.emplace(cell_name_temp,i);
        cell_index_inverse.emplace(i,cell_name_temp);
        cell_size[0][i] = Tech[Tech_index[TechA]][libcell_temp];//All cell's size in DieA
        cell_size[1][i] = Tech[Tech_index[TechB]][libcell_temp];//All cell's size in DieB
    }

    ifs>>dum_str>>num_net;
    cell_array.resize(num_cell);
    net_array.resize(num_net);
    net_distri.resize(num_net);
    for(int i = 0 ;i < num_net;i++ ){
        net_distri[i].resize(2);
    }

    for(int i = 0;i < num_net;i++){
        string netname_temp;
        int cell_in_net;
        ifs>>dum_str>>netname_temp>>cell_in_net;
        for(int j = 0;j < cell_in_net ; j++){
            string cell_name_temper;
            ifs>>dum_str>>cell_name_temper;
            cell_array[ cell_index[cell_name_temper] ].emplace_back(i);
            net_array[i].emplace_back(cell_index[cell_name_temper]);
        }
    }
    /*calculate Pmax*/
    Pmax = 0;
    for(int i = 0;i < num_cell;i++){
        int siz = cell_array[i].size();
        Pmax = max(Pmax,siz);
    }
    lista.resize(2*Pmax+1);
    for(int i = 0 ;i< 2*Pmax+1;i++){
        Cell* cell = new Cell(-4);
        lista[i] = cell;
    }
    /*end of calculate Pmax*/

    /*initial partition*/
    cell_group.resize(num_cell);
    bool flag = 1;
    //here
    unsigned long long   g0_size = 0;
    unsigned long long   g1_size = 0;



    for(int i =0 ;i <num_cell;i++){
        if( i<num_net/2){
            cell_group[i] = 0;
            g0_size += cell_size[0][i];
        }else{
            cell_group[i] = 1;
            g1_size += cell_size[1][i];
        }
    }
    

    // cout<<"Diesize:"<<DieSize<<endl;
    // cout<<"DieA limit:"<<DieA_area_limit<<" DieB limit:"<<DieB_area_limit<<endl;
    // cout<<"Initial finish!before fix:"<<endl;
    // cout<<"g0_size:   "<<g0_size<<"  g1_size:   "<<g1_size<<endl;

    for(int t = 0;t<1;t++){
        unsigned long long  g0_t = 0;
        unsigned long long  g1_t = 0;
        for(int i = 0 ;i<num_cell;i++){
            if(cell_group[i] == 0){
                g0_t += cell_size[0][i];
            }else{
                g1_t += cell_size[1][i];
            }
        }
        fix_area_div(g0_t,g1_t);
    }

    /*end of initial partition*/

    /*iteration*/
    int iter = 0;
    auto iter0 = std::chrono::high_resolution_clock::now();
    auto iter1 = std::chrono::high_resolution_clock::now();
    while(flag == 1)
    {
        if(iter == 0)iter0 = std::chrono::high_resolution_clock::now();
        if(iter == 1)iter1 = std::chrono::high_resolution_clock::now();
        // cout<<"iter :"<<iter<<" ";
        // time_check(startTime);
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        if ((duration.count() + std::chrono::duration_cast<std::chrono::milliseconds>(iter1 - iter0).count()) > 250000) {
            break;
        }
        iter ++;
        /*calculate  group0_size & group1_size*/
        //here
        unsigned long long  group0_size = 0;
        unsigned long long  group1_size = 0;
        for(int i = 0 ; i< num_cell ;i++)
        {
            if( cell_group[i] == 0 )
                group0_size +=  cell_size[0][i];
            if( cell_group[i] == 1 )
                group1_size +=  cell_size[1][i];
        }
        /*end of calculate  group0_size & group1_size*/

        /*contruct net distributtion*/
        for(int i = 0 ;i < num_net;i++){
            net_distri[i][0] = 0;
            net_distri[i][1] = 0;
        }

        for(int i = 0 ;i<num_net ;i++)
        {
            int net_size = net_array[i].size();
            for(int j = 0 ;j< net_size ; j++)
            {
                if(cell_group[net_array[i][j]] == 0)
                    net_distri[i][0]++;
                else
                    net_distri[i][1]++;
            }
        }
        /*end of contruct net distributtion*/


        /*calculate cost*/
        // int cost = 0;
        // for(int i = 0 ;i < num_net ;i++)
        // {
        //     if( net_distri[i][0] > 0 && net_distri[i][1] > 0 )
        //         cost++;
        // }
        // cout<<"\ncost:"<<cost<<"\n";


        /*end of calculate cost*/

        /*initial cell gain*/
        for(int i = 0 ;i < num_cell;i++){
            cells_addr[i]->next = nullptr;
            cells_addr[i]->prev = nullptr;
        }
        for(int i = 0 ;i < lista.size();i++){
            lista[i] ->next = nullptr;
            lista[i] ->prev = nullptr;
        }
        for(int i = 0 ;i <num_cell;i++){
            cell_gain[i] = 0;
        }

        for(int i = 0 ;i < num_cell ;i++)
        {
            bool F = cell_group[i];
            bool T = 1-F;
            int siz = cell_array[i].size();

            for(int j = 0 ;j < siz ; j++)
            {
                if(net_distri[ cell_array[i][j] ][F] == 1)
                    cell_gain[i]++;
                if(net_distri[ cell_array[i][j] ][T] == 0)
                    cell_gain[i]--;
            }

        }

        max_gain_val = -Pmax;
        for(int i = 0 ;i < num_cell ;i++){
            append(cell_gain[i],cells_addr[i]);
        }

        for(int i = 0 ;i < num_cell;i++){
            if(cell_gain[i] > max_gain_val){
                max_gain_cell_index= i;
                max_gain_val = cell_gain[i];
            }
        }

        /*end of initial cell gain*/

        /*every step's info in one iteration*/
        chosen_cell.resize(num_cell + 1);//index means the step i of p37
        fill(chosen_cell.begin(), chosen_cell.end(), 0);

        max_gain.resize(num_cell + 1);
        fill(max_gain.begin(), max_gain.end(), 0);

        A_size.resize(num_cell + 1);
        fill(A_size.begin(), A_size.end(), 0);

        B_size.resize(num_cell + 1);
        fill(B_size.begin(), B_size.end(), 0);

        locked.resize(num_cell);
        fill(locked.begin(), locked.end(), 0);

        A_size[0] = group0_size;
        B_size[0] = group1_size;

        /*end of every step's info in one iteration*/

        /*choose one cell and lock ,also update others gain*/
        for(int t = 1 ;t <= num_cell ; t++ )//STEP 1~STEP num_cell
        { 
            
            set<int>updated_cells;
            /*find max gain*/
            int cell_name = max_gain_cell_index;
            int m_gain = max_gain_val;
           
            del(cell_gain[cell_name],cells_addr[cell_name]);
            replace_next_max(t);
           

            /*locked the cell and store this step's info (chosen_cell , max_gain, A_size,locked)*/
            chosen_cell[ t ] = cell_name;
            max_gain[ t ] = m_gain;
            if (cell_group [ cell_name ] == 0){
                A_size[ t ] = A_size[ t-1 ] - cell_size[0][cell_name];
                B_size[ t ] = B_size[ t-1 ] + cell_size[1][cell_name];
            }    
            else{
                A_size[ t ] = A_size[ t-1 ] + cell_size[0][cell_name];
                B_size[ t ] = B_size[ t-1 ] - cell_size[1][cell_name];
            }                                  
            locked[cell_name] = 1;

           
            /*end of locked the cell and store this step's info (chosen_cell , max_gain, A_size,locked)*/

            /*update cell gains & net_distri*/
            int cell_array_size = cell_array[cell_name].size();
            bool F = cell_group [ cell_name ];
            bool T = 1-F;
            for(int i = 0 ;i < cell_array_size ;i++)
            {
                int net_name = cell_array[cell_name][i];

                //before the move
                if( net_distri[ net_name ][T] == 0 )
                {
                    int net_size = net_array[net_name].size();
                    for(int j = 0 ; j< net_size ; j++)
                    {
                        if( locked [ net_array[net_name][j] ] == 0 ){
                            int idx = net_array[net_name][j];
                            del(cell_gain[idx] , cells_addr[idx]);
                            cell_gain[ idx ]++;
                            append(cell_gain[idx] , cells_addr[idx]);
                            updated_cells.insert(idx);
                        }
                    }
                }
                else if( net_distri[ net_name ][T] == 1 )
                {
                    int net_size = net_array[net_name].size();
                    for(int j = 0 ; j< net_size ; j++)
                    {
                        if( locked [ net_array[net_name][j] ] == 0 && cell_group [ net_array[net_name][j] ] == T)
                        {
                            int idx = net_array[net_name][j];
                            del(cell_gain[idx] , cells_addr[idx]);
                            cell_gain[ idx ]--;
                            append(cell_gain[idx] , cells_addr[idx]);
                            updated_cells.insert(idx);
                            break;
                        }
                    }
                }

                //chang F,T to reflect the move
                net_distri[ net_name ][F]--;
                net_distri[ net_name ][T]++;

                //after the move
                if( net_distri[ net_name ][F] == 0 )
                {
                    int net_size = net_array[net_name].size();
                    for(int j = 0 ; j< net_size ; j++)
                    {
                        if( locked [ net_array[net_name][j] ] == 0 ){
                            int idx = net_array[net_name][j];
                            del(cell_gain[idx] , cells_addr[idx]);
                            cell_gain[ idx ]--;
                            append(cell_gain[idx] , cells_addr[idx]);
                            updated_cells.insert(idx);
                        }
                    }
                }
                else if( net_distri[ net_name ][F] == 1 )
                {
                    int net_size = net_array[net_name].size();
                    for(int j = 0 ; j< net_size ; j++)
                    {
                        if( locked [ net_array[net_name][j] ] == 0 && cell_group [ net_array[net_name][j] ] == F)
                        {
                            int idx = net_array[net_name][j];
                            del(cell_gain[idx] , cells_addr[idx]);
                            cell_gain[ idx ]++;
                            append(cell_gain[idx] , cells_addr[idx]);
                            updated_cells.insert(idx);
                            break;
                        }
                    }
                }

            }
            /*end of update cell gains & net_distri*/

            /*update max_gain_val*/
            for(auto& x:updated_cells){
                max_update(cell_gain[x],t,x);
            }            
            /*end of update max_gain_val*/
        }

        /*end of choose one cell and lock ,also update others gain*/


        /*find the largest partial sum & need to be balance,if tie chooce the most balanced one*/
        int k = 0;
        int max_partial_sum = 0;
        int part_sum = 0;
        for(int i = 1 ;i<=num_cell ;i++)
        {
            part_sum += max_gain[i];
            //need to be balanced
            if(A_size[i] <DieA_area_limit && B_size[i] <DieB_area_limit && A_size[i]>0 && B_size[i]>0)
            {
                // cout<<A_size[i]<<" "<<B_size[i]<<endl;
                if(part_sum > max_partial_sum)
                {
                    k = i;
                    max_partial_sum = part_sum;
                }
            }
        }
        /*find the largest partial sum & need to be balance,if tie chooce the most balanced one*/


        /*if Gk > 0 then swap and  do it again ,else stop*/

        if( max_partial_sum > 0 )
        {
            for(int i =1 ;i <= k;i++)
            {
                bool group = cell_group[ chosen_cell[i]];
                cell_group[ chosen_cell[i]] = !group;
            }
        }
        else
        {
            flag = 0;
        }
        /*end of if Gk > 0 then swap and  do it again ,else stop*/
    }
    /*end of iteration*/

    /*fix area*/
    //herr
    unsigned long long  g0 = 0;
    unsigned long long  g1 = 0;
    for(int i = 0 ;i<num_cell;i++){
        if(cell_group[i] == 0){
            g0 += cell_size[0][i];
        }else{
            g1 += cell_size[1][i];
        }
    }
    // cout<<"DieA limit:"<<DieA_area_limit<<" DieB limit:"<<DieB_area_limit<<endl;
    // cout<<"FM finished!before fix:"<<endl;
    // cout<<"g0_size:   "<<g0<<"  g1_size:   "<<g1<<endl;
    fix_area_div(g0,g1);

    /*end of fix area*/

    /*final cost*/
    int final_cost = 0;
    for(int i = 0; i < num_net ;i++)
    {
        int size = net_array[i].size();
        bool group = cell_group[ net_array[i][0] ];
        for(int j = 1 ;j < size ;j++)
        {
            if(cell_group[ net_array[i][j] ]!= group )
            {
                final_cost++;
                break;
            }
        }
    }
    //cout<<"\nfinal cost:"<<final_cost<<"\n";
    /*end of final cost*/

    /*output*/
    for(int i =0;i<num_cell;i++){
        if(cell_group[i] == 0)
            DieA_cells.emplace_back(i);
        else
            DieB_cells.emplace_back(i);
    }
    out_file<<"CutSize "<<final_cost<<endl;
    out_file<<"DieA "<<DieA_cells.size()<<endl;
    for(int i = 0;i < DieA_cells.size();i++){
        out_file<<cell_index_inverse[DieA_cells[i]]<<endl;
    }
    out_file<<"DieB "<<DieB_cells.size()<<endl;
    for(int i = 0;i < DieB_cells.size();i++){
        out_file<<cell_index_inverse[DieB_cells[i]]<<endl;
    }
    /*end of output*/
    out_file.close();
    ifs.close();


    // auto endTime = std::chrono::high_resolution_clock::now();
    // auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    // std::cout << "最終時間: " << duration.count() /1000.0<< " 秒" << std::endl;
    return 0;

}