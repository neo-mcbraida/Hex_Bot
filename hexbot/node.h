#ifndef NODE_H
#define NODE_H
#pragma once
#include <cmath>
#include <mutex>
#include <thread>
#include <syncstream>
#include <string_view>
#include "colors.h"
#include "grid.h" // Assuming this is the header file for HexGrid and COLOUR

struct Node {
private:
    std::mutex mtx;
    Node* parent;
    int visits;
    float AMAFScore;
    int AMAFVisits;
    const int raveConst = 50;
    //why do nodes need to know the grid?
    COLOUR agent;
    double totalReward;
    bool root;
    int depth;
    std::vector<Node*> children;
    Coord move;
    bool clean;

public:
    Node(COLOUR a);
    Node(COLOUR a, Coord c, Node* p);
    ~Node();
    void HealthyClean(Node* n);
    float GetAveReward();
    bool IsRoot() const;
    void SetRoot(bool r);
    float SecureScore() const;
    float UCB() const;
    int GetVisits() const;
    void IncrementVisits();
    void UpdateStep(float reward);
    void SetChildren(std::vector<Node*> ch);
    void UpdateReward(float reward);
    Node* GetParent() const;
    void SetParent(Node* p);
    void MakeRoot();
    float GetRaveScore() const;
    float GetAMAF() const;
    void IncrementAMAFVisits();
    void UpdateAMAFScore(float reward);
    //what is the original defintion for here
    // const std::vector<Node*>& GetChildren() const;
    int GetAMAFVisits() const;
    std::vector<Node*> GetChildren();
    void AddChild(Node* child);
    COLOUR GetAgent() const;
    void SetMove(Coord coord);
    int GetDepth();
    Coord GetMove() const;
};
#endif // NODE_H
