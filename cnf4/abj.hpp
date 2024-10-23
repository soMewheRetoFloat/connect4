#pragma once
#include "Judge.h"
#include "Point.h"
#include <iostream>
#include "Strategy.h"
#include <algorithm>

#define NEGATIVE_INF -1000007;
#define POSITIVE_INF 1000007;

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

int evaluate(int x, int y, int M, int N, int **board, int *top, const int noX, const int noY, bool is_max_floor) {
	int score = 0;
	if (is_max_floor) { // 最大层 机器方
		if (userWin(x, y, M, N, board)) { // g
			score -= 1000;
		}
		if (machineWin(x, y, M, N, board)) { // 6
			score += 1000;
		}
	}
	else { // 最小层 用户方
		if (userWin(x, y, M, N, board)) { // 6
			score += 1000;
		}
		if (machineWin(x, y, M, N, board)) { // g
			score -= 1000;
		}
	}

	int place_num = int(is_max_floor) + 1; // 如果是最大层 那么这里应该是2 否则应该是1

	bool* varified = new bool[M * N];
	memset(varified, 0, M * N * sizeof(bool));

	int mid = (N - 1) / 2;

	// 中间的权值大
	int* weight_col = new int[N];
	for (int i = 0; i < N; ++i) {
		if (i <= mid) {
			weight_col[i] = i + 1;
		}
		else {
			weight_col[i] = weight_col[N - 1 - i];
		}
	}

	for(int col = 0; col < N; ++col) {
		for (int row = M - 1; row < 0; --row) {
			if (board[row][col] == 0) {
				// 测试威胁性
				int weight_tmp = row * weight_col[col];
				// 对于威胁性 下点权值大 向上权值小
				int up_row_left, down_row_left, up_row_left, down_row_left, left_col, right_col;
				int connection = 0, connection_left, connection_right;
				int chesstype = int(is_max_floor) + 1;

				// 横的斜的都要看 竖的没意义
				for (left_col = col - 1; left_col >= 0 && board[row][left_col] == chesstype; --left_col) {
					++connection;
				}
				for (right_col = col + 1; right_col < N && board[row][right_col] == chesstype; ++left_col) {
					++connection;
				}
				if (connection >= 3) {
					score += weight_tmp;
					continue;
				}

				connection = 0;

			}
			else {

			}
		}
	}

	return score;
}

int minmax(int xtd, int ytd, int M, int N, int **board, int *top, const int noX, const int noY, int step, bool is_max_floor, int alpha, int beta) {
	if (step == 0) {
		return evaluate(xtd, ytd, M, N, board, top, noX, noY, is_max_floor);
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
				minmax_sum = max(minmax_sum, minmax(row, col, M, N, board, top, noX, noY, step - 1, !is_max_floor, alpha, beta));
				
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
				minmax_sum = min(minmax_sum, minmax(row, col, M, N, board, top, noX, noY, step - 1, !is_max_floor, alpha, beta));
				
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
Point ab_decide(int M, int N, int **board, int *top, const int noX, const int noY, int step) {
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
		int tmp = minmax(row, col, M, N, board, top, noX, noY, step - 1, true, NEGATIVE_INF, POSITIVE_INF);
		if (minmax_sum <= tmp) {
			minmax_sum = tmp;
			x = row;
			y = col;
		}
		board[row][col] = 0;
		top[col] = tmp_top;
	}
	return Point(x, y);
}