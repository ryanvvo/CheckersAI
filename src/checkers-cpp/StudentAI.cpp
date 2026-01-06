#include "StudentAI.h"
#include <random>
#include <cmath>
#include <limits>
#include <memory>
#include <utility>

const int ITERATIONS = 250;
const double UTC_EXP = 1.414;

//The following part should be completed by students.
//The students can modify anything except the class name and exisiting functions and varibles.
StudentAI::StudentAI(int col,int row,int p)
	:AI(col, row, p)
{
    board = Board(col,row,p);
    board.initializeGame();
    player = 2;
}

Move StudentAI::GetMove(Move move)
{ // we are probably changing this

    if (move.seq.empty())
    { // if empty move, we are player 1 and make first move
        player = 1;
    } else{ // otherwise, match the opponent's last move onto the board
        board.makeMove(move,player == 1?2:1);
    }

    vector<vector<Move> > moves = board.getAllPossibleMoves(player);

    // implementation
    // if no tree yet, initialize the tree using current moves
    if (root.empty()) // if no tree yet
        root = this->nodify(moves, player, board); // turn all current moves into node heads
    else{ //if tree existed
        // clear up old nodes and update so root is at new moves
        this->updateRoot(move, root, board);
    }
    // apply MCTS an ITERATIONS amount of times
    Node* target;
    Node* new_target;
    int winner;
    for (int i = 0; i < ITERATIONS; ++i){
        Board board_copy = board;    
        // selection: use UTC alg to select a leaf node
        target = this->selection(root, board_copy);
        // expansion: give leaf node its moves
        new_target = this->expand(target, board_copy);
        // simulation: simulate the rest of the game
        if (new_target != nullptr) // If there exists something to expand
            target = new_target;
        int winner = this->simulateGame(board_copy, target->player);
        // backtrack: record results and back track it
        this->backpropogate(target, winner);
    }
    Move res = chooseBestMove(root);
    board.makeMove(res,player); // match move onto board
    this->updateRoot(res, root, board);
    return res;
}

Move StudentAI::getRandomMove(vector<vector<Move>>& moves){
    // Chooses a random move.
    if (moves.empty()){
        cout << "getRandomMove given no moves." << endl;
        throw 5;
    }
    int i = rand() % (moves.size());
    const auto& checker_moves = moves[i];
    if (checker_moves.empty()){
        cout << "getRandomMove given moves with empty row." << endl;
        throw 5;
    }
    int j = rand() % (checker_moves.size());
    return checker_moves[j];
}

int StudentAI::simulateGame(Board &b, int player){
    // Simulates a random game from the move, returning the player who won.
    int capture1 = 0;
    int capture2 = 0;
    int moves_since_last_capture = 0;

    while (b.isWin(player) == 0 && abs(capture1-capture2) < 3 && moves_since_last_capture < 15){ // end game if win or a player has 3 more captures or 15 moves since last capture
        auto moves = b.getAllPossibleMoves(player);
        if (moves.empty()) break;

        if (moves[0][0].isCapture()){
            (player == 1) ? ++capture1 : ++capture2;
            moves_since_last_capture = 0;
        }
        else
            ++moves_since_last_capture;

        b.makeMove(this->getRandomMove(moves), player);
        player = (player == 1)? 2:1; // swaps player
    }

    if (capture1 - capture2 >= 3) // if 1 captured 3 more, return their win
        return 1;
    else if (capture2 - capture1 >= 3)
        return 2;

    if (moves_since_last_capture >= 15)
        if (capture1 > capture2) return 1;
        else if (capture2 > capture1) return 2;
        else return this->player;

    if (b.isWin(player) == -1) return this->player; // any tie is win for us
    return b.isWin(player);
}

double StudentAI::calculateUTC(Node* node){
    // Returns the result using the UTC formula (keep mind of the UTC_EXP constant up top.)
    if (node->visits == 0)
        return numeric_limits<double>::infinity();


    double exploitation = static_cast<double>(node->wins) / node->visits;

    double exploration = 0.0;
    if (node->parent != nullptr)
        exploration = UTC_EXP * sqrt(log(node->parent->visits) / node->visits);

    return exploitation + exploration;
}

