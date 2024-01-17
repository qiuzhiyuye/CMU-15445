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
// 访问控制符 的测试
class A{
public:	
	bool get_is_(){
		return this->is_;
	}
protected:
	bool is_{true};
};
signed main (){
	A a;
	cout<<a.get_is_();//正确
//	cout<<a.is_;//错误
	
	
	
	
	
	
	
	
	
	
	
	
	return 0;
}
int qmi(int a, int k){int res = 1;while (k){if (k & 1) res = (ll)res * a % mod;a = (ll)a * a % mod;k >>= 1;}return res;}
