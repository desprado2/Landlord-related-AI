#ifndef _cell_h
#define _cell_h

#include<cmath>
#include<time.h>
#include<cstdlib>
#include"matrix.h"

inline void F(double &x){x=(tanh(x)+1.0)/2.0;}
inline double diff_F(double Fx){return (1.0-Fx*Fx)/2.0;}
class cell{
public:
    matrix W;
    matrix in,b,out;
    cell(int i,int o,int cases):W(o,i),in(i,cases),b(o,cases),out(o,cases){
        srand(time(NULL));
        double ini[o];
        for (int i=0; i<o; i++) ini[i]=rand()%10000/5000.0-1.0;
        for (int i=0; i<o; i++)
            for (int j=0; j<cases; j++)
                b(i,j)=ini[i];
    }

    void calc()
    {
        matrix tmp=W*in + b;
        for (int i=0; i<tmp.rows(); i++)
            for (int j=0; j<tmp.cols(); j++)
                F(tmp(i,j));
        out=tmp;
    }
};
#endif // _cell_h
