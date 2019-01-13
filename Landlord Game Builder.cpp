#include<bits/stdc++.h>
#define MAX_point 16  //最大点数 
#define GAME_OVER 66666.9999
#define TRY_TO_WIN 6666.9999
#define INF 1073741823
using namespace std;

int attach(int a){if (a==-1) return -1; return (a<=16)?1:2;}//判断带牌的类型 1:单; 2:对 
struct action
{
	string cat;//catagory
	int p1,p2,p3;//parameters
	//参数说明:p1:表达这手牌的大小  p2:表达这手牌的牌型(例如三带到底是带单还是对还是不带,以及顺子的长度)  p3:对飞机而言，记录牌型(p2用于记录长度了) 
	bool operator==(const action &a){return cat==a.cat&&p1==a.p1&&p2==a.p2&&p3==a.p3;}
};

struct hand
{
	double pts=-INF;//牌力的评分 
	int single[MAX_point]={0},pair[MAX_point]={0},tri[MAX_point]={0},bomb[MAX_point][2]={{0},{0}}\
	    ,plane[MAX_point]={0},straight[MAX_point]={0},strpair[MAX_point]={0};
	//tri/plane:带牌。-1:不带; <=16:带单; >16:带对。 bomb:-1:不带，1~16:带单,16~29:带对
	int rec[MAX_point]={0};//rec:记牌器，记录外面还有什么牌;
	int turn;//0:lord; 0->1->2->0
	int policy;//policy=1:正常进攻状态; 2:杀死比赛状态; -1:被动状态; -2:不出牌状态(仅限斗地主报单时出现) 
	action minimal;//如果打算杀死比赛，必须忽略一个最小牌 
	int remain[MAX_point]={0};
	bool maximize_single=0;//是否进入"报单紧急状态" 
} player[3];

void init(hand &h,const int turn,const bool clear_all=true)//initialize the hand
{
	memset(h.single,0,4*MAX_point),memset(h.pair,0,4*MAX_point),memset(h.tri,0,4*MAX_point),memset(h.straight,0,4*MAX_point),\
	memset(h.strpair,0,4*MAX_point),memset(h.plane,0,4*MAX_point),memset(h.bomb,0,8*MAX_point),memset(h.remain,0,4*MAX_point);
	if (clear_all)//clear_all:表示是否要初始化记牌器的信息 
	{
		for (int i=1; i<=13; i++) h.rec[i]=4; h.rec[15]=h.rec[14]=1;
		h.maximize_single=false;
	}
	h.turn=turn;
}

void d8g(const hand &a_optimal)//输出理牌结果 
{
	for (int i=1; i<=15; i++) cout<<a_optimal.remain[i]<<' ';
	cout<<"\n        S   P   T  ST  SP  PL  BB\n";
	for (int i=1; i<=15; i++)
		cout<<setw(4)<<i<<':'<<setw(4)<<a_optimal.single[i]<<setw(4)<<a_optimal.pair[i]<<setw(4)<<a_optimal.tri[i]<<setw(4)<<a_optimal.straight[i]<<setw(4)<<a_optimal.strpair[i]<<setw(4)<<a_optimal.plane[i]<<setw(4)<<a_optimal.bomb[i][0]<<endl;
}

void prcard(const int i)
{
	if (i<=8)cout<<setw(4)<<i+2;
	if (i==9)cout<<setw(4)<<"J";
	if (i==10)cout<<setw(4)<<"Q";
	if (i==11)cout<<setw(4)<<"K";
	if (i==12)cout<<setw(4)<<"A";
	if (i==13)cout<<setw(4)<<"2";
	if (i==14)cout<<setw(4)<<"J-";
	if (i==15)cout<<setw(4)<<"J+";
}

void print(const int *h)
{
	int i;
	for (i=1; i<=15; i++) prcard(i);
	cout<<endl;
	for (i=1; i<=15; i++)cout<<setw(4)<<h[i];cout<<endl;
}

int decipher(const hand &a,int *h,const bool print_needed=false)//把已经理好的手牌读取出来 
{
	int i,j,k,cnt=0;
	for (i=0; i<MAX_point; i++) h[i]=0;
	for (i=1; i<=15; i++) h[i]+=a.single[i]+2*a.pair[i],cnt+=a.single[i]+2*a.pair[i];
	for (i=1; i<=13; i++)
		if (a.tri[i]!=0)
		{
			h[i]+=3,cnt+=3;
			if (a.tri[i]>16) h[a.tri[i]-16]+=2,cnt+=2;
			else if (a.tri[i]>0) h[a.tri[i]]++,cnt++;
		}
	for (i=1; i<=13; i++)
		if (a.plane[i]!=0)
		{
			h[i]+=3,cnt+=3;
			if (a.plane[i]>16) h[a.plane[i]-16]+=2,cnt+=2;
			else if (a.plane[i]>0) h[a.plane[i]]++,cnt++;
		}
	for (i=1; i<=8; i++)//3~10
		if (a.straight[i])
		{
			for (j=i,k=1; k<=a.straight[i]; j++,k++) h[j]++,cnt++;
		}
	for (i=1; i<=10; i++)//3~Q
		if (a.strpair[i])
		{
			for (j=i,k=1; k<=a.strpair[i]; j++,k++) h[j]+=2,cnt+=2;
		}
	for (i=1; i<=13; i++)
		if (a.bomb[i][0]!=0)
		{
			h[i]+=4,cnt+=4;
			if (a.bomb[i][0]>16) h[a.bomb[i][0]-16]+=2,h[a.bomb[i][1]-16]+=2,cnt+=4;
			else if (a.bomb[i][0]>0) h[a.bomb[i][0]]++,h[a.bomb[i][1]]++,cnt+=2;
		}
	if (a.bomb[14][0]==-1) h[14]++,h[15]++,cnt+=2;
	if (print_needed) print(h);
	return cnt;
}

double c_single(const hand &a)
{
	int i,s=0,m=0,l=0;
	double ret=0;
	for (i=1; i<=8; i++) s+=a.single[i];//3~10:small 
	for (i=9; i<=11; i++) m+=a.single[i];//J~K:middle
	l=a.single[13];//2:large
	ret+=l-max(s,m)+1.2*a.single[14]+1.5*a.single[15];
	if (a.single[12]>1) ret-=0.5*(a.single[12]-1);//单张A不大不小，不计分。但是如果有多张，那么影响不好。 
	if (a.turn>0&&s>m&&s>=3&&m>0) ret-=1;
	if (a.turn==2) ret+=0.5;//同样的单牌，如果处于地主上家，那么单牌状况实际上略优于另外两人 
	if (ret<=-2) ret*=1.5;//如果单牌状况太差，那么会造成非常恶劣的影响，因此要"放大"这个影响 
	if (ret>=2) ret-=1;//如果单牌太好，那么没有必要，所以不鼓励这种情况 
	return ret*0.75;
}

double c_pair(const hand &a)
{
	int i;
	double ret=0;
	for (i=1; i<=4; i++) ret-=a.pair[i];//3~6:small
	for (i=5; i<=11; i++) ret+=a.pair[i]*(i-8)*0.25*0.8;//7~K:middle
	for (i=12; i<=13; i++) ret+=a.pair[i];//1~2:large
	if (ret<=-1.5) ret-=1.5;//"放大"太差的对子情况 
	if (ret>=1.5) ret-=1;//同单牌 
	return ret;
}

