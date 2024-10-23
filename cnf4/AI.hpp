#pragma once
#define NOTYET double(-97)
#include "Point.h"
#include "Judge.h"
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <iostream>
#include <random>

using std::cerr;
using std::endl;

std::random_device rd;
std::mt19937 gen(rd());

const double time_limit = 2 * CLOCKS_PER_SEC;

int rand_with_weight(int num_choices, int* weights, int total) {
   int rnd = int(gen()) % total;
   rnd = (rnd + total) % total;
   for (int i = 0; i < num_choices; ++i) {
      if (rnd < weights[i]) {
         return i;
      }
      rnd -= weights[i];
   }
   return (num_choices - 1) / 2;
}

class Node {
 public:
   int total_row; // 总行数
   int total_col; // 总列数
   int** board = nullptr; // 棋盘
   int* top = nullptr; // 顶
   int noX, noY; // 禁制
   int place_x, place_y; // 下棋点
   int site; // 是否是己方
   int layer; // 所在层数

   Node** children = nullptr; // 子
   Node* parent = nullptr; // 父
   int expandable_num = 0; // 可扩展节点数 为0代表无法扩展
   int* expandable_index; // 可扩展的位置
   double winRate = 0.0; // 胜率
   int visited_cnt = 0; // 被访问次数


   Node(int _row, int _col, int** _board, int* _top, int _noX, int _noY,
   int p_x = -1, int p_y = -1, bool _site = false, Node* _parent = nullptr, int _layer = 1):
   total_row(_row),
   total_col(_col),
   noX(_noX),
   noY(_noY),
   site(_site),
   place_x(p_x),
   place_y(p_y),
   parent(_parent),
   layer(_layer),
   expandable_num(0)
   {
      top = new int[_col];
      for (int i = 0; i < _col; ++i) {
         top[i] = _top[i];
      }

      board = new int*[_row];
      for (int i = 0; i < _row; ++i) {
         board[i] = new int[_col];
         for (int j = 0; j < _col; ++j) {
            board[i][j] = _board[i][j];
         }
      }

      children = new Node* [_col];
      expandable_index = new int[_col];
      for (int i = 0; i < _col; ++i) {
         children[i] = nullptr;
         if (top[i]) {
            expandable_index[expandable_num++] = i;
         }
      }
   }

   ~Node() {
      for (int i = 0; i < total_col; ++i) {
         if (children[i]) delete children[i];
      }
      delete[] children;
      delete[] top;
      delete[] expandable_index;
      for (int i = 0; i < total_row; ++i) {
         delete[] board[i];
      }
      delete[] board;
   }
};

class UCT {
 public:
   int total_row; // 总行数
   int total_col; // 总列数
   int noX, noY; // 禁制
   int* col_weight; // 列权重，棋是按照列下的
   int total_weight; // 权重和
   Node* root; // 根节点

   UCT(int _row, int _col, int** _board, const int* _top, int _noX, int _noY):
   total_row(_row),
   total_col(_col),
   noX(_noX),
   noY(_noY)
   {
      col_weight = new int[_col];
      int *tp_tmp = new int[_col];
      for (int i = 0; i < _col; ++i) {
         tp_tmp[i] = _top[i];
      }
      
      root = new Node(_row, _col, _board, tp_tmp, _noX, _noY);

      delete[] tp_tmp;

      // 分段赋值 根据我的下棋经验 没人一开始下角上 因此中间大
      int mid = (total_col - 1) / 2;
      total_weight = (total_col % 2) ? (mid + 1) * (mid + 1) : (mid + 1) * (mid + 2);

      for (int i = 0; i <= mid; ++i) {
         col_weight[i] = i + 1;
      }

      for (int i = mid + 1; i < _col; ++i) {
         col_weight[i] = col_weight[_col - i - 1];
      }
   }

   ~UCT() {
      delete root;
      delete[] col_weight;
   }

   Node* UctSearch() {
      cerr << root->expandable_num << endl;
      clock_t startTime = clock();
      while(clock() - startTime < time_limit) {
         Node* next = TreePolicy(root);
         double wR = DefaultPolicy(next);
         // backup
         while(next) {
            ++(next->visited_cnt);
            (next->winRate) += wR * exp(double(1.0) / double(5 * next->layer));
            next = (next -> parent);
         }
      }
      cerr << "sch end" << endl;
      return BestChild(root, 0.0);
   }

   bool nodeEnd(Node* v) { // 判断一个节点是否为终止节点
      return isTie(v->total_col, v->top) || 
      (!(v->site) && userWin(v->place_x, v->place_y, v->total_row, v->total_col, v->board)) ||
      ((v->site) && machineWin(v->place_x, v->place_y, v->total_row, v->total_col, v->board));
   }

