#include "adjacency_list.h"

Adjacency_List::Adjacency_List()
{
    k->clear();
    tr.clear();
    this->n=0;
    this->m=0;
}

Adjacency_List::Adjacency_List(int n,int m){
    this->n=n;
    this->m=m;
    k->clear();
    tr.clear();
}

void Adjacency_List::addEdge(transport t){
    node temp;
    tr.push_back(t);
    temp.from=t.getfrom();
    temp.to=t.getto();
    temp.weight=t.getWeight();
    for( vector<node>:: iterator iter=k->begin();iter!=k->end();)
    {
        if(iter->key==t.getto())
        {
            iter->indegree++;
        }
        else{
            iter++;
        }
    }
    k[temp.from].push_back(temp);
    m++;//边数加1

}  //增加边

void Adjacency_List::delEdge(transport t){
/****************删除链接的结点*********************/
    for(vector<node>::iterator iter=k->begin();iter!=k->end();){
        /*将其所对应的下一个节点入度减1*/
        if(iter->key==t.getto()){
            iter->indegree--;
        }
        else
            iter++;
    }
        /*将其所在的行消除*/
    for(vector<node>::iterator iter=k->begin();iter!=k->end();){
        if(iter->key==t.getfrom()){
            k[t.getfrom()].erase(iter);
        }
        else
            iter++;
    }
/**************************删除存入VECTOR中的边************************/
    for(vector<transport>::iterator iter=tr.begin();iter!=tr.end();){
        if(iter->getkey() == t.getkey()){
            tr.erase(iter);
        }
      else {
             iter++;
            }
      }

}//删除边


void Adjacency_List:: addvertax(int key){
    node temp;
    temp.key=key;
    k->push_back(temp);
    n++;//顶点数加1

}    //增加顶点

void Adjacency_List::delvertax(int key){
    for(vector<node>::iterator iter=k->begin();iter!=k->end();){
        if(iter->key==key)
            k->erase(iter);
        else
            iter++;
    }

}    //删除顶点


void Adjacency_List::TopSort(Adjacency_List &graph,int key,int qiflag){//启动flag
    int cnt=0; //判断是否有环
    queue<node> q;
    /*********找入度为0的点并将他们压入队列*********/
    for(vector<node>::iterator iter=graph.k->begin();iter!=graph.k->end();){
        if(iter->indegree==0)
        {
            node temp;
            temp=*iter;
            q.push(temp);
        }
        else
            iter++;
    }
    while(!q.empty()){
        node ver=q.front();//将列中的第一个元素给ver
        q.pop();           //删除第一个元素
        cnt++;

/****************将当前值传入下一个神经元***************/

        for(vector<node>::iterator iter=graph.k->begin();iter!=graph.k->end();){
            if(iter->key==ver.to){
                iter->in=iter->in+(ver.out*iter->weight);
            }
            else
                iter++;
        }



/*********************删除入度为0的点********************/
        for(vector<node>::iterator iter=graph.k->begin();iter!=graph.k->end();){
           if(iter->from==ver.key){
               iter->indegree--;
           }
           else
               iter++;
       }

/*****************将入读为0的点压入队列*****************/
        for(vector<node>::iterator iter=graph.k->begin();iter!=graph.k->end();){
            if(iter->indegree==0)
            {
                node temp;
                temp=*iter;
                q.push(temp);
            }
            else
                iter++;
        }
        graph.k[ver.key].clear();

    }
    if(cnt<n){
        cout<<"该图有环"<<endl;
    }

}  //拓扑排序


int Adjacency_List::getvertax(){
    return this->n;
}            //返回顶点个数


int Adjacency_List::getEdge(){
    return this->m;
}             //返回边个数