double pts_tri(const int i,const int SortofAttach)//单个triple的估价 
{
	return (i<=7)?(SortofAttach==-1?0.4:0.5):(SortofAttach==-1?0.65:0.85);
}

double c_triple(const hand &a)//triple的总估价 
{
	double ret=0;
	for (int i=1; i<=13; i++) ret+=(a.tri[i]!=0)*pts_tri(i,a.tri[i]);
	return ret;
}

double c_straight(const int len){return len/5.0*1.8;}//单个straight的估价 
double c_strpair(const int len){return len/3.0*1.8;}//单个strpair的估价 
double c_plane(const int attach){return (attach==-1?0.7:0.9);}//单个plane的估价,注意plane作为稳定的过牌手段(不容易被压)，评分高于triple 
const double c_fours=1.8*6.0/5.0;//单个四带二估价 
const double c_bomb=2.5;//单个炸弹估价 

double c_all(hand &a,const bool maximize_single)//计算总牌力; maximize_single=1:在敌方报单的时候，要让第二大的单牌尽可能大。 
{
	//d8g(a);system("pause");
	int i,j,k,turns=0;
	
	//以下判断能否一次出完 
	for (i=1; i<=15; i++)
		turns+=a.single[i]+a.pair[i]+(a.tri[i]!=0)+(a.bomb[i][0]!=0)+(a.plane[i]!=0)+(a.straight[i]!=0)+(a.strpair[i]!=0);
	if (turns<=1) {a.policy=1; return GAME_OVER;}//一次出完的必须资磁。
	
	if (maximize_single)
	{
		a.policy=2,a.minimal={"N",0,0,0};
		for (i=1; i<=15; i++)
			if (a.single[i])
			{
				a.minimal={"single",i,0,0};
				break;
			}
		for (i=i+1; i<=15; i++)
			if (a.single[i]) break;
		return -TRY_TO_WIN*(16-i);
	}//特殊处理报单 
	
	//以下判部分断是否能够只留最多一手小牌即可杀死比赛。 
	int c,cc=0,tmp;//c:每个牌型的"净大牌数量"(绝对优势的大牌数量-小牌数量); cc:所有牌型的小牌数量 
	bool noBomb=1,fl=0;//nobomb:外面没有王炸，其它炸弹不考虑。 fl:记录能否杀死比赛 
	action mins;
	
	if (a.rec[14]==1&&a.rec[15]==1) noBomb=0;
	c=0;//找三带 
	for (i=13; i&&noBomb; i--) 
		if (a.tri[i])
		{
			int bigger=0;
			for (j=i+1; j<=13; j++)
				if (a.rec[j]>=3) bigger++;
			if (bigger>=2) c--,tmp=i;
			else c++;
		}
	if (c<0) {cc-=c; mins.cat="tri",mins.p1=tmp,mins.p2=attach(a.tri[tmp]),mins.p3=0;}
	
	for (i=8; i&&noBomb; i--)//找顺子 
		if (a.straight[i]&&!(a.straight[i]>=7||a.straight[i]+i>=12)) {cc++; mins.cat="straight",mins.p1=i,mins.p2=a.straight[i],mins.p3=0;}
	
	c=0;//找对
	for (i=13; i&&noBomb; i--) 
		if (a.pair[i])
		{
			bool bigger=0;
			for (j=i+1; j<=13; j++)
				if (a.rec[j]>=2) {bigger=1; break;}
			if (bigger) c-=a.pair[i],tmp=i;//如果有能压死i的牌，则算小牌 
			else if (!a.rec[i]>=2) c+=a.pair[i];//如果没有比i大或者相等的牌，则记最大牌 
		}
	if (c<0) {cc-=c; mins.cat="pair",mins.p1=tmp,mins.p2=mins.p3=0;}
	
	c=0;//找单
	for (i=15; i&&noBomb; i--) 
		if (a.single[i])
		{
			bool bigger=0;
			for (j=i+1; j<=15; j++)
				if (a.rec[j]) {bigger=1; break;}
			if (bigger) c-=a.single[i],tmp=i;//如果有能压死i的牌，则算小牌 
			else if (!a.rec[i]) c+=a.single[i];//如果没有比i大或者相等的牌，则记最大牌 
		}
	if (c<0) {cc-=c; mins.cat="single",mins.p1=tmp,mins.p2=mins.p3=0;}
	
	for (i=1; i<=14; i++)//找炸弹，一个炸弹可以解决一个小牌问题 
		if (a.bomb[i][0]==-1) cc--;
	
	if (noBomb&&cc<=1) {a.policy=2,a.minimal=mins; fl=1;}//能终结比赛的必须资磁。
	
	//正常计算
	
	double ret=c_single(a)+c_pair(a)+c_triple(a)+(fl?TRY_TO_WIN:0);
	for (i=1; i<=14; i++)
	{
		if (a.bomb[i][0]==-1) ret+=c_bomb;
		else if (a.bomb[i][0]!=0) ret+=c_fours;
		ret+=c_straight(a.straight[i]);
		ret+=c_strpair(a.strpair[i]);
		ret+=(a.plane[i]!=0)*c_plane(a.plane[i]);
	}
	if (!fl&&a.policy==2) a.policy=1;
	return ret;
}

hand a_optimal;//由于attribute()没有写好接口, 这个全局变量用来存储attribute后最优牌型组合 
bool in_interval(int a,int lh,int rh){return a>=lh&&a<=rh;}//判断a是否在区间[lh,rh]内 

