#include "adjacency_list.h"
using namespace std;

Adjacency_List::Adjacency_List()
{
    k.clear();
    tr.clear();
    this->n=0;
    this->m=0;
}

Adjacency_List::Adjacency_List(int n,int m){
    this->n=n;
    this->m=m;
    k.clear();
    tr.clear();
}

void Adjacency_List::addEdge(transport t){
    node temp;
    int count=0;
    t.setkey(m);
    tr.push_back(t);
    for(int i=0;i<(int)k.size();i++)
    {

        if(k[i].key==t.getto())
        {
            k[i].indegree++;

        }

        count++;

    }


    m++;//边数加1

}  //增加边

void Adjacency_List::delEdge(transport t){
/****************删除链接的结点*********************/
    for(int i=0;i<(int)k.size();i++){
        /*将其所对应的下一个节点入度减1*/

        if(k[i].key==t.getto()){
            k[i].indegree--;
            }

    }


/**************************删除存入VECTOR中的边************************/
    for(int i=0;i<(int)tr.size();i++){
        if(tr[i].getkey() == t.getkey()){
            tr.erase(tr.begin()+i);
        }
      }

    m--;
}//删除边


void Adjacency_List:: addvertax(int key){
    node temp;
    temp.key=key;
    k.push_back(temp);
    n++;//顶点数加1

}    //增加顶点

void Adjacency_List::delvertax(int key){

    for(int i=0;i<(int)k.size();i++){
            if(k[i].key==key){
                for(int t=0;t<(int)tr.size();t++){
                    if(tr[t].getfrom()==key)
                    {
                        delEdge(tr[t]);
                    }
                    else if(tr[t].getto()==key){
                        tr.erase(tr.begin()+t);
                    }
                }
                k.erase(k.begin()+i);
            }
    }   //删除k中的节点
    n--;


}    //删除顶点


void Adjacency_List::TopSort(){
    int cnt=0; //判断是否有环
    vector<node>k2(k);
    vector<transport>tr2(tr);
    queue<node> q;
    /*********找入度为0的点并将他们压入队列*********/
    for(int i=0;i<(int)k2.size();i++){

            if(k2[i].indegree==0)
            {
                node temp;
                temp=k2[i];
                q.push(temp);
                qDebug()<<"qishi:"<<k2[i].key<<i;
            }

    }
    for(int i=0;i<(int)k2.size();i++)
    {
        qDebug()<<"k2:"<<k2[i].key<<i;
    }
    for(int i=0;i<(int)k.size();i++)
    {
        qDebug()<<" k:"<<k[i].key<<i;
    }

    while(!q.empty()){
        node ver=q.front();//将列中的第一个元素给ver
        q.pop();           //删除第一个元素
        cnt++;

/****************将当前值传入下一个神经元***************/

        for(int m=0;m<(int)tr2.size();m++){

             if(ver.key==tr2[m].getfrom()){

                for(int i=0;i<(int)k2.size();i++){

                        if(tr2[m].getto()==k2[i].key && ver.key!=k2[i].key)

                        {
                            qDebug()<<"look"<<k2[i].in<<ver.out<<tr2[m].getWeight()<<ver.key<<tr2[m].getto()<<k2[i].key;
                            k2[i].in=k2[i].in+(ver.out*tr2[m].getWeight());
                            qDebug()<<"look2"<<k2[i].in;
                            tr2[m].setWeight(0);
                        }

                            /**************************隐层输入*********************/
                          for(int h=0;h<(int)hidden.size();h++)
                          {
                              if(k2[i].key==hidden[h].getkey())
                              {
                                  hidden[h].setin(k2[i].in);
                                  hidden[h].setout(hidden[h].Activation_Function(hidden[h].getjiflag()));
                                  k2[i].out=hidden[h].Output();
                                  qDebug()<<"yincengshuru:"<<hidden[h].Output()<<k2[i].key<<k2[i].out;

                              }
                          }
                          for(int o=0;o<(int)Outne.size();o++){
                              if(k2[i].key==Outne[o].getkey())
                              {
                                  qDebug()<<"zaishuchu"<<k[i].key<<" "<<Outne[o].getkey()<<k2[i].in;
                                  Outne[o].setin(k2[i].in);
                                  Outne[o].setout();
                                  qDebug()<<"shuchujieguo"<<Outne[o].output();

                              }
                          }


                          }

                }

        }






     /***********************************删除入读为0的点*********************************/
        for(int i=0;i<(int)tr.size();i++){
            if(tr[i].getfrom()==ver.key){
                for(int j=0;j<(int)k2.size();j++){
                    if(k2[j].key==tr[i].getto())
                    {
                        k2[j].indegree--;
                    }
                }
            }
        }




        for(int i=0;i<(int)k2.size();i++){

                if(k2[i].key==ver.key)
                {
                    k2.erase(k2.begin()+i);
                    qDebug()<<"delete:"<<k2[i].key;
                }

        }



/*****************将入读为0的点压入队列*****************/
        for(int i=0;i<(int)k2.size();i++){

                if(k2[i].indegree==0)
                {
                    node temp;
                    temp=k2[i];
                    q.push(temp);

                }

        }

    }//while

    for(int i=0;i<(int)tr.size();i++)
    {
        qDebug()<<"Edge";
        qDebug()<<tr[i].getkey()<<"weight:"<<tr[i].getWeight()<<"from"<<tr[i].getfrom()<<"to:"<<tr[i].getto();
    }
    for(int i=0;i<(int)Inputne.size();i++)
    {
        qDebug()<<"shuru"<<Inputne[i].getInput()<<Inputne[i].getoutput();
    }

    for(int i=0;i<(int)Outne.size();i++)
    {
        qDebug()<<"shuchu";
        qDebug()<<Outne[i].input();

    }


}  //拓扑排序


