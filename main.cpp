#include <vector>
#include <iostream>
#include <sstream>
#include <limits>
#include <algorithm>
#include <signal.h>
#include <string.h>
#include <fstream>
#include <random>

using namespace std;

volatile sig_atomic_t time_limit_exceeded = 0;
 
void signal_term_handler(int signum)
{
    time_limit_exceeded = 1;
}

struct Graph{
  vector<vector<int>>neighbor_list;

  void input(istream&in){
    string line;
    while(getline(in, line)){
      istringstream lin(line);
      if(lin.peek() == 'c'){
        continue;
      }else if(lin.peek() == 'p'){
        char p;
        string cep;
        int n, m;
        lin >> p >> cep >> n >> m;
        for(auto&x:neighbor_list){
          x.clear();
        }
        neighbor_list.resize(n);
      }else{
        int x, y;
        lin >> x >> y;
        --x;
        --y;
        neighbor_list[x].push_back(y);
        neighbor_list[y].push_back(x);
      }
    }

    for(vector<int>&x:neighbor_list){
      std::sort(x.begin(), x.end());
    }
  }
};

struct Clustering{
  vector<int>color_of_node;
  
  void init_with_singletons(const Graph&g){
    const int node_count = g.neighbor_list.size();

    color_of_node.resize(node_count);
    for(int i=0; i<node_count; ++i){
      color_of_node[i] = i;
    }
  }

  void output_edits(const Graph&g, ostream&out){
    const int node_count = g.neighbor_list.size();
    for(int x=0; x<node_count; ++x){
      for(int y:g.neighbor_list[x]){
        if(x < y){
          if(color_of_node[x] != color_of_node[y]){
            out << (x+1) << ' ' << (y+1) << '\n';
          }
        }
      }
    }

    vector<vector<int>>node_with_color_list(node_count);
    for(int x=0; x<node_count; ++x){
      node_with_color_list[color_of_node[x]].push_back(x);
    }
    
    for(int x=0; x<node_count; ++x){
      int color = color_of_node[x];
      vector<int>edits;
      set_difference(
        node_with_color_list[color].begin(), node_with_color_list[color].end(),
        g.neighbor_list[x].begin(), g.neighbor_list[x].end(),
        back_inserter(edits)
      );
      for(int y:edits){
        if(x<y){
          out << (x+1) << ' ' << (y+1) << '\n';
        }
      }
    }
  }

  int count_removals(const Graph&g){
    const int node_count = color_of_node.size();

    int count = 0;

    for(int x=0; x<node_count; ++x){
      for(int y:g.neighbor_list[x]){
        if(x < y){
          if(color_of_node[x] != color_of_node[y]){
            ++count;
          }
        }
      }
    }
    return count;
  }

  int count_inserts(const Graph&g){
    const int node_count = color_of_node.size();

    int count = 0;

    vector<vector<int>>node_with_color_list(node_count);
    for(int x=0; x<node_count; ++x){
      node_with_color_list[color_of_node[x]].push_back(x);
    }

    vector<int>edge_count_with_color(node_count, 0);
    for(int x=0; x<node_count; ++x){
      for(int y:g.neighbor_list[x]){
        if(x < y){
          if(color_of_node[x] == color_of_node[y]){
            ++edge_count_with_color[color_of_node[x]];
          }
        }
      }
    }

    std::vector<int>color_count(node_count, 0);
    for(int x=0; x<node_count; ++x){
      ++color_count[color_of_node[x]];
    }

    for(int i=0; i<node_count; ++i){
      count += (color_count[i]*(color_count[i]-1))/2 - edge_count_with_color[i];
    }

    return count;
  }

  int count_edits(const Graph&g){
    return count_inserts(g) + count_removals(g);
  }

  void print(std::ostream&out){
    const int node_count = color_of_node.size();

    for(int x=0; x<node_count; ++x){
      out << x << " " << color_of_node[x] << endl;
    }
  }
};

struct Algo{
  void init(const Graph&g){
    sol.init_with_singletons(g);

    const int node_count = g.neighbor_list.size();

    color_count.resize(node_count);
    for(int i=0; i<node_count; ++i){
      color_count[i] = 1;
    }

    neighbor_color_count.resize(node_count);
    for(int i=0; i<node_count; ++i){
      neighbor_color_count[i] = 0;
    }

    is_active_node.resize(node_count);

    color_order.resize(node_count);
    for(int i=0; i<node_count; ++i){
      color_order[i] = i;
    }
  }