void attribute(hand cur,int step,const bool maximize_single=false)//用深搜的方法找出最优牌型分配方案 
//case step:1:straight; 2:strpair; 3:bomb; 4:plane; 5:triple; 6:single and pair 
{
//	d8g(cur),system("pause");
	if (step==7)
	{
		double score=c_all(cur,maximize_single);
		if (score>a_optimal.pts) a_optimal=cur,a_optimal.pts=score;
		return;
	}
	switch (step){
		case 1://straight
		{
			attribute(cur,step+1,maximize_single);
			int i,j,k,c;
			hand temp;
			for (i=1; i<=8; i++)
			{
				if (!cur.remain[i]) continue;
				temp=cur;
				bool fl=0;
				for (j=i,c=1; cur.remain[j]&&j<13; j++,c++)
				{
					temp.remain[j]--;
					temp.straight[i]=c;
					if (c>=5){attribute(temp,step,maximize_single);fl=1;}
				}
				if (fl&&cur.remain[i]==1) break;//如果这个顺子的开头是单牌，那不需要考虑用下一个牌开头的情况了(例如有345678肯定不会打45678) 
			}
			break;
		}
		case 2:
		{
		//	d8g(cur);system("pause");
			attribute(cur,step+1,maximize_single);
			int i,j,k,c;
			hand temp;
			for (i=1; i<=10; i++)
			{
				if (cur.remain[i]<2) continue;
				temp=cur;
				for (j=i,c=1; cur.remain[j]>=2&&j<13; j++,c++)
				{
					temp.remain[j]-=2;
					temp.strpair[i]=c;
					if (c>=3)
						attribute(temp,step,maximize_single);
				}
			}
			break;
		}
		case 3://bomb/fours
		{
			attribute(cur,step+1,maximize_single);
			int i,j,k;
			int s[3],p[3],c;//s[0],s[1]:带出的单牌,s[2]:有没有足够单牌给我带,当s[2]==2时表示至少有两张单牌，则允许四带二 
			c=0;
			for (i=1; i<=12&&c<2; i++)
				if (cur.remain[i]==1) s[c]=i,c++;
			s[2]=c;
			c=0;
			for (i=1; i<=12&&c<2; i++)//p:带出的对子,意义同上 
				if (cur.remain[i]==2) p[c]=i,c++;
			p[2]=c;
			
			hand temp;
			for (i=1; i<=13; i++)
				if (cur.remain[i]==4)
				{
					temp=cur,temp.remain[i]-=4;
					temp.bomb[i][0]=-1;
					attribute(temp,step,maximize_single);//炸弹 
					if (s[2]==2)
					{
						temp=cur,temp.remain[i]-=4;
						temp.remain[s[0]]--,temp.remain[s[1]]--,temp.bomb[i][0]=s[0],temp.bomb[i][1]=s[1];
						attribute(temp,step,maximize_single);//四带二，单牌 
					}
					if (p[2]==2)
					{
						temp=cur,temp.remain[i]-=4;
						temp.remain[p[0]]-=2,temp.remain[p[1]]-=2,temp.bomb[i][0]=p[0]+16,temp.bomb[i][1]=p[1]+16;
						attribute(temp,step,maximize_single);//四带二，对子 
					}
				}
			if (cur.remain[14]&&cur.remain[15])//王炸 
			{
				temp=cur;
				temp.remain[14]--,temp.remain[15]--,temp.bomb[14][0]=-1;
				attribute(temp,step+1,maximize_single);
			}
			break;
		}
		case 4://plane 
		{
			attribute(cur,step+1,maximize_single);
			int i,j,k,lh;
			hand temp;
			for (i=1; i<=11; i++)
			{
				if (cur.remain[i]>=3)
				{
					int c=0;
					for (j=i; cur.remain[j]>=3&&j<13; j++) c++;
					if (c>=2)
					{
						temp=cur;
						for (lh=i; lh<j; lh++)
							temp.remain[lh]-=3;
						
						for (lh=i; lh<j; lh++)
							temp.plane[lh]=-1;
						attribute(temp,step,maximize_single);
						
						int s[6],p[6],cs=0,cp=0;//s:单牌，p:对子，cs:能找到的单牌数量,cp:能找到的对子数量。参考四带二的操作。 
						
						for (k=1; k<=13&&cs<c; k++)
							if (temp.remain[k]==1&&!in_interval(k,i,j-1))
								s[++cs]=k;
						for (k=1; k<=13&&cs<c; k++)
							if (temp.remain[k]==2&&!in_interval(k,i,j-1))
								s[++cs]=k;
						for (k=14; k<=15&&cs<c; k++)
							if (temp.remain[k]==1)
								s[++cs]=k;
						if (cs==c)
						{
							for (k=1,lh=i; k<=c; k++,lh++)
								temp.remain[s[k]]--,temp.plane[lh]=s[k];
							attribute(temp,step,maximize_single);
							for (k=1,lh=i; k<=c; k++,lh++)
								temp.remain[s[k]]++,temp.plane[lh]=0;
						}
						
						for (k=1; k<=13&&cp<c; k++)
							if (temp.remain[k]==2)
								p[++cp]=k;
						if (cp==c)
						{
							for (k=1,lh=i; k<=c; k++,lh++)
								temp.remain[p[k]]-=2,temp.plane[lh]=p[k]+16;
							attribute(temp,step,maximize_single);
						}
					}
				}
			}
			break;
		}
		case 5://三带 
		{
		//	d8g(cur),system("pause");
			int i,j;
			//以下是优先带单牌的打法
			hand temp=cur;
			for (i=1; i<13; i++)
				if (cur.remain[i]>=3)
				{
					temp.remain[i]-=3;
					bool fl=0;
					for (j=1; j<=15; j++)
						if (temp.remain[j]==1&&j!=i) {fl=1; break;}
					if (fl) {temp.tri[i]=j,temp.remain[j]--; continue;}
					for (j=1; j<=15; j++)
						if (temp.remain[j]==2) {fl=1; break;}
					if (fl) {temp.tri[i]=j+16,temp.remain[j]-=2; continue;}
					temp.tri[i]=-1;
				}
			attribute(temp,step+1,maximize_single);//不考虑三张2 
			//思路：比2小的所有三张必须以三带形式出(注意此前已经搜完其它牌型，因此在这里还有三张的话，不是三带就是单/对)。 
			if (cur.remain[13]>=3)//考虑三张2 
			{
				temp.remain[13]-=3;
				bool fl=0;
				for (j=1; j<=15; j++)
					if (temp.remain[j]==1) {fl=1; break;}
				if (fl) temp.tri[2]=j,temp.remain[j]--;
				else {
					for (j=1; j<=15; j++)
						if (temp.remain[j]==2) {fl=1; break;}
					if (fl) temp.tri[2]=j+16,temp.remain[j]-=2;
					else temp.tri[2]=-1;
				}
				attribute(temp,step+1,maximize_single);
			}
			//以下是优先带一对的打法
			temp=cur; 
			for (i=1; i<13; i++)
				if (cur.remain[i]>=3)
				{
					temp.remain[i]-=3;
					bool fl=0;
					for (j=1; j<=15; j++)
						if (temp.remain[j]==2) {fl=1; break;}
					if (fl) {temp.tri[i]=j+16,temp.remain[j]-=2; continue;}
					for (j=1; j<=15; j++)
						if (temp.remain[j]==1&&j!=i) {fl=1; break;}
					if (fl) {temp.tri[i]=j,temp.remain[j]--; continue;}
					temp.tri[i]=-1;
				}
			attribute(temp,step+1,maximize_single);
			if (cur.remain[13]==3)
			{
				temp.remain[13]-=3;
				bool fl=0;
				for (j=1; j<=15; j++)
					if (temp.remain[j]==2) {fl=1; break;}
				if (fl) temp.tri[13]=j+16,temp.remain[j]-=2;
				else {
					for (j=1; j<=15; j++)
						if (temp.remain[j]==1) {fl=1; break;}
					if (fl) temp.tri[13]=j,temp.remain[j]--;
					else temp.tri[13]=-1;
				}
				attribute(temp,step+1,maximize_single);
			}
			break;
		}
		case 6://single and pairs
		{
		//	d8g(cur),system("pause");
			int i,j; bool fl=0;
			hand temp;
			for (j=1; j<13; j++)
				if (cur.remain[j]>=3) return;//这个时候还有小于2的三张相同牌，肯定不是最优的，不考虑 
			
			for (i=1; i<13; i++)
				if (cur.remain[i]==1) cur.single[i]=1,cur.remain[i]=0,fl=1;
				else if (cur.remain[i]==2) cur.pair[i]=1,cur.remain[i]=0,fl=1;
			for (i=14; i<=15; i++)
				if (cur.remain[i]==1) cur.single[i]=1,cur.remain[i]=0,fl=1;
			temp=cur;
			switch(temp.remain[13])//枚举2 
			{
				case 0:{attribute(temp,step+1,maximize_single); break;}
				case 1:{temp.single[13]=1,temp.remain[13]=0; attribute(temp,step+1,maximize_single); break;}
				case 2:
				{
					temp.remain[13]=0;
					temp.single[13]=2; attribute(temp,step+1,maximize_single); temp.single[13]=0;
					temp.pair[13]=1; attribute(temp,step+1,maximize_single);
					break; 
				}
				case 3:
				{
					temp.remain[13]=0;
					temp.single[13]=3; attribute(temp,step+1,maximize_single); temp.single[13]=0;
					temp.pair[13]=temp.single[13]=1; attribute(temp,step+1,maximize_single);
					break;
				}
				case 4:
				{
					temp.remain[13]=0;
					temp.single[13]=4; attribute(temp,step+1,maximize_single); temp.single[13]=0;
					temp.pair[13]=1,temp.single[13]=2; attribute(temp,step+1,maximize_single); temp.pair[13]=temp.single[13]=0;
					temp.pair[13]=2; attribute(temp,step+1,maximize_single); temp.pair[13]=0;
					temp.bomb[13][0]=-1; attribute(temp,step+1,maximize_single);
					break;
				}
			}
			break;
		}
	}
}
void reattribute(hand &h,const bool maximize_single=false)//对已经理好但是打出了若干张牌的手牌重新理牌 
{
	int temp[MAX_point];
	a_optimal.pts=-INF;
	decipher(h,temp),init(h,h.turn,false),memcpy(h.remain,temp,4*MAX_point);
	attribute(h,1,maximize_single);
	h=a_optimal;//if (maximize_single){for (int i=1; i<=15; i++)cout<<temp[i]<<' ';cout<<endl;d8g(h);system("pause");}
}

