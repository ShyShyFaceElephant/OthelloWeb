#include<iostream>
#include<stdio.h>
#include<algorithm>
#include<vector>
#include<utility>
#include<tuple>
#include<set>
#include <cmath>
#include<fstream>
#include "json.hpp"
using json = nlohmann::json;
using namespace std;

class State{
    /*****************************

    變數定義
    table:棋盤 (-1空, 0白, 1黑)
    color:下棋方(0白,1黑)
    legalStepList:下棋方合法步陣列
    numberOfPieces[2]:棋子數(0白,1黑)

    ******************************/
    friend class MCTS;
    private:
        int table[8][8];
        vector<int> legalStepList;
        int numberOfPieces[2];
        int color;
        bool gameOver;
    /*****************************

    函數定義
    move(state, position):下子並更新棋局(state)
    count(table): 計算白子及黑子個數
    updateLegalStepList(table, color): 更新下棋方合法步陣列
    findLegalStep(table,position,direction,color): 以position座標與direction出發尋找合法步 -1:沒找到
    flip(table,position,direction,color): 以position座標與direction出發按規則將棋子翻轉成color

    ******************************/
    public:
        State();
        void move(int position);
        void updateNumberOfPieces();
        void updateLegalStepList();
        int findLegalStep(int position,int direction);
        bool flip(int position,int direction);
        bool isLegal(int position);
        bool isGameOver();
        void print();
        State(json inputData);//用(棋局 + 新步) json建構
        json toJson();//轉為棋局json檔
};
class Node{
    public:
        State state;
        int wins,loses,simulations,newPosition;
        Node *parent;
        vector<Node*> children;
        Node(State state);
        void print();
};
class MCTS{
    Node *root;
    /*****************************

    函數定義
    MCTS(state):初始化，放上root並擴展一次
    predict(&root):用蒙特卡洛樹搜索尋找落子位置
    selection(&root):用UCB尋找目標節點並回傳
    expansion(&node):擴展節點
    rollout(&node):仿真，隨機走到底，回傳-1平手、0白贏、1黑贏
    backpropagation(&node):向父節點更新仿真次數、勝利次數

    ******************************/
    public:
        MCTS(State state);
        ~MCTS();
        void deleteNode(Node* node);
        double UCB(Node*);
        int predict();
        Node* selection(Node*);
        void expansion(Node*);
        int rollout(State);
        void backpropagation(Node *node,int addWins,int addLoses,int addSimulations);
};
/*************State類別定義**************/
State::State() :  color(1), gameOver(false) {
    for(int i=0;i<64;i++)table[0][i]=-1;
    table[3][3]=table[4][4]=1;
    table[4][3]=table[3][4]=0;
    numberOfPieces[0]=numberOfPieces[1]=0;
    updateLegalStepList();
    updateNumberOfPieces();
}
void State::move(int position){
    //position==-1 蒙特卡洛樹搜索找落子位置
    if(position==-1){
        State State=*this;
        MCTS model(State);
        position=model.predict();
        cout<<position<<'\n';
    }
    legalStepList = vector<int>(0);
    int r=position/8,c=position%8;
    table[r][c]=color;
    for(int direction=0;direction<8;direction++)
        flip(position,direction);
    color^=1;
    updateLegalStepList();
    updateNumberOfPieces();
    //沒路走 跳過回合
    if(legalStepList.size()==0){
        color^=1;
        updateLegalStepList();
        //雙方沒路走 遊戲結束
        if(legalStepList.size()==0)
            gameOver=true;
    }
    return;
}
void State::updateNumberOfPieces(void){
    int tmp[2]={};
    for(int i=0;i<8;i++)
        for(int j=0;j<8;j++)
            if(table[i][j]!=-1)
                tmp[table[i][j]]++;
    numberOfPieces[0]=tmp[0];
    numberOfPieces[1]=tmp[1];
}
int dr[8]={-1,-1,-1,0,0,1,1,1},dc[8]={-1,0,1,-1,1,-1,0,1};
void State::updateLegalStepList(){
    set<int> s;
    for(int i=0;i<8;i++)
        for(int j=0;j<8;j++)
            if(table[i][j]==color){
                for(int direction=0;direction<8;direction++){
                    int i1=i+dr[direction],j1=j+dc[direction];
                    if(i1>=0 && i1<8 && j1>=0 && j1<8 && table[i1][j1]==(color^1)){
                        int x=findLegalStep(i1*8+j1,direction);
                        if(x!=-1) //有找到
                            s.insert(x);
                    }

                }
            }

    for(int e:s)
        legalStepList.push_back(e);
    return;
}
int State::findLegalStep(int position,int direction){
    int r=position/8,c=position%8;
    int r1=r+dr[direction],c1=c+dc[direction];
    if(r1<0 || r1>=8 || c1<0 || c1>=8 || table[r1][c1]==color)
        return -1;
    if(table[r1][c1]==(color^1))
        return findLegalStep(r1*8+c1,direction);
    return r1*8+c1;
}
bool State::flip(int position,int direction){
    bool flag;
    int r=position/8,c=position%8;
    int r1=r+dr[direction],c1=c+dc[direction];
    if(r1<0 || r1>=8 || c1<0 || c1>=8 || table[r1][c1]==-1)//下一個為界外或為空 全部不翻
        flag= false;
    else if(table[r1][c1]==(color^1)) //下一個為敵方 繼續走
        flag= flip(r1*8+c1,direction);
    else if(table[r1][c1]==color)//下一個為我方 全部都翻
        flag=true;
    if(flag)
        table[r][c]=color;
    return flag;
}
bool State::isLegal(int position){
    return find(legalStepList.begin(),legalStepList.end(),position)!=legalStepList.end();
}
bool State::isGameOver(){
    return gameOver;
}
void State::print(){
    for(int i=0;i<8;i++){
        for(int j=0;j<8;j++){
            if(table[i][j]==-1)
                printf("%2d ",i*8+j);
            else if(table[i][j])
                cout<<" B ";
            else
                cout<<" W ";
        }
        cout<<'\n';
    }
    cout<<"legalStepList: ";
    for(int e:legalStepList)
        cout<<e<<' ';
    cout<<'\n';
    cout<<"numberOfPieces: "<<numberOfPieces[0]<<' '<<numberOfPieces[1]<<'\n';
    cout<<"color:"<<color<<'\n';
    cout<<"gameOver:"<<gameOver<<'\n';
}
State::State(json inputData){
    for(int i=0;i<64;i++){
        table[0][i]=inputData["gameState"]["table"][i/8][i%8];
    }
    cout<<1<<'\n';
    numberOfPieces[0]=inputData["gameState"]["numberOfPieces"][0];
    numberOfPieces[1]=inputData["gameState"]["numberOfPieces"][1];
    cout<<2<<'\n';
    for(int i=0;i<inputData["gameState"]["legalStepList"].size();i++){
        legalStepList.push_back(inputData["gameState"]["legalStepList"][i]);
    }
    cout<<3<<'\n';
    color=inputData["gameState"]["color"];
    gameOver=inputData["gameState"]["gameOver"];
    cout<<4<<'\n';
    move(inputData["newPosition"]);
}
json State::toJson(){
    json jsonObject;
    jsonObject["color"] = color; // 1 表示颜色值
    jsonObject["gameOver"] = gameOver;
    jsonObject["numberOfPieces"] = {numberOfPieces[0], numberOfPieces[1]};
    jsonObject["legalStepList"] =json::array();
    for(int e:legalStepList)
        jsonObject["legalStepList"].push_back(e);
    jsonObject["table"]=json::array();
    for (int i = 0; i < 8; i++) {
        json row;
        for (int j = 0; j < 8; j++) {
            // 在此添加特定的颜色值
            row.push_back(table[i][j]); // 1 表示颜色值
        }
        jsonObject["table"].push_back(row);
    }
    return jsonObject;
}
/*************Node類別定義**************/
Node::Node(State x):wins(0),loses(0),simulations(0),newPosition(-1),state(x),parent(nullptr),children(vector<Node*>(0)){}
void Node::print(){
    cout<<"========================\n";
    cout<<"win rate:"<<(1.*wins/simulations*100)<<'%'<<'\n';
    cout<<"wins:"<<wins<<'\n';
    cout<<"loses:"<<loses<<'\n';
    cout<<"simulations:"<<simulations<<'\n';
    cout<<"newPosition:"<<newPosition<<'\n';
    state.print();
    cout<<"========================\n";
}
/*************MCTS類別定義**************/
MCTS::MCTS(State state){
    root=new Node(state);
    expansion(root);
    for(int k=0;k<10000;k++){
        Node *node = selection(root);
        int addWins=0,addLoses=0,addSimulations=0;
        if(node->simulations==0){
            int x=rollout(node->state);
            if(x==node->state.color)
                addWins++;
            else if(x!=-1)
                addLoses++;
            addSimulations++;
        }
        else {
            expansion(node);
            addSimulations++;
        }

        backpropagation(node,addWins,addLoses,addSimulations);
    }
}
MCTS::~MCTS() {
    // 在析构函数中释放节点
    deleteNode(root);
}
void MCTS::deleteNode(Node* node) {
    for (Node* child : node->children) {
        deleteNode(child);
        delete child;
    }
}
double MCTS::UCB(Node* node){
    if(node->simulations==0)
        return 1000000000;
    double w=node->wins,n=node->simulations,N=node->parent->simulations;
    if(node->parent->state.color != node->state.color)
        w=node->loses;
    return w/n+sqrt(2*log(N)/n);
}
int MCTS::predict(){
    int ret=-1;
    double mx=-1;
    root->print();
    for(Node* child:root->children){
        cout<<"UCB="<<UCB(child)<<'\n';
        child->print();
        double winRate;
        if(root->state.color==child->state.color)
            winRate=1.*child->wins/child->simulations;
        else
            winRate=1.*(child->loses)/child->simulations;
        if(winRate>=mx){
            mx=winRate;
            ret=child->newPosition;
        }

    }
    return ret;
}
Node* MCTS::selection(Node* node){
    if(node->children.size()==0)
        return node;
    Node *next;
    double mx=0;
    for(Node* child:node->children){
        double ucb=UCB(child);
        if(ucb>mx){
            mx=ucb;
            next=child;
        }

    }
    return selection(next);
}
void MCTS::expansion(Node* node){
    vector<int> v=node->state.legalStepList;
    for(int pos:v){
        Node *newNode;
        newNode=new Node(node->state);
        newNode->parent=node;
        newNode->state.move(pos);
        newNode->newPosition=pos;
        node->children.push_back(newNode);
    }
}
int MCTS::rollout(State state){
    if(state.isGameOver()){
        int delta=state.numberOfPieces[1]-state.numberOfPieces[0];
        //cout<<"rollout result:"<<delta<<'\n';
        if(delta>0)
            return 1;
        if(delta<0)
            return 0;
        return -1;
    }
    int random=rand()%state.legalStepList.size();
    int pos=state.legalStepList[random];
    State newState=state;
    newState.move(pos);
    return rollout(newState);
}
void MCTS::backpropagation(Node *node,int addWins,int addLoses,int addSimulations){
    node->wins+=addWins;
    node->loses+=addLoses;
    node->simulations+=addSimulations;
    if(node->parent==nullptr)
        return;
    if(node->parent->state.color==node->state.color)
        backpropagation(node->parent,addWins,addLoses,addSimulations);
    else
        backpropagation(node->parent,addLoses,addWins,addSimulations);
}
int main() {
    ifstream ifs("input.json");
    if(!ifs.is_open()){
        cout<<"input.json開啟失敗\n";
        return 1;
    }

    cout<<"input.json開啟成功\n";
    json inputData = json::parse(ifs);
    State state(inputData);
    ifs.close();
    cout<<"input.json 關閉\n";

    json outputData = state.toJson();
    cout<<outputData.dump(4);
    // Write JSON data to a file
    ofstream ofs("output.json");
    if (ofs.is_open()) {
        ofs << outputData.dump(4); // Write JSON to the file
        ofs.close();
        cout << "JSON data has been written to 'output.json'" << endl;
    } else {
        cerr << "Unable to open the file for writing." << endl;
        return 1;
    }

    return 0;
}