  template<class RNG>
  void randomize_color_order(RNG&rng){
    std::shuffle(color_order.begin(), color_order.end(), rng);
  }

  void activate_all(const Graph&g){
    const int node_count = is_active_node.size();
    for(int i=0; i<node_count; ++i){
      is_active_node[i] = true;
    }

    active_node_list = color_order;
  }

  void unlock_color(int c){
    --color_count[c];
    if(color_count[c] == 0){
      free_color_list.push_back(c);
    }
  }

  void lock_color(int c){
    ++color_count[c];
  }

  int alloc_color(){
    int c = free_color_list.back();
    free_color_list.pop_back();
    color_count[c] = 1;
    return c;
  }

  bool move_node(const Graph&g, int x, int delete_weight, int insert_weight){

    int color_before = sol.color_of_node[x];
    if(color_count[color_before] != 1){
      unlock_color(sol.color_of_node[x]);
      sol.color_of_node[x] = alloc_color();
    }

    for(int y : g.neighbor_list[x]){
      ++neighbor_color_count[sol.color_of_node[y]];
    }

    int best_color = sol.color_of_node[x];
    int min_edit_count = delete_weight*g.neighbor_list[x].size();

    for(int y : g.neighbor_list[x]){
      int color = sol.color_of_node[y];
      int edit_count = delete_weight*g.neighbor_list[x].size() + insert_weight * color_count[color] - (delete_weight+insert_weight)*neighbor_color_count[color];
      if(edit_count < min_edit_count || (edit_count == min_edit_count && color_order[color] < color_order[best_color])){
        best_color = color;
        min_edit_count = edit_count;
      }
    }

    if(best_color != sol.color_of_node[x]){
      unlock_color(sol.color_of_node[x]);
      sol.color_of_node[x] = best_color;
      lock_color(best_color);
    }

    for(int y : g.neighbor_list[x]){
      --neighbor_color_count[sol.color_of_node[y]];
    }

    int color_after = sol.color_of_node[x];

    if(color_before != color_after){
      is_active_node[x] = true;
      active_node_list.push_back(x);
      for(int y : g.neighbor_list[x]){
        if(!is_active_node[y]){
          is_active_node[y] = true;
          active_node_list.push_back(y);
        }
      }
    }

    return color_before != color_after;
  }

  int pop_active_node(){
    int node = active_node_list.back();
    active_node_list.pop_back();
    is_active_node[node] = false;
    return node;
  }

  void run(const Graph&g, int delete_weight, int insert_weight){
    bool did_something;
    do{
      activate_all(g);
      did_something = false;
      while(!active_node_list.empty()){
        did_something |= move_node(g, pop_active_node(), delete_weight, insert_weight);
        if(time_limit_exceeded){
          return;
        }
      }
    }while(did_something && !time_limit_exceeded);
  }

  Clustering sol;

  vector<int>color_count;
  vector<int>neighbor_color_count;
  vector<int>free_color_list;

  vector<int>active_node_list;
  vector<bool>is_active_node;

  vector<int>color_order;

  int order_type = 0;
};

int main(int argc, char*argv[]){
  struct sigaction action;
  memset(&action, 0, sizeof(struct sigaction));
  action.sa_handler = signal_term_handler;
  sigaction(SIGTERM, &action, NULL);
  sigaction(SIGINT, &action, NULL);
 
  Graph g;

  g.input(cin);


  Clustering best_sol;
  best_sol.init_with_singletons(g);
  int best_sol_edit_count = best_sol.count_edits(g);

  Algo algo;
  
  auto test_for_improve = [&]{
    int new_count = algo.sol.count_edits(g);
    if(new_count < best_sol_edit_count){
      best_sol = algo.sol;
      best_sol_edit_count = new_count;
    }
  };

  algo.init(g);

  std::minstd_rand rng;

  int iter = 0;

  while(!time_limit_exceeded){
    ++iter;
    int type = (iter/4) % 100;
    if(type == 49){
      algo.run(g, 2, 1);
    }else if(type == 99){
      algo.run(g, 1, 2);
    }else{
      algo.run(g, 1, 1);
    }
    test_for_improve();
    algo.randomize_color_order(rng);
  }

  best_sol.output_edits(g, cout);
}

