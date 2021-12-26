#ifndef Utils_h__
#define Utils_h__

#include <functional>

std::function<double(unsigned char*)> getFuncPcmToReal(int bits);

template<int N> struct PowerOfTwo {
	static const int Result = PowerOfTwo<N - 1>::Result * 2;
};

template<> struct PowerOfTwo<0> {
	static const int Result = 1;
};


enum class WindowFunction {
	NoWindow,
	GuassWindow,
	HannWindow,
	HammingWindow,
	BartlettWindow,
	TriangleWindow,
	BlackmanWindow,
	NuttallWindow,
	SinWindow
};
std::vector<double> createWindowFunction(int size, WindowFunction funcType);

#endif // Utils_h__
