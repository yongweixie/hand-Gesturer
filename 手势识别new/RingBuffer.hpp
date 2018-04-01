#pragma once
#include<vector>

using namespace std;

template<class T>
class ringBuffer
{
public:
	ringBuffer(int size) {
		buf = new T[size]();
		_size = size;
	}
	void push_back(T dat) {
		buf[pos] = dat;
		pos < _size - 1 ? pos += 1 : pos = 0;
	}
	T &operator [] (int n) {
		int i = pos + n;
		return buf[i < _size ? i : i - _size];
	}
	size_t size() {
		return _size;
	}
	void clear() {
		delete[] buf;
		buf = new T[_size]();
	}
private:
	T* buf;
	int _size = 0, pos = 0;
};
#pragma once