bool find_single(const hand &h,action &temp,int lh,int rh,const action &minimal={"",0,0,0})//找到lh~rh中最小的单牌 
//minimal:舍去的最小牌型
{
	action t;
	for (int i=lh; i<=rh; i++)
		if (h.single[i])
		{
			t={"single",i,0,0};
			if (!(t==minimal)) {temp=t; return 1;}
		}
	return 0;
}
bool find_pair(const hand &h,action &temp,int lh,int rh,const action &minimal={"",0,0,0})
//minimal:舍去的最小牌型
{
	action t;
	for (int i=lh; i<=rh; i++)
		if (h.pair[i])
		{
			t={"pair",i,0,0};
			if (!(t==minimal)) {temp=t; return 1;}
		}
	return 0;
}
bool find_straight(const hand &h,action &temp,int lh,int rh,const action &minimal={"",0,0,0},const int pat=0)
//minimal:舍去的最小牌型;pat:指定长度 
{
	action t;
	for (int i=lh; i<=rh; i++)
		if (h.straight[i])
		{
			if (pat&&pat!=h.straight[i]) continue;
			t={"straight",i,h.straight[i],0};
			if (!(t==minimal)) {temp=t; return 1;}
		}
	return 0;
}
bool find_strpair(const hand &h,action &temp,int lh,int rh,const int pat=0)//pat:指定长度 
{
	for (int i=lh; i<=rh; i++)
		if (h.strpair[i])
		{
			if (pat&&pat!=h.strpair[i]) continue;
			temp={"strpair",i,h.strpair[i],0};
			return 1;
		}
	return 0;
}
bool find_tri(const hand &h,action &temp,int lh,int rh,const action &minimal={"N",0,0,0},const int pat=0)
//minimal同上;pat:带牌的类型(-1,1,2) 
{
	action t;
	for (int i=lh; i<=rh; i++)
		if (h.tri[i])
		{
			if (pat&&pat!=attach(h.tri[i])) continue;
			t={"tri",i,attach(h.tri[i]),0};
			if (!(t==minimal)) {temp=t; return 1;}
		}
	return 0;
}
bool feigei(const hand &h,action &temp,int lh,int rh)
{
	int len,i,j;
	for (i=lh; i<=rh; i++)
		if (h.plane[i]!=0)
		{
			for (j=i,len=0; h.plane[j]!=0; j++) len++;//len:这个飞机的长度 
			temp={"plane",i,len,attach(h.plane[i])};
			return 1;
		}
	return 0;
}
bool find_bomb(const hand &h,action &temp,int lh,int rh)
{
	for (int i=lh; i<=rh; i++)
		if (h.bomb[i][0]==-1)
		{
			temp={"bomb",i,0,0};
			return 1;
		}
	return 0;
}
bool find_fours(const hand &h,action &temp,int lh,int rh,const int pat=0)
//pat:带牌的类型(1,2) 
{
	for (int i=lh; i<=rh; i++)
		if (h.bomb[i][0]>0)
		{
			if (pat&&pat!=attach(h.bomb[i][0])) continue;
			temp={"fours",i,attach(h.bomb[i][0]),0};
			return 1;
		}
	return 0;
}

action decide_off(hand &h)//on the offensive; -1:被动出牌状态下要考虑给队友过牌,1:正常出牌,2:对手报单/企图杀死比赛 
{
	int i,j,k;
	action temp;
	if (h.policy==1)
	{
		if (find_single(h,temp,1,7))                    {h.single[temp.p1]--; return temp;}//single 3~9
		if (find_pair(h,temp,1,8))                        {h.pair[temp.p1]--; return temp;}//pair 3~10
		if (find_straight(h,temp,1,6))                {h.straight[temp.p1]=0; return temp;}//straight 3~8
		if (find_strpair(h,temp,1,8))                  {h.strpair[temp.p1]=0; return temp;}//straight pairs 3~10
		if (find_tri(h,temp,1,8))                          {h.tri[temp.p1]=0; return temp;}//triple 3~10
		if (find_single(h,temp,8,10))                   {h.single[temp.p1]--; return temp;}//single 10~Q
		if (feigei(h,temp,1,9))//plane 3~J
		{
			for (i=temp.p1,j=1; j<=temp.p2; j++,i++) h.plane[i]=0;
			return temp;
		}
		//小牌尝试结束 
		if (find_tri(h,temp,9,12))                          {h.tri[temp.p1]=0; return temp;}//triple J~A
		if (find_strpair(h,temp,9,10))                  {h.strpair[temp.p1]=0; return temp;}//straight pairs J~Q
		if (find_fours(h,temp,1,13)) {h.bomb[temp.p1][0]=h.bomb[temp.p1][1]=0; return temp;}//all fours
		if (find_straight(h,temp,7,8))                 {h.straight[temp.p1]=0; return temp;}//straight 9~10
		if (feigei(h,temp,10,11))//plane Q,K
		{
			for (i=temp.p1,j=1; j<=temp.p2; j++,i++) h.plane[i]=0;
			return temp;
		}
		if (find_pair(h,temp,9,13))                        {h.pair[temp.p1]--; return temp;}//pair J~2
		if (find_single(h,temp,11,12))                   {h.single[temp.p1]--; return temp;}//single K,A
		if (find_tri(h,temp,13,13))                         {h.tri[temp.p1]=0; return temp;}//triple 2
		if (find_single(h,temp,13,15))                   {h.single[temp.p1]--; return temp;}//single 2,Jokers
		if (find_bomb(h,temp,1,14))                     {h.bomb[temp.p1][0]=0; return temp;}//all bombs
	}
	else if (h.policy==2)//如果决定一波流，那么不能打最小的牌(minimal)而要留着最后打。其他牌照常打。 
	{//cout<<"E"<<h.minimal.cat<<' '<<h.minimal.p1;system("pause");
		if (find_strpair(h,temp,1,10))                  {h.strpair[temp.p1]=0; return temp;}
		if (feigei(h,temp,1,11))//plane 3~K
		{
			for (i=temp.p1,j=1; j<=temp.p2; j++,i++) h.plane[i]=0;
			return temp;
		}
		if (find_fours(h,temp,1,13)) {h.bomb[temp.p1][0]=h.bomb[temp.p1][1]=0; return temp;}
		if (find_tri(h,temp,1,13,h.minimal))                {h.tri[temp.p1]=0; return temp;}
		if (find_straight(h,temp,1,8,h.minimal))       {h.straight[temp.p1]=0; return temp;}
		for (i=1; i<=15; i++)
		{
			if (find_single(h,temp,i,i,h.minimal))          {h.single[temp.p1]--; return temp;}
			if (find_pair(h,temp,i,i,h.minimal))              {h.pair[temp.p1]--; return temp;}
		}
		if (find_bomb(h,temp,1,14))                     {h.bomb[temp.p1][0]=0; return temp;}
	}
	else//防守出牌的话直接给最小牌 
	{
		if (find_single(h,temp,1,4)) 				     {h.single[temp.p1]--; return temp;}
		for (i=5; i<=11; i++)
		{
			if (find_single(h,temp,i,i))			     {h.single[temp.p1]--; return temp;}
			if (find_pair(h,temp,i-4,i-4))                 {h.pair[temp.p1]--; return temp;}
		}
		if (find_pair(h,temp,8,11))                        {h.pair[temp.p1]--; return temp;}
		if (find_straight(h,temp,1,8))                 {h.straight[temp.p1]=0; return temp;}
		if (find_strpair(h,temp,1,10))                  {h.strpair[temp.p1]=0; return temp;}
		if (find_tri(h,temp,1,13))                          {h.tri[temp.p1]=0; return temp;}
		if (find_pair(h,temp,12,13))                       {h.pair[temp.p1]--; return temp;}
		if (find_fours(h,temp,1,13)) {h.bomb[temp.p1][0]=h.bomb[temp.p1][1]=0; return temp;}
		if (find_single(h,temp,12,15)) 				     {h.single[temp.p1]--; return temp;}
		if (find_bomb(h,temp,1,14))                     {h.bomb[temp.p1][0]=0; return temp;}
	}
}

