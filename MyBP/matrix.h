#ifndef _matrix_h
#define _matrix_h

#include<iomanip>
#include<iostream>
#include<time.h>
#include<vector>
using namespace std;

inline void ERR(char* s){cerr<<s; exit(0);}

class matrix;

class matrix{
    friend ostream& operator<<(ostream& os,matrix obj);
    friend matrix dot(const matrix &a,const matrix &b);
    friend matrix operator*(const double &k,const matrix &m);
private:
    int row,col;
    vector<vector<double>> a;
private:
    void free(){
        a.clear();
    }
public:
    matrix(int r=0,int c=0):row(r),col(c)
    {
        srand(time(NULL));
        a.resize(r);
        for (int i=0; i<r; i++)
            a[i].resize(c);

        for (int i=0; i<r; i++)
            for (int j=0; j<c; j++)
                a[i][j]=(rand()%10000)/2500.0-2.0;
    }
    matrix operator+(const matrix &b)//plus
    {
        if (row!=b.row||col!=b.col) ERR((char*)"PLUS ERROR\n");
        matrix ret(row,col);
        for (int i=0; i<row; i++)
            for (int j=0; j<col; j++)
                ret.a[i][j]=a[i][j]+b.a[i][j];
        return ret;
    }
    matrix operator-(const matrix &b)//minus
    {
        if (row!=b.row||col!=b.col) ERR((char*)"MINUS ERROR\n");
        matrix ret(row,col);
        for (int i=0; i<row; i++)
            for (int j=0; j<col; j++)
                ret.a[i][j]=a[i][j]-b.a[i][j];
        return ret;
    }
    matrix operator*(const matrix &b)//general matrix multiplication
    {
        if (col!=b.row) ERR((char*)"MULTIPLY ERROR mat*mat\n");
        matrix ret(row,b.col);
        for (int i=0; i<row; i++)
            for (int j=0; j<b.col; j++)
            {
                ret.a[i][j]=0;
                for (int k=0; k<col; k++) ret.a[i][j]+=a[i][k]*b.a[k][j];
            }
        return ret;
    }
    matrix operator*(const double k)//postmultiply a number
    {
        matrix ret(row,col);
        for (int i=0; i<row; i++)
            for (int j=0; j<col; j++)
                ret.a[i][j]=a[i][j]*k;
        return ret;
    }
    matrix& operator=(const matrix &b)
    {
        row=b.row,col=b.col;
        a=b.a;
        return *this;
    }
    double& operator()(int r,int c){return a[r][c];}//Accessing the elements of matrix
    matrix trans()//transpose
    {
        matrix ret(col,row);
        for (int i=0; i<row; i++)
            for (int j=0; j<col; j++)
                ret.a[j][i]=a[i][j];
        return ret;
    }
    int rows(){return row;}
    int cols(){return col;}
};

matrix operator*(const double &k,const matrix &m)//premultiply a number
{
    matrix ret(m.row,m.col);
    for (int i=0; i<m.row; i++)
        for (int j=0; j<m.col; j++)
            ret.a[i][j]=k*m.a[i][j];
    return ret;
}

ostream& operator<<(ostream& os,matrix obj)
{
    int i,j;
    for (i=0; i<obj.row; i++){
        for (j=0; j<obj.col; j++) os<<setiosflags(ios::fixed)<<setprecision(3)<<obj.a[i][j]<<' ';
        os<<endl;
    }
    return os;
}

matrix dot(const matrix &a,const matrix &b)
{
    if (a.row!=b.row||a.col!=b.col) ERR((char*)"Dot ERROR\n");
    matrix ret(a.row,a.col);
    for (int i=0; i<a.row; i++)
        for (int j=0; j<a.col; j++)
            ret.a[i][j]=a.a[i][j]*b.a[i][j];
    return ret;
}
#endif // _matrix_h
