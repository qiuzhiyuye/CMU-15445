#include<bits/stdc++.h>
using namespace std; 
//测试union
struct node1{
	char x;
	short y;
	int z;
};
union node2{
	int x;
	short y;
	int z;
};
signed main (){
//	cout<<sizeof(node1)<<"\n";
//	cout<<sizeof(node2)<<"\n";
	node2 a;
	cout<<(&a.x)<<" x\n";
	cout<<(&a.y)<<" y\n";
	cout<<(&a.z)<<" z\n";
	
	
	
	
	
	
	
	
	
	return 0;
}

