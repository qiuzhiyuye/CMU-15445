#include<bits/stdc++.h>
using namespace std; 
typedef long long ll; 
#define int long long 
#define ull unsigned long long
#define CY printf("YES\n")
#define CN printf("NO\n")
#define x first
#define y second
#define PII pair<int,int > 
#define de(x) cout<<#x<<'='<<x<<'\n'
#define dde(x,y) cout<<#x<<'='<<x<<","<<#y<<"="<<y<<'\n'
#define rep(i, a, b) for(int i = (a); i <= (b); i ++)
#define dec(i, a, b) for(int i = (a); i >= (b); i --)
#define vi vector<int>
#define vpii vector<PII>
#define pb push_back
#define rvs(s) reverse(s.begin(),s.end())
#define all(s) s.begin(),s.end()
int qmi(int a, int k);
int mo(int x,int p){return x = ((x%p)+p)%p;}
const int N=2000007;
const int mod=1e9+7;
const int inf=0x3f3f3f3f3f3f3f3f;
const double eps=1e-8;
int dx[4]={-1,0,1,0};
int dy[4]={0,1,0,-1};
//成员变量和函数之间是没有顺序关系的，不需要和外面一样，调用的函数要定义在调用点之前
class A{
public:	
	int get_(){
		return get_x();
	}
	int get_x(){
		return this->x;
	}
	A()=default;
	~A()=default;
private:
	int x{122};
};
signed main (){
	A a;
	cout<<a.get_();//正确
	
	
	
	
	
	
	
	
	
	
	
	
	
	return 0;
}
int qmi(int a, int k){int res = 1;while (k){if (k & 1) res = (ll)res * a % mod;a = (ll)a * a % mod;k >>= 1;}return res;}
