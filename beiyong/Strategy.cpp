#pragma once
#include <iostream>
#include <unistd.h>
#include "Point.h"
#include "Strategy.h"
#include "Judge.h"
#include "Node.h"

using namespace std;

/*
	策略函数接口,该函数被对抗平台调用,每次传入当前状态,要求输出你的落子点,该落子点必须是一个符合游戏规则的落子点,不然对抗平台会直接认为你的程序有误
	
	input:
		为了防止对对抗平台维护的数据造成更改，所有传入的参数均为const属性
		M, N : 棋盘大小 M - 行数 N - 列数 均从0开始计， 左上角为坐标原点，行用x标记，列用y标记
		top : 当前棋盘每一列列顶的实际位置. e.g. 第i列为空,则_top[i] == M, 第i列已满,则_top[i] == 0
		_board : 棋盘的一维数组表示, 为了方便使用，在该函数刚开始处，我们已经将其转化为了二维数组board
				你只需直接使用board即可，左上角为坐标原点，数组从[0][0]开始计(不是[1][1])
				board[x][y]表示第x行、第y列的点(从0开始计)
				board[x][y] == 0/1/2 分别对应(x,y)处 无落子/有用户的子/有程序的子,不可落子点处的值也为0
		lastX, lastY : 对方上一次落子的位置, 你可能不需要该参数，也可能需要的不仅仅是对方一步的
				落子位置，这时你可以在自己的程序中记录对方连续多步的落子位置，这完全取决于你自己的策略
		noX, noY : 棋盘上的不可落子点(注:涫嫡饫锔?龅膖op已经替你处理了不可落子点，也就是说如果某一步
				所落的子的上面恰是不可落子点，那么UI工程中的代码就已经将该列的top值又进行了一次减一操作，
				所以在你的代码中也可以根本不使用noX和noY这两个参数，完全认为top数组就是当前每列的顶部即可,
				当然如果你想使用lastX,lastY参数，有可能就要同时考虑noX和noY了)
		以上参数实际上包含了当前状态(M N _top _board)以及历史信息(lastX lastY),你要做的就是在这些信息下给出尽可能明智的落子点
	output:
		你的落子点Point
*/
extern "C" Point *getPoint(const int M, const int N, const int *top, const int *_board,
						   const int lastX, const int lastY, const int noX, const int noY)
{
	/*
		不要更改这段代码
	*/
	int x = -1, y = -1; //最终将你的落子点存到x,y中
	int **board = new int *[M];
	for (int i = 0; i < M; i++)
	{
		board[i] = new int[N];
		for (int j = 0; j < N; j++)
		{
			board[i][j] = _board[i * N + j];
		}
	}

	/*
		根据你自己的策略来返回落子点,也就是根据你的策略完成对x,y的赋值
		该部分对参数使用没有限制，为了方便实现，你可以定义自己新的类、.h文件、.cpp文件
	*/
	//Add your own code below

	

	//a naive example 只下到第一个
	// for (int i = N - 1; i >= 0; --i) {
	// 	if (top[i] > 0) {
	// 		x = top[i] - 1;
	// 		y = i;
	// 		break;
	// 	}
	// }
	Point ret = decide(M, N, board, top, lastX, lastY, noX, noY, 5);
	/*
		不要更改这段代码
	*/
	clearArray(M, N, board);
	return ret;
}

/*
	getPoint函数返回的Point指针是在本so模块中声明的，为避免产生堆错误，应在外部调用本so中的
	函数来释放空间，而不应该在外部直接delete
*/
extern "C" void clearPoint(Point *p)
{
	delete p;
	return;
}

/*
	清除top和board数组
*/
void clearArray(int M, int N, int **board)
{
	for (int i = 0; i < M; i++)
	{
		delete[] board[i];
	}
	delete[] board;
}

bool inBound(int M, int N, int x, int y) { // 点是否在棋盘内
	return (x >= 0 && x < M && y >= 0 && y < N);
}

bool canPlace(int M, int N, int **board, int *top, int x, int y, const int noX, const int noY) {
	// 禁止下点不能下
	if (x == noX && y == noY) return false;
	// 底下没垫不能下
	if (top[y] != x + 1) return false;
	// 下过的地方不能下
	if (board[x][y] != 0) return false;
	// 边界外不能下
	return inBound(M, N, x, y);
}

int evaluate(int x, int y, int M, int N, int **board, int *top, const int noX, const int noY) {

	if (userWin(x, y, M, N, board)) {
		return NEGATIVE_INF - 100;
	}
	if (machineWin(x, y, M, N, board)) {
		return POSITIVE_INF + 100;
	}

	for(int row = 0; row < M; ++row) {
		for(int col = 0; col < N; ++col) {
			// 分情况看，这里是空的还是有子的
			if (canPlace(M, N, board, top, row, col, noX, noY)) {
				
			}
			else {

			}
		}
	}
}

int minmax(int xtd, int ytd, int M, int N, int **board, int *top, const int noX, const int noY, int step, int is_max_floor, int alpha, int beta) {
	if (step == 0) {
		return evaluate(xtd, ytd, M, N, board, top, noX, noY);
	}
	else {
		int minmax_sum;
		if (is_max_floor) { // 如果是max层
			minmax_sum = NEGATIVE_INF;
			for (int col = 0; col < N; ++col) {
				int tmp_top = top[col];
				int row = top[col] - 1;
				if (row < 0) break;
				// 下出这一步
				top[col] = row;
				board[row][col] = 2;

				// 取最优情况
				minmax_sum = max(mimax_sum, minmax(row, col, M, N, board, top, noX, noY, step - 1, !is_max_floor, alpha, beta));
				
				// 回溯
				board[row][col] = 0;
				top[col] = tmp_top;
				if (minmax_sum > 0) {
					return minmax_sum;
				}
			}
		}
		else { // 如果是min层
			minmax_sum = POSITIVE_INF;
			for (int col = 0; col < N; ++col) {
				int tmp_top = top[col];
				int row = top[col] - 1;
				if (row < 0) break;

				// 下棋
				top[col] = row;
				board[row][col] = 1;

				// 取最劣势情况
				minmax_sum = min(mimax_sum, minmax(row, col, M, N, board, top, noX, noY, step - 1, !is_max_floor, alpha, beta));
				
				// 回溯
				board[row][col] = 0;
				top[col] = tmp_top;
				if (minmax_sum > 0) {
					return minmax_sum;
				}
			}
		}
	}
}

// 第一层决策一定是max层
Point decide(int M, int N, int **board, int *top, const int noX, const int noY, int step) {
	int x = -1;
	int y = -1;
	int minmax_sum = NEGATIVE_INF;
	for (int col = 0; col < N; ++col) {
		int tmp_top = top[col];
		int row = top[col] - 1;
		if (row == noX && col == noY) {
			row -= 1;
		}
		if (row < 0) break;
		top[col] = row;
		board[row][col] = 2;
		int tmp = minmax(row, col, M, N, board, top, noX, noY, step - 1, 1, NEGATIVE_INF, POSITIVE_INF);
		if (minmax_sum <= tmp) {
			minmax_sum = tmp;
			x = row;
			y = col;
		}
		board[row][col] = 0;
		top[col] = tmp_top;
		if (minmax_sum > 0) {
			return minmax_sum;
		}
	}
	return new Point(x, y);
}