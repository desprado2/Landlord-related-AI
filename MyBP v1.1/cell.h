#ifndef _cell_h
#define _cell_h

#include<bits/stdc++.h>
using namespace std;

inline double F(double x){return 1.0/(1+exp(-x));}//sigmoid(x)
inline double diff_F(double Fx){return Fx*(1.0-Fx);}

struct edge{
    int to,next;
    double weight;
};

struct cell{
    double in,out;
    double bias;
};

class graph;

class graph{
private:
    vector<cell> g;//g:图的点集

    vector<edge> edges,cl_edges;
    vector<int> last;
    vector<double> cl_bias;

    int v,e;//v:图的点数 e:图的边数
    int input_dimension,output_dimension;//输入维度和输出维度
    vector<int> v_input,v_output;//输入点的编号和输出点的编号

public:
    graph(int size_of_graph,int in_d,int out_d,int* v_in,int* v_out):
    v(size_of_graph),e(0),input_dimension(in_d),output_dimension(out_d)
    {
        for (int i=0; i<=in_d; i++) v_input.push_back(v_in[i]);
        for (int i=0; i<=out_d; i++) v_output.push_back(v_out[i]);

        g.resize(v+1),cl_bias.resize(v+1,0),last.resize(v+1,-1);
        edges.clear(),cl_edges.clear();
    }

    void set_bias(int v,const double &init_b){g[v].bias=init_b;}

    void add_edge(int fr,int to,double w)
    {
        edges.push_back({to,last[fr],w}); last[fr]=e , e++;
        edges.push_back({fr,last[to],w}); last[to]=e , e++;

        cl_edges.push_back({to,last[fr],0}) , cl_edges.push_back({fr,last[to],0});
    }

    void update(double eps)
    {
        for (int i=1; i<=v; i++) g[i].bias-=eps*cl_bias[i] , cl_bias[i]=0;

        for (int i=0; i<e; i++) edges[i].weight-=eps*cl_edges[i].weight , cl_edges[i].weight=0;
    }

    void calc(double* input_data);
    void get_output(double* out);
    void BP(double* err);
};

void graph::calc(double* input_data)
{
    int i,in[v+1]={0};
    queue<int> qu;

    for (i=1; i<=v; i++) g[i].in=0;//初始化神经元的输入
    for (i=1; i<=input_dimension; i++)
        qu.push(v_input[i]) , g[v_input[i]].out=input_data[i];//初始化队列和输入信息

    for (i=0; i<e; i+=2) in[edges[i].to]++;//初始化入度

    while (!qu.empty())//拓扑排序
    {
        int x=qu.front(),y;
        qu.pop();
        for (i=last[x]; i!=-1; i=edges[i].next)
        {
            if (i&1) continue;//如果是反向边，那么忽略
            y=edges[i].to;

            in[y]-- , g[y].in+=g[x].out*edges[i].weight;

            if (in[y]==0)
            {
                qu.push(y);
                g[y].out=F(g[y].in);
            }
        }
    }
}

void graph::get_output(double* out)
{
    for (int i=1; i<=output_dimension; i++)
        out[i]=g[v_output[i]].out;
}

void graph::BP(double* err)
{
    int i,in[v+1]={0};
    double diff_out[v+1]={0},diff_in[v+1]={0};
    queue<int> qu;

    for (i=1; i<=output_dimension; i++)
        qu.push(v_output[i]) , diff_out[v_output[i]]=err[i];

    for (i=1; i<e; i+=2) in[edges[i].to]++;

    while (!qu.empty())
    {
        int x=qu.front(),y;
        qu.pop();

        diff_in[x]=diff_F(g[x].out)*diff_out[x];
        cl_bias[x]=diff_in[x];

        for (i=last[x]; i!=-1; i=edges[i].next)
        {
            if ((i&1)==0) continue;
            y=edges[i].to;

            in[y]-- , diff_out[y]+=diff_in[x]*edges[i].weight;
            cl_edges[i].weight+=diff_in[x]*g[y].out , cl_edges[i^1].weight+=diff_in[x]*g[y].out;

            if (in[y]==0) qu.push(y);
        }
    }
}

#endif // _cell_h
