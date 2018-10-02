#pragma  once

namespace LC {

//引用计数类，自动构造和析构，放在其他类中用于判断动态数据是否需要析构（使用needToRelease()函数）
class ReferenceCounter {
private:
	int* ref=nullptr; //引用计数

	inline bool isRefZero() { //判断引用计数是否为0
		return (*ref) <= 0;
	}
public:

	inline void allocateRef() { //分配引用计数，初始化的值为1
		ref = new int(1);
	}

	inline void increaseRef() { //增加引用计数
		++(*ref);
	}
	//是否已经分配了引用计数
	inline bool isAllocatedRef() const {
//        return ref>0;
		return ref != nullptr;
	}

	//获取当前的引用计数的值
	inline int getRefCounter() const {
		if (isAllocatedRef())
			return *ref;
		else
			return 0;
	}

	inline bool needToRelease() {
		return *ref == 1;
	}

private:
	void decreaseRef() {
		if (nullptr == ref) //未分配内存or未采用引用计数（未采用引用计数时不负责释放指针）
			return;
		else if (needToRelease())
			delete ref;
		else
			--(*ref);
	}
	//end of 引用计数及内存分配  *************************************************
public:
	//begin of Constructors    *************************************************

	//返回一个分配了num个数据的实例，num=0时不分配内存
	explicit ReferenceCounter() {
		allocateRef();
	}

	~ReferenceCounter() {
		decreaseRef();
		// #ifndef NDEBUG
		// 		std::cout<<"Deconstructor:---->DataWithReferenceCounterAutoConstruct"<<endl;
		// #endif
	}

	ReferenceCounter& operator=(const ReferenceCounter& m) {
		if (this == &m)
			return *this;
		if (isAllocatedRef())
			decreaseRef();
		this->ref = m.ref;
		if (isAllocatedRef())
			increaseRef();
		return *this;
	}

	//拷贝构造函数，不进行数据的深拷贝，只复制指针
	ReferenceCounter(const ReferenceCounter& m) {
		this->ref = m.ref;
		if (isAllocatedRef())
			increaseRef();
	}

	//end   of Constructors    *************************************************

};

}