const double BIG_bonus=1.1;
double is_big(const action &a,int *rec=nullptr)//对敌方打出的牌是否有抢夺主动权的意图(e.g. 是否是大牌)进行判断 
{
	if (a.cat=="single")
		if (a.p1<12) return 0;
		else if (a.p1==12 && rec[13]>0) return 0.5;
		else if (a.p1==14&&rec[15] || a.p1==13&&(rec[14]>0)+(rec[15]>0)>0 || a.p1==12&&((rec[13]>0)+(rec[14]>0)+(rec[15]>0)>0))return 0.8;
	if (a.cat=="pair")
		if (a.p1<10) return 0;
		else if (a.p1<12) return ((rec[12]>=2)+(rec[13]>=2)+(a.p1==10&&rec[11]>=2)>=2?0.5:0.8);
		else if (a.p1==12) return ((rec[13]>=2)?0.8:1);
	if (a.cat=="tri")
		if (a.p1<8) return 0;
		else if (a.p1<11) return 0.7;
	return 1;
}

hand update_hand(int *remain,const hand &h)//在决定拆牌后，手牌信息存放在remain[]里面，在这里重新理手牌 
{
	hand temph;
	memcpy(temph.rec,h.rec,4*MAX_point),memcpy(temph.remain,remain,4*MAX_point);
	temph.maximize_single=h.maximize_single,temph.turn=h.turn,temph.policy=h.policy;
	a_optimal.pts=-INF;
	attribute(temph,1,temph.maximize_single);//d8g(a_optimal);system("pause");
	return a_optimal;
}

double def_bomb(int *remain,hand &h,action &act,const int last_bomb=0)//defensive bomb.出炸弹，如果没有炸弹可出，返回-INF 
{
	int i,j,optimal=0;
	double maxpts=-INF;
	for (i=last_bomb+1; i<=13; i++)
	{
		if (remain[i]<4) continue;
		remain[i]-=4;
		
		update_hand(remain,h);
		double pts=BIG_bonus+a_optimal.pts+c_bomb/2;
		if (pts>maxpts) maxpts=pts,optimal=i;
		
		remain[i]+=4;
	}
	if (remain[14]&&remain[15])
	{
		remain[14]--,remain[15]--;
		
		update_hand(remain,h);
		double pts=BIG_bonus+a_optimal.pts+c_bomb/2;
		if (pts>maxpts) maxpts=pts,optimal=14;//在有多重炸弹方法的时候，找最好的出炸弹方式 
		
		remain[14]++,remain[15]++;
	}
	if (maxpts>-INF)
	{
		if (optimal<14) remain[optimal]-=4;
		else remain[14]--,remain[15]--;
		
		h=update_hand(remain,h);
		act={"bomb",optimal,0,0};
		
		if (optimal<14) remain[optimal]+=4;
		else remain[14]++,remain[15]++;
		return maxpts;
	}
	return -INF;
}