int Adjacency_List::getvertax(){
    return this->n;
}            //返回顶点个数


int Adjacency_List::getEdge(){
    return this->m;
}             //返回边个数

void Adjacency_List::clrdesk(){
    tr.clear();
    k.clear();
    Inputne.clear();
    hidden.clear();
    Outne.clear();
    m=0;
    n=0;
}

void Adjacency_List::addInput(InputNeure Ineure){

    this->Inputne.push_back(Ineure);
}

void Adjacency_List::addhidden(hidden_layer Hneure){

    this->hidden.push_back(Hneure);
}

void Adjacency_List::addOutput(OutputNeure Oneure){

    this->Outne.push_back(Oneure);

}

void Adjacency_List::retInput(vector<InputNeure>::iterator iterIn){
    for(int i=0;i<(int)Inputne.size();i++)
    {
        if(iterIn->getkey()==Inputne[i].getkey())
        {
            iterIn->setInput(Inputne[i].getInput());
            iterIn->setOutput();

        }
    }

}


void Adjacency_List::retHidden(vector<hidden_layer>::iterator iterHidden){
    for(int i=0;i<(int)hidden.size();i++)
    {
        if(iterHidden->getkey()==hidden[i].getkey())
        {
            iterHidden->setin(hidden[i].Input());
            iterHidden->setjiflag(hidden[i].getjiflag());
            iterHidden->setout(iterHidden->Activation_Function(iterHidden->getjiflag()));

        }
    }

}


void Adjacency_List::retOutput(vector<OutputNeure>::iterator iterOut){
    for(int i=0;i<(int)Outne.size();i++){
        if(iterOut->getkey()==Outne[i].getkey()){
            iterOut->setin(Outne[i].input());
            iterOut->setout();

        }
    }
}

void Adjacency_List::intransport(vector<transport>::iterator ittrans){
    for(int i=0;i<(int)tr.size();i++){
        if(ittrans->getkey()==tr[i].getkey()){
            tr[i].setWeight(ittrans->getWeight());
            qDebug()<<"wangtulishe"<<tr[i].getWeight();
        }
    }

}


/****************把生成的数据传入图里****************/
void Adjacency_List::inInput(vector<InputNeure>::iterator it){//it 这个是图形化界面的游标
    for(vector<InputNeure>::iterator it1=this->Inputne.begin();it1!=this->Inputne.end();)//it1是图里面的输入层游标
    {
        if(it->getkey()==it1->getkey())
        {
            it1->setInput(it->getInput());
            it1->setOutput();
            for(int i=0;i<(int)k.size();i++)//itk 是图node的游标
            {

                    if(k[i].key==it1->getkey())
                    {
                        k[i].in=it1->getInput();
                        k[i].out=k[i].in;

                    }

            }
            break;
        }
        else
            it1++;
    }


}



void Adjacency_List::inhidden(vector<hidden_layer>::iterator  it){
    for(int i=0;i<(int)hidden.size();i++)
    {
        if(it->getkey()==hidden[i].getkey())
        {
            hidden[i].setjiflag(it->getjiflag());
            hidden[i].setthr(it->getthr());

        }

    }

}


