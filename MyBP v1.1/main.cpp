#include<bits/stdc++.h>
#include "cell.h"

const double eps=0.10;
using namespace std;

int main()
{
    int i,j,n,m,T;
    int in_d,out_d;
    int V,E;

    freopen("form.txt","r",stdin);
    cin>>in_d>>out_d;
    cin>>V>>E;

    int v_in[in_d+1],v_out[out_d+1];


    for (i=1; i<=in_d; i++) cin>>v_in[i];
    for (i=1; i<=out_d; i++) cin>>v_out[i];

    graph G(V,in_d,out_d,v_in,v_out);
    double ini;
    int x,y;
    for (i=1; i<=V; i++){
        cin>>ini;
        G.set_bias(i,ini);
    }
    for (i=1; i<=E; i++){
        cin>>x>>y>>ini;
        G.add_edge(x,y,ini);
    }

    cin>>T>>n;
    while (T&&n){
        double input_data[n][in_d+1],output_data[n][out_d+1];
        for (i=1; i<=n; i++){
            for (j=1; j<=in_d; j++) cin>>input_data[i][j];
            for (j=1; j<=out_d; j++) cin>>output_data[i][j];
        }

        while (T--)
        {
            for (i=1; i<=n; i++){
                G.calc(input_data[i]);

                double tmp[out_d+1];
                G.get_output(tmp);
                for (j=1; j<=out_d; j++) tmp[j]=tmp[j]-output_data[i][j];

                G.BP(tmp);
            }
            G.update(eps);
        }
        cin>>T>>n;
    }

    freopen("out.txt","w",stdout);
    cin>>m;
    for (i=1; i<=m; i++){
        double test[in_d+1],ans[out_d+1];
        for (j=1; j<=in_d; j++) cin>>test[j];

        G.calc(test);
        G.get_output(ans);

        for (j=1; j<=out_d; j++) cout<<setiosflags(ios::fixed)<<setprecision(3)<<ans[j];
        cout<<endl;
    }
    return 0;
}
