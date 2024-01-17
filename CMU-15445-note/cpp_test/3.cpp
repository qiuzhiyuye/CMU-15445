#include<bits/stdc++.h>
using namespace std; 
//测试this->
int x=3;
class A{
public:
	int x=2;
	int get_x(){
		return x;
	}
	int get_this_x(){
		return this->x;
	}
	
};
signed main (){
	A a;
	cout<<a.get_x()<<"\n";
	cout<<a.get_this_x();
	
	
	
	
	
	
	
	
	
	
	
	
	return 0;
}