action split_tri(int *remain,const action &pre,hand &h,bool alarm,const bool use_bomb=true)//拆三带并考虑打炸弹，以下同理 
//use_bomb:是否考虑炸弹 
{
	int i,j,k,optimal1=0,optimal2=0;//optimal1:三张,optimal2:带的牌 
	double maxpts=-INF;
	for (i=pre.p1+1; i<=13; i++)
		for (j=1; j<=13; j++)
		{
			if (i==j||remain[i]<3||remain[j]<pre.p2) continue;
			remain[i]-=3,remain[j]-=max(pre.p2,0);
		
			update_hand(remain,h);
			double pts=is_big({"tri",i,pre.p2,0},h.rec)*BIG_bonus+a_optimal.pts+pts_tri(i,pre.p2);
			if (pts>maxpts) maxpts=pts,optimal1=i,optimal2=j;
			
			remain[i]+=3,remain[j]+=max(pre.p2,0);
		}
	
	hand h_clone=h;
	action act_clone;
	double bombpts=def_bomb(remain,h_clone,act_clone);
	bool useBomb=0;
	if (use_bomb&&bombpts>maxpts) maxpts=bombpts,useBomb=1;//在打炸弹和拆三带之间找评分最大的组合 
	
	if (maxpts==-INF||(maxpts<h.pts-0.5&&maxpts<6666&&!alarm)) return {"N",0,0,0};
	if (useBomb)
	{
		h=h_clone;
		return act_clone;
	}
	else{
		remain[optimal1]-=3,remain[optimal2]-=max(pre.p2,0);
		h=update_hand(remain,h);
		return {"tri",optimal1,pre.p2,0};
	}
}
action split_straight(int *remain,const action &pre,hand &h,bool alarm,const bool use_bomb=true)
{
	int i,j,k,optimal;
	double maxpts=-INF;
	for (i=pre.p1+1; i<=8; i++)
	{
		for (j=i,k=0; remain[j]&&j<13&&k<pre.p2; j++) k++;
		if (k==pre.p2)
		{
			for (j=i,k=1; k<=pre.p2; j++,k++) remain[j]--;
			
			update_hand(remain,h);
			double pts=BIG_bonus+a_optimal.pts+c_straight(pre.p2);
			if (pts>maxpts) maxpts=pts,optimal=i;
			
			for (j=i,k=1; k<=pre.p2; j++,k++) remain[j]++;
		}
	}
	
	hand h_clone=h;
	action act_clone;
	double bombpts=def_bomb(remain,h_clone,act_clone);
	bool useBomb=0;
	if (use_bomb&&bombpts>maxpts) maxpts=bombpts,useBomb=1;
	
	if (maxpts==-INF||(maxpts<h.pts-0.5&&maxpts<6666&&!alarm)) return {"N",0,0,0};
	if (useBomb)
	{
		h=h_clone;
		return act_clone;
	}
	else{
		for (i=optimal,j=1; j<=pre.p2; j++,i++) remain[i]--;
		h=update_hand(remain,h);
		return {"straight",optimal,pre.p2,0};
	}
}
action split_strpair(int *remain,const action &pre,hand &h,bool alarm,const bool use_bomb=true)
{
	int i,j,k,optimal;
	double maxpts=-INF;
	for (i=pre.p1+1; i<=10; i++)
	{
		for (j=i,k=0; remain[j]>=2&&j<13&&k<pre.p2; j++) k++;
		if (k==pre.p2)
		{
			for (j=i,k=1; k<=pre.p2; j++,k++) remain[j]-=2;
			
			update_hand(remain,h);
			double pts=BIG_bonus+a_optimal.pts+c_strpair(pre.p2);
			if (pts>maxpts) maxpts=pts,optimal=i;
			
			for (j=i,k=1; k<=pre.p2; j++,k++) remain[j]+=2;
		}
	}
	
	hand h_clone=h;
	action act_clone;
	double bombpts=def_bomb(remain,h_clone,act_clone);
	bool useBomb=0;
	if (use_bomb&&bombpts>maxpts) maxpts=bombpts,useBomb=1;
	
	if (maxpts==-INF||(maxpts<h.pts-0.5&&maxpts<6666&&!alarm)) return {"N",0,0,0};
	if (useBomb)
	{
		h=h_clone;
		return act_clone;
	}
	else{
		for (i=optimal,j=1; j<=pre.p2; j++,i++) remain[i]-=2;
		h=update_hand(remain,h);
		return {"strpair",optimal,pre.p2,0};
	}
}
action split_plane(int *remain,const action &pre,hand &h,bool alarm,const bool use_bomb=true)
{
	int i,j,k,optimal=0,opt_attach[4]={0,0,0,0};
	double maxpts=-INF;
	if (pre.p2==3)
	{
		int l;
		for (i=pre.p1+1; i<=10; i++)
		{
			for (j=i,k=0; remain[j]>=3&&j<13&&k<pre.p2; j++) k++;
			if (k==pre.p2)
			{
				if (pre.p3==-1)//上一手飞机不带 
				{
					for (j=i,k=1; k<=pre.p2; j++,k++) remain[j]-=3;
					
					update_hand(remain,h);
					double pts=BIG_bonus+a_optimal.pts+c_plane(pre.p3)*3;
					if (pts>maxpts)
						maxpts=pts,optimal=i;
					
					for (j=i,k=1; k<=pre.p2; j++,k++) remain[j]+=3;
					continue;
				}
				//上一手飞机带单/对 
				for (j=i,k=1; k<=pre.p2; j++,k++) remain[j]-=3;
				
				for (j=1; j<=13; j++)
					for (k=j+1; k<=14; k++)
						for (l=k+1; l<=15; l++)
						{
							if (remain[j]<pre.p3||remain[k]<pre.p3||remain[l]<pre.p3) continue;
							if (in_interval(j,i,i+2)||in_interval(k,i,i+2)||in_interval(l,i,i+2)) continue;
							remain[j]-=pre.p3,remain[k]-=pre.p3,remain[l]-=pre.p3;
					
							update_hand(remain,h);
		
							double pts=BIG_bonus+a_optimal.pts+c_plane(pre.p3)*3;
							if (pts>maxpts)
							{
								maxpts=pts,optimal=i;
								opt_attach[1]=j,opt_attach[2]=k,opt_attach[3]=l;
							}
		
							remain[j]+=pre.p3,remain[k]+=pre.p3,remain[l]+=pre.p3;
						}
				
				for (j=i,k=1; k<=pre.p2; j++,k++) remain[j]+=3;
			}
		}
	}
	else if (pre.p2==2)
	{
		for (i=pre.p1+1; i<=11; i++)
		{
			for (j=i,k=0; remain[j]>=3&&j<13&&k<pre.p2; j++) k++;
			if (k==pre.p2)
			{
				if (pre.p3==-1)//飞机不带 
				{
					for (j=i,k=1; k<=pre.p2; j++,k++) remain[j]-=3;
					
					update_hand(remain,h);
					double pts=BIG_bonus+a_optimal.pts+c_plane(pre.p3)*2;
					if (pts>maxpts)
						maxpts=pts,optimal=i;
					
					for (j=i,k=1; k<=pre.p2; j++,k++) remain[j]+=3;
					continue;
				}
				//飞机带单/对 
				for (j=i,k=1; k<=pre.p2; j++,k++) remain[j]-=3;
				
				for (j=1; j<=14; j++)
					for (k=j+1; k<=15; k++)
					{
						if (remain[j]<pre.p3||remain[k]<pre.p3) continue;
						if (in_interval(j,i,i+2)||in_interval(k,i,i+2)) continue;
						remain[j]-=pre.p3,remain[k]-=pre.p3;
				
						update_hand(remain,h);
	
						double pts=BIG_bonus+a_optimal.pts+c_plane(pre.p3)*2;
						if (pts>maxpts)
						{
							maxpts=pts,optimal=i;
							opt_attach[1]=j,opt_attach[2]=k;
						}
		
						remain[j]+=pre.p3,remain[k]+=pre.p3;
					}
				
				for (j=i,k=1; k<=pre.p2; j++,k++) remain[j]+=3;
			}
		}
	}
	
	hand h_clone=h;
	action act_clone;
	double bombpts=def_bomb(remain,h_clone,act_clone);
	bool useBomb=0;
	if (use_bomb&&bombpts>maxpts) maxpts=bombpts,useBomb=1;
	
	if (maxpts==-INF||(maxpts<h.pts-0.5&&maxpts<6666&&!alarm)) return {"N",0,0,0};
	if (useBomb)
	{
		h=h_clone;
		return act_clone;
	}
	else{
		for (i=1,j=optimal; i<=pre.p2; i++,j++) remain[j]-=3;
		for (i=1; i<=pre.p2; i++) remain[opt_attach[i]]-=max(pre.p3,0);
		h=update_hand(remain,h);
		return {"plane",optimal,pre.p2,pre.p3};
	}
}

