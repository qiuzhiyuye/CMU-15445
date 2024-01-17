#include<bits/stdc++.h>
using namespace std; 
//测试static
class A{
public:
	static int x{1};
	A()=default;
private:
	static int y{1};
};
signed main (){
	A a;
	cout<<A::x;//正确
	//cout<<a::y;
	//cout<<a.x;
	
	
	
	
	
	
	
	
	
	
	
	
	return 0;
}

