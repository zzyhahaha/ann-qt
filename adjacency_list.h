
#ifndef ADJACENCY_LIST_H
#define ADJACENCY_LIST_H

#include<iostream>
#include<vector>
#include<cassert>
#include"inputneure.h"
#include"outputnerue.h"
#include"hidden_layer.h"
#include"neure.h"
#include"transport.h"
#include<queue>
#define MAX 1000
using namespace std;

class Adjacency_List
{
    struct node{
        int out;      // 单条边的输出
        int to;
        int weight;
        int key;            //节点的标号
        int indegree;
        int from;
        int in;
    };
private:
    int n, m ;   //n是节点数 m是边数
    vector<node> k[MAX];
    vector<transport> tr;        //传输的边
public:
    Adjacency_List();
    Adjacency_List(int n,int m);
    void create_graph();
    void addEdge(transport t);  //增加边
    void delEdge(transport m);  //删除边
    void addvertax(int key);    //增加顶点
    void delvertax(int key);    //删除顶点
    void TopSort(Adjacency_List &graph,int key,int qiflag);  //拓扑排序
    int getvertax();            //返回顶点个数
    int getEdge();              //返回边个数

};

#endif // ADJACENCY_LIST_H
