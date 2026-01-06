#ifndef STUDENTAI_H
#define STUDENTAI_H
#include "AI.h"
#include "Board.h"
#include <random>
#include <cmath>
#include <limits>
#include <memory>
#include <utility>
#pragma once

//The following part should be completed by students.
//Students can modify anything except the class name and exisiting functions and varibles.
class StudentAI :public AI
{
public:
    Board board;
	StudentAI(int col, int row, int p);
	virtual Move GetMove(Move board);
	struct Node{
        Node* parent;
        vector<unique_ptr<Node>> children;
        int player;
        Move move;

        int visits = 0;
        int wins = 0;

        vector<vector<Move>> untriedMoves;

        Node(Node* parent, Move move, int player, Board& b):parent(parent), move(move), player(player){
            b.makeMove(move, player);
            untriedMoves = b.getAllPossibleMoves(player == 1?2:1); 
            b.Undo();
        };
    };
	vector<unique_ptr<Node>> root;


	// student methods
private:
	Move getRandomMove(vector<vector<Move>>& moves);
	int simulateGame(Board &b, int player);
	double calculateUTC(Node* node);
	vector<unique_ptr<Node>> nodify(vector<vector<Move>>& moves, int player, Board& b);
	Node* selection(const vector<unique_ptr<Node>>& root, Board& board_copy);
	Node* expand(Node* target, Board& board_copy);
	void backpropogate(Node* target, int winner);
	Move chooseBestMove(vector<unique_ptr<Node>>& nodes);
	void updateRoot(Move move, vector<unique_ptr<Node>>& root, Board& b);

	
};

template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

#endif //STUDENTAI_H
