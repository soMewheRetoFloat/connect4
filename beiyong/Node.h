#include <unistd.h>

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <vector>
#include <queue>

#include "Judge.h"
#include "Point.h"
using namespace std;
#pragma once

#define NEGATIVE_INF -1000007;
#define POSITIVE_INF 1000007;

class Node {
    // 节点只需要计本次下的点、分数、alpha、beta、归属、上下亲即可
 public:
    int lower_bound; // alpha
    int upper_bound; // beta
    int pos_x, pos_y; // 本次下的位置
    bool my_chance; // 本次是否是己方决策
    int score; // 评价分数
    Node* parent = nullptr;
    vector<Node*> children;

    Node(bool my_chance, int x = -1, int y = -1, Node* parent == nullptr) {
        this->pos_x = x;
        this->pos_y = y;
        this->my_chance = my_chance;
        this->parent = parent;
        this->lower_bound = NEGATIVE_INF;
        this->upper_bound = POSITIVE_INF;
        this->score = 0;
    }
};