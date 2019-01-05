#include<bits/stdc++.h>
#include"matrix.h"
#include"cell.h"
#define TIMES 10000
using namespace std;

int main()
{
    freopen("form.txt","r",stdin);

    int n,i,j;
    double eps;
    cin>>n>>eps;  //n:number of layers; eps:learning rate.
    int m[n+1];
    for (i=0; i<=n; i++)cin>>m[i];

    int t;
    cin>>t;
    matrix x(m[0],t),y(m[n],t);
    for (j=0; j<t; j++)
    {
        for (i=0; i<m[0]; i++) cin>>x(i,j);//x:input matrix
        for (i=0; i<m[n]; i++) cin>>y(i,j);//y:output matrix
    }

    vector<cell> arr;
    for (i=0; i<=n; i++)
    {
        if (i) {cell tmp(m[i-1],m[i],t); arr.push_back(tmp);}
        else {cell tmp(0,m[i],t); arr.push_back(tmp);}
    }
    for (int T=TIMES; T; T--)
    {
        arr[0].out=x;
        for (i=1; i<=n; i++)
            arr[i].in=arr[i-1].out,arr[i].calc();//forward calculation
	//update the layers backward
        matrix g(m[n],t);//g:gradient matrix
        g=arr[n].out-y;//let error function J be L2-norm.
        for (int j=n; j; j--){
            matrix d_tmp(m[j],t);
            for (int i=0; i<m[j]; i++)
                for (int k=0; k<t; k++)
                    d_tmp(i,k)=diff_F(arr[j].out(i,k));
            g=dot(g,d_tmp);

            double ave[m[j]];//ave[i]=the sum of i-th row in gradient matrix
            for (i=0; i<m[j]; i++)
            {
                ave[i]=0;
                for (int k=0; k<t; k++)
                    ave[i]+=g(i,k);
                for (int k=0; k<t; k++)
                    arr[j].b(i,k)-=ave[i]*eps;
            }

            matrix tmpW/*(arr[j].W.cols(),arr[j].W.rows())*/; tmpW=arr[j].W.trans();
            arr[j].W=arr[j].W - g * arr[j-1].out.trans() * eps;
            g=tmpW * g;
        }
    }

    freopen("out.txt","w",stdout);
    cin>>t;
    for (int k=0; k<t; k++)
    {
        matrix test(m[0],1);
        for (i=0; i<m[0]; i++) cin>>test(i,0);
        for (j=1; j<=n; j++)
        {
            test=arr[j].W * test;
            for (i=0; i<m[0]; i++)
                test(i,0)+=arr[j].b(i,0);
            for (i=0; i<m[0]; i++)
                F(test(i,0));
        }
        cout<<test;
    }
    for (i=1; i<=n; i++)
    {
        cout<<"____________________\nHidden layer "<<i<<":\nWeight Matrix:\n"<<arr[i].W;
        cout<<"Bias vector:\n";
        for (j=0; j<m[i]; j++) cout<<arr[i].b(j,0)<<endl;
        cout<<"Activation funcion: A(x)=(tanh(x)+1)/2\n";
    }
    //cout<<arr[n].out;
    return 0;
}
