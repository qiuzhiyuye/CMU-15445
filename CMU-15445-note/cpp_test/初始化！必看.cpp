#include<bits/stdc++.h>
using namespace std; 
//测试初始化
class obj{
public:
	int x_;
	obj()=default;
	obj(int x):x_(x){}
	int get_x(){
		return x_;
	}
};
signed main (){
	// int *p=nullptr;
	// *p=int(2);//这个是括号初始化+ 修改 *p(*p可以视为是一个实体) 
	// cout<<p<<" "<<(*p)<<"\n";

	//1.  带名字新建一个对象  
	obj obj_name(2);//obj_name后面的括号始终是构造函数
	//记住括号是构造函数，这里简单看得懂，套上了智能指针就开始犯浑了


	//2.  不带名字新建对象
    vector<obj>v;
	v.push_back(obj(2));
	//这里是 obj() 类名+(构造函数)

	//3.普通指针
    obj* p=nullptr;
	p=&obj_name;     //obj_name实体 的地址传给p
	*p=obj(3);// 这个是2中不带名字的新建对象

    //4.智能指针
	unique_ptr<obj> q(new obj(4));//这里是对象+名称的新建方式，看得懂
	//代表着obj对象的unique指针
	vector<unique_ptr<obj>>vv;
	vv.push_back(unique_ptr<obj>(new obj(4)));
	//这里是pb一个新建的对象
	//新建对象没有名字
	//用 对象(构造函数) 形式
	//这里可以看出来 unique_ptr<?> 构造函数可以传入 一个？类型的指针
	// new A() 返回值是一个A类型指针
	//A() 又是A的构造函数 ，所以这里面两层括号都解释清楚了。

	//5.智能指针的指针：
	unique_ptr<obj> *point;
	point=&q;
	cout<<(*point)->get_x()<<"here\n";
	//这里就很奇怪，(*point)说明是一个对象，是unique_ptr<obj>的对象
	//但是这里就可以直接用obj的成员函数了。记住就好了
	//unique_ptr<obj> 约等于obj对象 就是理解为普通指针 即可 p是普通指针时候，(p)->？？
	
	
	
	
	
	
	
	return 0;
}

