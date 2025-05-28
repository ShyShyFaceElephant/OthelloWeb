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
    friend class MCTS; //讓MCTS可以使用State的私人變數
    /*****************************

    類別State 變數定義
    table:棋盤 (-1空, 0白, 1黑)
    legalStepList:下棋方合法步陣列
    numberOfPieces[2]:棋子數(0白,1黑)
    color:下棋方(0白,1黑)
    gameOver:遊戲是否結束

    ******************************/
    private:
        int table[8][8];
        vector<int> legalStepList;
        int numberOfPieces[2];
        int color;
        bool gameOver;
    /*****************************

    類別State 函數定義
    State():建構棋局及初始化
    move(position):下子並更新棋局(state)，若position==-1則啟動AI落子
    updateNumberOfPieces():更新黑白子的棋子數(0白,1黑)
    updateLegalStepList(): 更新下棋方合法步陣列legalStepList
    findLegalStep(position,direction): 以position座標與direction出發尋找合法步 -1:沒找到
    flip(position,direction): 以position座標與direction出發按規則將棋子翻轉成color
    isLegal(position):判斷位置是否為合法步
    isGameOver():回傳私人變數gameOver(1遊戲結束/0遊戲未結束)
    print():在終端機印出現在棋局，Debug用
    getColor():取得當前棋子顏色

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
        int getColor();
    /*****************************

    類別State 與json相關函數定義
    State(inputData):用Json(包含:棋局、新步)建構棋局，「並以新落子位置更新棋局」
    json toJson():將棋局打包成Json檔

    ******************************/
        State(json inputData);
        json toJson();

};

class Node{
    /*****************************

    類別Node 變數定義
    state:棋局
    wins:勝利次數(對於此節點玩家)
    loses:失敗次數(對於此節點玩家)
    simulations:仿真次數，走過此節點的次數
    newPosition:新落子位置(相對於父節點)
    *parent:指向父節點的指標
    children:指向子節點指標陣列

    ******************************/
    public:
        State state;
        int wins,loses,simulations,newPosition;
        Node *parent;
        vector<Node*> children;
    /*****************************

    類別Node 函數定義
    Node(state):用state建構節點
    print():在終端機印出節點，Debug用

    ******************************/
        Node(State state);
        void print();
};

class MCTS{
    /*****************************

    類別MCTS 變數定義
    *root:指向根節點的指標

    ******************************/
    Node *root;
    /*****************************

    類別MCTS 變數定義
    MCTS(state):初始化，放上root並擴展(expansion)一次
    ~MCTS():解構MCTS時，呼叫deleteNode(root)釋放所有節點記憶體空間
    deleteNode(*node):釋放所有節點記憶體空間
    predict(*node):用蒙特卡洛樹搜索尋找落子位置
    UCB(*node):回傳此節點的UCB值
    selection(*root):用UCB尋找目標葉節點並回傳
    expansion(*node):擴展節點
    rollout(*node):仿真，隨機走到底，回傳-1平手、0白贏、1黑贏
    backpropagation(*node):向父節點更新仿真次數、勝利次數

    ******************************/
    public:
        MCTS(State state);
        ~MCTS();
        void deleteNode(Node* node);
        double UCB(Node* node);
        int predict();
        Node* selection(Node* node);
        void expansion(Node* node);
        int rollout(State state);
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
        MCTS model(*this);
        position=model.predict();
        //cout<<position<<'\n';
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
    numberOfPieces[0]=inputData["gameState"]["numberOfPieces"][0];
    numberOfPieces[1]=inputData["gameState"]["numberOfPieces"][1];
    for(int i=0;i<inputData["gameState"]["legalStepList"].size();i++){
        legalStepList.push_back(inputData["gameState"]["legalStepList"][i]);
    }
    color=inputData["gameState"]["color"];
    gameOver=inputData["gameState"]["gameOver"];
    //最後更新棋局
    move(inputData["newPosition"]);
}
json State::toJson(){
    json jsonObject;
    jsonObject["color"] = color;
    jsonObject["gameOver"] = gameOver;
    jsonObject["numberOfPieces"] = {numberOfPieces[0], numberOfPieces[1]};
    jsonObject["legalStepList"] =json::array();
    for(int e:legalStepList)
        jsonObject["legalStepList"].push_back(e);
    jsonObject["table"]=json::array();
    for (int i = 0; i < 8; i++) {
        json row;
        for (int j = 0; j < 8; j++) {
            row.push_back(table[i][j]);
        }
        jsonObject["table"].push_back(row);
    }
    return jsonObject;
}
int State::getColor(){
    return color;
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
/**************多執行緒定義**************/
typedef struct threadParam{
    MCTS *MCTS_ptr;
    State *state_ptr;
    int *addWins_ptr;
    int *addLoses_ptr;
}threadParam;
void* runThread(void *ptr){
    threadParam *param_ptr = (threadParam*)ptr;
    MCTS *MCTS_ptr = param_ptr->MCTS_ptr;
    State state = *(param_ptr->state_ptr);
    int *addWins_ptr = param_ptr->addWins_ptr;
    int *addLoses_ptr = param_ptr->addLoses_ptr;
    int x=MCTS_ptr->rollout(state);
    if(x==state.getColor())
        (*addWins_ptr)++;
    else if(x!=-1)
        (*addLoses_ptr)++;
    delete param_ptr;
    return nullptr;
}
/*************MCTS類別定義**************/
MCTS::MCTS(State state){
    root=new Node(state);
    expansion(root);
    for(int k=0;k<200;k++){
        Node *node = selection(root);
        int addWins=0,addLoses=0,addSimulations=0;
        if(node->simulations==0){
            pthread_t pid[50];
            for(int i=0;i<50;i++){
                threadParam *param = new threadParam{this, &(node->state), &addWins, &addLoses};
                pthread_create(&pid[i], NULL, runThread, param);
            }
            for(int i=0;i<50;i++){
                pthread_join(pid[i],NULL);
                addSimulations++;
            }
        }
        else {
            expansion(node);
            addSimulations++;
        }
        backpropagation(node,addWins,addLoses,addSimulations);
    }
}
MCTS::~MCTS() {
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
    //root->print();
    for(Node* child:root->children){
        //cout<<"UCB="<<UCB(child)<<'\n';
        //child->print();
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
    /********** 讀取 stdin 中的 json **********/
    string inputStr((istreambuf_iterator<char>(cin)), istreambuf_iterator<char>());
    json inputData = json::parse(inputStr);

    /********** 處理棋局 **********/
    State state(inputData); // 假設此處會根據輸入更新棋局
    json outputData = state.toJson();

    /********** 輸出 json 到 stdout **********/
    cout << outputData.dump(); // 注意不要加縮排以方便前端解析

    return 0;
}