vector<unique_ptr<StudentAI::Node>> StudentAI::nodify(vector<vector<Move>>& moves, int player, Board& b){
    // Given moves, returns a vector of nodes that represent the moves.
    if (moves.empty()){
        cout << "nodify with empty moves" << endl;
        throw 5;
    }
    vector<unique_ptr<Node>> res;
    for (int i = 0; i < moves.size(); ++i)
        for (int j = 0; j < moves[i].size(); ++j)
            res.push_back(make_unique<Node>(nullptr, moves[i][j], player, b));
 
    return move(res);
}

StudentAI::Node* StudentAI::selection(const vector<unique_ptr<Node>>& root, Board& board_copy){
    // searches the tree and returns a node that has unexplored moves.
    if (root.empty()){
        cout << "selection: empty root" << endl;
        throw 5;
    }
    Node* curr_node = root[0].get();
    for (const auto& head: root)
        if (this->calculateUTC(curr_node) < this->calculateUTC(head.get()))
            curr_node = head.get();
    
    board_copy.makeMove(curr_node->move, curr_node->player);

    if (!curr_node->untriedMoves.empty() || curr_node->children.empty())
        return curr_node; // return node if it can be expanded or is a leaf
    else
        return selection(curr_node->children, board_copy); // fully expanded, recurse into children
}

StudentAI::Node* StudentAI::expand(Node* target, Board& board_copy){
    // Expands a random untried move and returns a pointer to that move.
    int i = target->untriedMoves.size();
    if (i == 0){
        return nullptr; // NOTHING TO EXPAND
    } 
    i = rand() % i;

    int j = target->untriedMoves[i].size();
    if (j == 0) {
        cout <<  "expand: empty untriedMoves row" << endl;
        throw 5;
    }
    j = rand() % j;

    Move mv = target->untriedMoves[i][j];

    target->untriedMoves[i].erase(target->untriedMoves[i].begin() + j);
    if (target->untriedMoves[i].empty())
        target->untriedMoves.erase(target->untriedMoves.begin() + i);

    auto child = make_unique<Node>(target, mv, target->player==1?2:1, board_copy); 
    
    board_copy.makeMove(mv, child->player);
    Node* childPtr = child.get(); 
    target->children.push_back(move(child));
    return childPtr;
}

void StudentAI::backpropogate(Node* target, int winner){
    // Backpropogates the win/loss to parent nodes.
    while (target != nullptr) {
        target->visits += 1;
        if (target->player == winner)    // if the player at this node won
            target->wins += 1;
        target = target->parent;  // go up the tree
    }
}

Move StudentAI::chooseBestMove(vector<unique_ptr<Node>>& nodes){
    // Chooses the best move based on win rate.
    if (nodes.empty()){
        cout << "chooseBestMove: No move.";
        throw 5;
    }
    Node* curr_node = nodes[0].get();
    for (const auto& node: nodes){
        if (node->visits == 0) continue;
        if (static_cast<double>(node->wins)/node->visits > static_cast<double>(curr_node->wins)/curr_node->visits) // choose highest winrate
            curr_node = node.get();
    }
    
    return curr_node->move;
}

void StudentAI::updateRoot(Move mv, vector<unique_ptr<Node>>& root, Board& b){
    // Updates the root to be on the current available moves.
    if (root.empty()){
        cout << "empty root" << endl;
        throw 5;
    }
    vector<unique_ptr<Node>> temp;
    Node* target;
    for (auto& node: root)
        if (node->move.toString() == mv.toString()){
            target = node.get();
            break;
        }
    
    if (!target->untriedMoves.empty()){
        temp = this->nodify(target->untriedMoves, (target->player==1?2:1), b);
        target->untriedMoves.clear();

    }
    for (auto& node: target->children)
        temp.push_back(move(node));

    root.clear();
    for (auto& node: temp){
        node->parent = nullptr;
        root.push_back(move(node));
    }
}