   Node* TreePolicy(Node* v) {
      bool policy_end = false;
      while(true) {
         if (!v -> parent) { // 根节点 必不可能结束
            policy_end = false;
         }
         else if (nodeEnd(v)) { // 终止节点
            policy_end = true;
         }

         if (policy_end) break;
         else { // 非终止非根
            if ((v->expandable_num) > 0) { // 可扩展就扩展
               return Expand(v);
            }
            else { // 不可扩展就下层
               v = BestChild(v);
            }
         }
      }
      return v;
   }

   double GameState(int x, int y, int row, int col, int** board, int* top, bool site) {
      if (isTie(col, top)) return double(0);
      else if (site && machineWin(x, y, row, col, board)) return double(1);
      else if (!site && userWin(x, y, row, col, board)) return double(-1);
      else return NOTYET;
   }

   double DefaultPolicy(Node* v) {
      double ret = 0.0;
      int* _top = new int[total_col];
      for (int i = 0; i < total_col; ++i) {
         _top[i] = v->top[i];
      }

      int** _board = new int*[total_row];
      for (int i = 0; i < total_row; ++i) {
         _board[i] = new int[total_col];
         for (int j = 0; j < total_col; ++j) {
            _board[i][j] = v->board[i][j];
         }
      }

      bool p_site = v->site;
      int x = v->place_x;
      int y = v->place_y;
      ret = GameState(x, y, total_row, total_col, _board, _top, p_site);

      while (ret == NOTYET) {
         p_site = !p_site;
         // 加权随机数
         while(true) {
            y = rand_with_weight(total_col, col_weight, total_weight);
            // y = (int(gen()) % total_col);
            // y = (y + total_col) % total_col;
            // 有效数
            if (_top[y] > 0) break;
         }
         int x = --(_top[y]);
         //cerr << "get d_p col" << x << " " << y << endl;
         if (x - 1 == noX && y == noY) {
            --(_top[y]);
         }
         _board[x][y] = int(p_site) + 1;
         ret = GameState(x, y, total_row, total_col, _board, _top, p_site);
      }

      delete[] _top;
      for (int i = 0; i < total_row; ++i) {
         delete[] _board[i];
      }
      delete[] _board;
      return ret;
   }

   Node* Expand(Node* v) {
      int* _top = new int[total_col];
      for (int i = 0; i < total_col; ++i) {
         _top[i] = v->top[i];
      }

      int** _board = new int*[total_row];
      for (int i = 0; i < total_row; ++i) {
         _board[i] = new int[total_col];
         for (int j = 0; j < total_col; ++j) {
            _board[i][j] = v->board[i][j];
         }
      }
      
      int rand_expand = (int(gen()) % (v->expandable_num));
      rand_expand = (rand_expand + (v->expandable_num)) % (v->expandable_num);

      int pl_y = v->expandable_index[rand_expand];
      int pl_x = --(_top[pl_y]);
      //cerr << "get expand col" << pl_x << " " << pl_y << endl;
      if (pl_x - 1 == noX && pl_y == noY) {
         --(_top[pl_y]);
      }
      // 子节点下的是对手棋
      bool p_site = v->site;
      _board[pl_x][pl_y] = 2 - (p_site);
      v->children[pl_y] = new Node(total_row, total_col, _board, _top,
         noX, noY, pl_x, pl_y, !p_site, v, (v->layer + 1));

      delete[] _top;
      for (int i = 0; i < total_row; ++i) {
         delete[] _board[i];
      }
      delete[] _board;


      // 将该节点方向置为不可扩展
      std::swap(v->expandable_index[rand_expand], v->expandable_index[--(v->expandable_num)]);
      return (v->children[pl_y]);
   }

   Node* BestChild(Node* parent, const double c = 0.65) { // return argmax(child_uct1)
      double cur = -9e7;
      Node* best = nullptr;
      if (c == 0.0) {
         cerr << "root have " << parent->expandable_num << endl;
      }
      for (int i = 0; i < parent->total_col; ++i) {
         if (parent->children[i]) {
            Node* now_node = parent->children[i];
            double st = (parent->site ? -1.0 : 1.0);
            double avr_wr = now_node->winRate / double(now_node->visited_cnt);
            double sqrt_para = sqrt(2 * log(double(parent->visited_cnt)) / double(now_node->visited_cnt));
            // 套公式 uct1 = V + c * sqrt_param
            double trust = st * avr_wr + c * sqrt_para;
            if (c == 0) {
               cerr << "UCT: " << trust << endl;
               cerr << "visited: " << (now_node->visited_cnt) << endl;
            }
            if (trust >= cur) {
               best = now_node;
               cur = trust;
            }
         }
      }
      return best;
   }

};