#include "node.h"
#include "mcts.h"
#include "colors.h"
#include "decisions.h"
#include <iostream>

Node::Node(COLOUR a){
    visits=0;
    totalReward=0.0;
    agent=a;
    AMAFScore = 0;
    AMAFVisits = 0;
    root=false;
    parent=nullptr;
    depth = 0;
}

Node::Node(COLOUR a, Coord c, Node* p) {
    visits = 0;
    totalReward = 0.0;
    agent = a;
    AMAFScore = 0;
    AMAFVisits = 0;
    root = false;
    parent = p;
    move = c;
    depth = p->GetDepth() + 1;
}

Node::~Node() {
    if(!clean){
        for (Node* child : children) {
            delete child;
        }
    }
}

//TODO return partial tree, itegrate into node
void Node::HealthyClean(Node* keep){
	for (Node* n : GetChildren()) {
        if(keep!=n){
            delete n;
        }
	}
    clean=true;
}

float Node::UCB() const{ // C is set to 2, a hyperparameter for MCTS
    double denom = (visits == 0) ? 1.0 : visits;
    //float d = 1.0;
    // denom * 100 because it should be total wins over denom, and a win is worth 100

    float confidence = totalReward / (denom * WIN_REWARD);
    float chance = MCTSC  * (std::sqrt(std::log(parent->GetVisits()) / denom));
    if(agent==BLUE){
        return confidence - chance;//2 * parent->GetVisits();
    }else{
        return confidence + chance;//2 * parent->GetVisits();
    }
}

float Node::SecureScore() const{ // C is set to 2, a hyperparameter for MCTS
    double denom = (visits == 0) ? 1.0 : visits;
    // loat d = 1.0;
    // denom * 100 because it should be total wins over denom, and a win is worth 100
    float confidence = totalReward / (denom * WIN_REWARD);
    float chance = MCTSC  * (std::sqrt(std::log(parent->GetVisits()) / denom));
    if(agent==RED){
        return confidence - chance;//2 * parent->GetVisits();
    }else{
        return confidence + chance;//2 * parent->GetVisits();
    }
}

void Node::UpdateStep(float reward) {
    std::lock_guard<std::mutex> lock(mtx);
    UpdateReward(reward);
    IncrementVisits();
}

void Node::SetChildren(std::vector<Node*> ch) {
    std::lock_guard<std::mutex> lock(mtx);
    if (children.size() == 0) {
        children = ch;
    }
}

std::vector<Node*> Node::GetChildren() {
    std::lock_guard<std::mutex> lock(mtx);
    return children;
}

float Node::GetAMAF() const {
    return (AMAFVisits == 0) ? 0 : AMAFScore / AMAFVisits;
}

float Node::GetRaveScore() const {
    float a = static_cast<float>(raveConst - visits) / raveConst;
    float beta = (a < 0) ? 0 : a;
    return (1 - beta) * UCB() + beta * GetAMAF();
}



//simple functions for access reasons?
float Node::GetAveReward() {return  (visits == 0) ? 0 : totalReward/visits; }
void Node::UpdateReward(float reward) { totalReward += reward; }
void Node::MakeRoot() {root = true;parent=nullptr;}
void Node::SetRoot(bool r) {root = r;}

void Node::UpdateAMAFScore(float reward) { AMAFScore += reward; }
void Node::IncrementAMAFVisits() { AMAFVisits++; }
int Node::GetDepth() { return depth; }
bool Node::IsRoot() const {return root;}
int Node::GetVisits() const {return visits;}
int Node::GetAMAFVisits() const { return AMAFVisits; }
void Node::IncrementVisits() {visits++;}
Node* Node::GetParent() const {return parent;}
void Node::SetParent(Node* p) {parent = p;}
void Node::AddChild(Node* child) {children.push_back(child);}
COLOUR Node::GetAgent() const {return agent;}
void Node::SetMove(Coord coord) {move = coord;}
Coord Node::GetMove() const {return move;}
