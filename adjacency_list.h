#ifndef ADJACENCY_LIST_H
#define ADJACENCY_LIST_H

#include<iostream>
#include<vector>
#include<cassert>
#include"inputneure.h"
#include"outputneure.h"
#include"hidden_layer.h"
#include"neure.h"
#include"transport.h"
#include<queue>
#include<QDebug>
#define MAX 30
using namespace std;

class Adjacency_List
{
    struct node{
        float out=0;      // 单条边的输出
        int weight=0;
        int key;            //节点的标号
        int indegree=0;
        float in=0;
    };
protected:
    int n=0, m=0 ;//n是节点数 m是边数

    vector<node>k;
    vector<transport> tr;        //传输的边
    vector<hidden_layer> hidden; //隐层
    vector<InputNeure> Inputne;  //输入层
    vector<OutputNeure> Outne; //输出层

public:
    Adjacency_List();
    Adjacency_List(int n,int m);
    void create_graph();
    void addEdge(transport t);  //增加边
    void delEdge(transport m);  //删除边
    void addvertax(int key);    //增加顶点
    void delvertax(int key);    //删除顶点
    void TopSort();  //拓扑排序
    int getvertax();            //返回顶点个数
    int getEdge();              //返回边个数
    void clrdesk();             //清空
    void addhidden(hidden_layer Hneure);           //隐层
    void addInput(InputNeure Ineure);            //输入层
    void addOutput(OutputNeure Oneure);           //输出层

    void retInput(vector<InputNeure>::iterator iterIn);
    void retHidden(vector<hidden_layer>::iterator iterHidden);
    void retOutput(vector<OutputNeure>::iterator iterOut);

    void inInput(vector<InputNeure>::iterator it);
    void inhidden(vector<hidden_layer>::iterator it);
    void intransport(vector<transport>::iterator it);



};

#endif // ADJACENCY_LIST_H