action decide_def(const action &pre,const int id,hand &h)//on the defensive. pre:上一手的打了什么牌; id:刚才出牌者的身份 
{
	int i,j,k,remain[MAX_point],nonsense[MAX_point];
	int last=decipher(player[id],nonsense);
	action act={"N",0,0,0};
	
	if (h.policy>0||(h.policy<0&&pre.cat!="single"&&pre.cat!="pair"&&pre.cat!="tri"))
	{
		if (pre.cat=="single")
		{//if (pre.p1==13) {cout<<"E???";system("pause");}
			if (((id>0)^(h.turn>0)==0)&&pre.p1>=13&&h.pts<GAME_OVER) return {"N",0,0,0};//不能压队友的大牌 
			bool fl=0;
			for (i=pre.p1+1; i<=12; i++)
				if (h.single[i]) {fl=1; break;}
			
			if (fl)
			{
				if (c_single(h)>0)
					for (i=12; i>pre.p1; i--){
						if (h.single[i]) {h.single[i]--; return {"single",i,0,0};}
					}
				else
					for (i=pre.p1+1; i<=12; i++){
						if (h.single[i]) {h.single[i]--; return {"single",i,0,0};}
					} 
			}
			else
			{
				decipher(h,remain);
				int optimal=0;
				double maxpts=-INF;
				for (i=pre.p1+1; i<=15; i++)
				{
					if (!remain[i]) continue;
					remain[i]--;
					
					update_hand(remain,h);
					double pts=is_big({"single",i,0,0},h.rec)*BIG_bonus+a_optimal.pts;
					//d8g(a_optimal);cout<<is_big({"single",i,0,0},h.rec)<<endl; system("pause");
					if (pts>maxpts) maxpts=pts,optimal=i;
					
					remain[i]++;
				}
				bool alarm=((id>0)^(h.turn>0))&&pre.p1>=13&&last<=6||h.policy<0;
				
				hand h_clone=h;
				action act_clone;
				double bombpts=def_bomb(remain,h_clone,act_clone);
				bool useBomb=0;
				if (bombpts>maxpts) maxpts=bombpts,useBomb=1;
	
				if (maxpts==-INF||(maxpts<h.pts-0.5&&maxpts<6666&&!alarm)) return {"N",0,0,0};
				if (useBomb)
				{
					h=h_clone;
					return act_clone;
				}
				else{
					remain[optimal]--;//for (i=1; i<=15; i++)cout<<remain[i]<<' ';cout<<endl;system("pause");
					h=update_hand(remain,h);
					act={"single",optimal,0,0};
					return act;
				}
			}
		}
		if (pre.cat=="pair")
		{
			if (!((id>0)^(h.turn>0))&&pre.p1>=13&&h.pts<GAME_OVER) {return {"N",0,0,0};}//不能压队友的大牌
			bool fl=0;
			for (i=pre.p1+1; i<=13; i++)
				if (h.pair[i]) {fl=1; break;}
			if (fl)
			{
				if (c_pair(h)>0)
					for (i=13; i>pre.p1; i--)
					{
						if (h.pair[i]) {h.pair[i]--; act={"pair",i,0,0}; return act;}
					}
				else
					for (i=pre.p1+1; i<=13; i++)
					{
						if (h.pair[i]) {h.pair[i]--; act={"pair",i,0,0}; return act;}//cout<<"E"<<i<<endl;system("pause");
					}
			}
			else{
				decipher(h,remain);
				int optimal=0;
				double maxpts=-INF;
				for (i=pre.p1+1; i<=13; i++)
				{
					if (remain[i]<2) continue;
					remain[i]-=2;
					
					update_hand(remain,h);
					double pts=is_big({"pair",i,0,0},h.rec)*BIG_bonus+a_optimal.pts;
					if (pts>maxpts) maxpts=pts,optimal=i;
					
					remain[i]+=2;
				}
				bool alarm=((id>0)^(h.turn>0))&&pre.p1>=12&&last<=6||h.policy<0;
				
				hand h_clone=h;
				action act_clone;
				double bombpts=def_bomb(remain,h_clone,act_clone);
				bool useBomb=0;
				if (bombpts>maxpts) maxpts=bombpts,useBomb=1;
				//cout<<"EE"<<maxpts;system("pause");
				if (maxpts==-INF||(maxpts<h.pts-0.5&&maxpts<6666&&!alarm)) return {"N",0,0,0};
				//不出牌的条件：无牌可出,有牌出但是太烂，而且没有报警、不能绝杀 
				if (useBomb)
				{
					h=h_clone;
					return act_clone;
				}
				else{
					remain[optimal]-=2;
					h=update_hand(remain,h);
					act={"pair",optimal,0,0};
					return act;
				}
			}
		}
		if (pre.cat=="tri")
		{
			if (!((id>0)^(h.turn>0))&&pre.p1>=8&&h.pts<GAME_OVER) return {"N",0,0,0};//是队友打的就手下留情
			
			if (find_tri(h,act,pre.p1+1,13,{"N",0,0,0},pre.p2)) {h.tri[act.p1]=0,h.pts-=pts_tri(act.p1,pre.p2); return act;}
			else{
				decipher(h,remain);
				bool alarm=((id>0)^(h.turn>0))&&pre.p1>=10&&last<=4;
				
				return split_tri(remain,pre,h,alarm);
			}
		}
		if (pre.cat=="plane")
		{
			if (!((id>0)^(h.turn>0))&&h.pts<GAME_OVER) return {"N",0,0,0};
			
			bool fl=0;
			for (i=pre.p1+1; i<=11; i++)
				if (h.plane[i]&&attach(h.plane[i])==pre.p3)
				{
					for (j=i,k=0; h.plane[j]&&attach(h.plane[j])==pre.p3&&k<pre.p2; j++) k++;
					if (k==pre.p2)
					{
						fl=1;
						act={"plane",i,pre.p2,pre.p3};
						for (j=i,k=1; k<=pre.p2; j++,k++) h.plane[j]=0;
						break;
					}
				}
			if (fl) {h.pts-=c_plane(pre.p3)*pre.p2; return act;}
			else{
				decipher(h,remain);
				bool alarm=((id>0)^(h.turn>0))&&last<=5||h.policy<0;
				
				return split_plane(remain,pre,h,alarm,!(h.policy<0&&last>=7));
			}
		}
		if (pre.cat=="straight")
		{
			if (find_straight(h,act,pre.p1+1,8,{"N",0,0,0},pre.p2)) {h.straight[act.p1]=0,h.pts-=c_straight(pre.p2); return act;}
			else{
				if (!((id>0)^(h.turn>0))&&h.pts<GAME_OVER) return {"N",0,0,0};
				
				decipher(h,remain);
				bool alarm=((id>0)^(h.turn>0))&&last<=2||h.policy<0;
				
				return split_straight(remain,pre,h,alarm,!(h.policy<0&&last>=5));
			}
		}
		if (pre.cat=="strpair")
		{
			if (!((id>0)^(h.turn>0))&&h.pts<GAME_OVER) return {"N",0,0,0};
				
			if (find_strpair(h,act,pre.p1+1,8,pre.p2)) {h.strpair[act.p1]=0,h.pts-=c_strpair(pre.p2); return act;}
			else{
				decipher(h,remain);
				bool alarm=((id>0)^(h.turn>0))&&last<=2||h.policy<0;
				
				return split_strpair(remain,pre,h,alarm,!(h.policy<0&&last>=5));
			}
		}
		if (pre.cat=="fours")
		{
			if (!((id>0)^(h.turn>0))&&h.pts<GAME_OVER) return {"N",0,0,0};
			if (find_fours(h,act,pre.p1+1,13,pre.p2)) {h.bomb[act.p1][0]=h.bomb[act.p1][1]=0,h.pts-=c_fours; return act;}
			else{
				decipher(h,remain);
				if (def_bomb(remain,h,act)>-INF) return act; else return {"N",0,0,0};
			}
		}
		if (pre.cat=="bomb")
		{
			if (!((id>0)^(h.turn>0))&&h.pts<GAME_OVER) return {"N",0,0,0};
			if (find_bomb(h,act,pre.p1+1,13)) {h.bomb[act.p1][0]=h.bomb[act.p1][1]=0,h.pts-=c_bomb; return act;}
			else{
				decipher(h,remain);
				bool alarm=last<=9;
				
				if (alarm)
					if (def_bomb(remain,h,act,pre.p1)>-INF) return act; else return {"N",0,0,0};
				else return {"N",0,0,0};
			}
		}
	}
	else if (h.policy==-1)//被动出牌 
	{
		if (!((id>0)^(h.turn>0))) return {"N",0,0,0};
		if (pre.cat=="single")
		{
			if (is_big(pre,h.rec)>0||last<=2)
			{
				for (i=pre.p1+1; i<=15; i++)
					if (h.single[i]) {h.single[i]--; return {"single",i,0,0};}
				
				decipher(h,remain);
				for (i=15; i>pre.p1; i--)
					if (remain[i])
					{
						remain[i]--;
						update_hand(remain,h);
						return {"single",i,0,0};
					}
				if (def_bomb(remain,h,act)>-INF) return act;
				else return {"N",0,0,0};
			}
			else{
				for (i=pre.p1+1; i<=min(pre.p1+4,13); i++)
					if (h.single[i]) {h.single[i]--; return {"single",i,0,0};}
				return {"N",0,0,0};
			}
		}
		if (pre.cat=="pair")
		{
			if (is_big(pre,h.rec)>0||last<=1)
			{
				for (i=pre.p1+1; i<=13; i++)
					if (h.pair[i]) {h.pair[i]--; return {"pair",i,0,0};}
				
				decipher(h,remain);
				for (i=13; i>pre.p1; i--)
					if (remain[i]>=2)
					{
						remain[i]-=2;
						update_hand(remain,h);
						return {"pair",i,0,0};
					}
				if (def_bomb(remain,h,act)>-INF) return act;
				else return {"N",0,0,0};
			}
			else{
				for (i=pre.p1+1; i<=min(pre.p1+3,13); i++)
					if (h.pair[i]) {h.pair[i]--; return {"pair",i,0,0};}
				return {"N",0,0,0};
			}
		}
		if (pre.cat=="tri")
		{
			if (is_big(pre,h.rec)>0||last<=2)
			{
				for (i=pre.p1+1; i<=13; i++)
					if (h.tri[i]&&attach(h.tri[i])==pre.p2) {h.tri[i]=0; return {"tri",i,pre.p2,0};}
				
				decipher(h,remain);
				return split_tri(remain,pre,h,true);//尽量压死 
			}
			else{
				for (i=pre.p1+1; i<=min(pre.p1+3,13); i++)
					if (h.tri[i]&&attach(h.tri[i])==pre.p2) {h.tri[i]=0; return {"tri",i,pre.p2,0};}
				return {"N",0,0,0};
			}
		}
	}
}

int tot(int x){return x<=13?4:1;}//返回有几张点数为x的牌 

int main()
{
	srand((int)time(NULL));
	while (true)
	{
		int n,i,j,k,x;
		int former[MAX_point],latter[MAX_point],lastid=0;
		action act,lastact;
		for (j=0; j<3; j++) init(player[j],j);
		
		int H[3][20],num[54],bigs[3]={0,0,0},lord;//这一段表示随机发牌 
		bool fl[54];
		memset(fl,0,sizeof(fl));
		for (i=1; i<=15; i++)
			for (j=1; j<=tot(i); j++)
			{
				x=rand()%54;
				while (fl[x]) x=rand()%54;
				num[x]=i,fl[x]=1;
			}
		for (i=k=0; i<51; i++)
		{
			H[i/17][i%17]=num[i];
			if (num[i]>=13) bigs[i/17]++;
		}
		int R=rand()%8;
		if (R<bigs[0]+1) lord=0;
		else if (R<bigs[0]+bigs[1]+2) lord=1;
		else lord=2;
		for (i=0; i<3; i++) H[lord][i+17]=num[i+51];
		
		for (i=0; i<20; i++)
			x=H[lord][i],player[0].rec[x]--,player[0].remain[x]++;
		for (i=0,j=1; i<3; i++)
		{
			if (i==lord) continue;
			for (k=0; k<17; k++)
				x=H[i][k],player[j].rec[x]--,player[j].remain[x]++;
			j++;
		}
		
		a_optimal.pts=-INF,attribute(player[0],1),player[0]=a_optimal;
		a_optimal.pts=-INF,attribute(player[1],1),player[1]=a_optimal;
		a_optimal.pts=-INF,attribute(player[2],1),player[2]=a_optimal;
		cout<<"Original State:\nPlayer 0(landlord):\n"; decipher(player[0],former,true);
		cout<<"Player 1:\n"; decipher(player[1],former,true); cout<<"Player 2:\n"; decipher(player[2],former,true);
		system("pause");

//		cout<<"input the landlord's hand:";   //这一段由用户输入手牌调试,1~11表示3~K; 12~13表示A,2; 14,15:joker。 
//		for (i=1; i<=20; i++)
//		{
//			cin>>x;
//			player[0].rec[x]--,player[0].remain[x]++;
//		}
//		cout<<"input the 1st farmer's hand:";
//		for (i=1; i<=17; i++)
//		{
//			cin>>x;
//			player[1].rec[x]--,player[1].remain[x]++;
//		}
//		cout<<"input the 2nd farmer's hand:";
//		for (i=1; i<=17; i++)
//		{
//			cin>>x;
//			player[2].rec[x]--,player[2].remain[x]++;
//		}
//		a_optimal.pts=-INF,attribute(player[0],1),player[0]=a_optimal; //d8g(a_optimal); system("pause");
//		a_optimal.pts=-INF,attribute(player[1],1),player[1]=a_optimal;
//		a_optimal.pts=-INF,attribute(player[2],1),player[2]=a_optimal;
		
		//这一段负责打牌 
		for (i=0; true; i=(i+1)%3)
		{
			decipher(player[i],former);
			printf("Player %d:\n",i);
			if (i==lastid)//此时相当于一轮出牌结束，需要重新理牌.
			{
				if (i>0&&decipher(player[0],latter)<=1)//地主报单 
				{
					player[0].policy=player[2].policy=player[1].policy=1;
					reattribute(player[0]);
					reattribute(player[1],true),player[1].maximize_single=true;
					reattribute(player[2],true),player[2].maximize_single=true;
					if ((player[1].pts-player[2].pts)/TRY_TO_WIN<-2) player[1].policy=-2;
				}
				else{
					player[0].policy=player[2].policy=player[1].policy=1;
					reattribute(player[1]);
					reattribute(player[2]);
					if (i==0&&(decipher(player[1],latter)<=1||decipher(player[2],latter)<=1))//农民报单 
						{reattribute(player[0],true),player[0].maximize_single=true;}
					else
						reattribute(player[0]);
					if (player[1].pts<3&&player[1].pts<=player[2].pts-2) player[1].policy=-1;
					if (player[2].pts>=TRY_TO_WIN) player[1].policy=1;//如果队友准备胜利,那么要准备好辅助。 
				}
				
				act=decide_off(player[i]); lastact=act,lastid=i;
				cout<<act.cat;prcard(act.p1);cout<<' '<<act.p2<<' '<<act.p3<<endl;
				if (decipher(player[i],latter,true)==0) {cout<<"END\n";break;}
				system("pause");
			}
			else
			{
				act=decide_def(lastact,lastid,player[i]);
				if (act.cat=="N") {cout<<"PASS\n";decipher(player[i],latter,true);}
				else {
					lastact=act,lastid=i;
					cout<<act.cat;prcard(act.p1);cout<<' '<<act.p2<<' '<<act.p3<<endl;
					if (decipher(player[i],latter,true)==0) {cout<<"END\n";break;}
				}
				system("pause");
			}
			decipher(player[i],latter);
			for (j=0; j<=2; j++)
			{
				if (j==i) continue;
				for (k=1; k<=15; k++)
					player[j].rec[k]+=latter[k]-former[k];//更新记牌器信息 
			}
		}
	}
	return 0;
}
