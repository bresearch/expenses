#ifndef FMT_H_
#define FMT_H_

#include <iostream>
#include <sstream>
// #include <type_traits>


namespace expenses {
	// forward declaration for class Format
	template<typename T> class Format;

	// first create the Binder;
	template<typename Real>
		struct Binder
		{
			Format<Real>& f;
			Real val;
		};

	template<typename Num>
		class Format
		{
			static_assert(std::is_arithmetic<Num>::value,
						  "An arithmetic type expected");
			template<typename T> 
				friend	typename std::enable_if<std::is_arithmetic<T>::value,
				std::ostream&>::type
				operator<<(std::ostream& os, const Binder<T>& binder);

		public:
		Format(int p, int w, std::ios_base::fmtflags f) :
			prc{p}, width{w}, fmt{f}{}
	
			// bind it to the value to be printed:
			Binder<Num> operator()(Num d)
				{
					return Binder<Num>{*this, d};
				}
	
			Format& setPrecision(int p) { prc = p; return *this;}
			Format& setWidth(int w) { width = w; return *this; }
			Format& scientific() { fmt = std::ios_base::scientific; return *this; }
			Format& general() { fmt = {}; return *this; }
			Format& fixed() { fmt = std::ios_base::fixed; return *this; }
			Format& hex() { fmt = std::ios_base::hex; return *this; }
			Format& dec() { fmt = std::ios_base::dec; return *this; }
			Format& boolAlpha() { fmt = std::ios_base::boolalpha; return *this;}
			Format& fill(char ch) { fChar = ch; }
			int getWidth() const { return width; }
		private:
			int prc;
			int width;
			std::ios_base::fmtflags fmt;
			char fChar {'*'};
			std::ios_base::fmtflags ffmt{std::is_integral<Num>::value
					? std::ios_base::basefield: std::ios_base::floatfield};
		};

	template<>
		struct Format<bool> : public Format<int>
		{
		Format() : Format<int>({}, {}, {std::ios_base::boolalpha}) {}	
			Binder<bool> operator()(bool b)
				{
					return Binder<bool>{*this, b};
				}
		};

	template<>
		struct Format<std::string>
		{
		Format(int w, std::ios_base::fmtflags f={}, char ch='*') :
			width{w}, fmt{f}, fChar{ch} {}
			Binder<std::string> operator()(const std::string& s)
				{
					return Binder<std::string>{*this, s};
				}
			void fill(char ch) { fChar = ch; }
			int getWidth() const { return width; }
			void setWidth(int w) { width = w; }
			int width;
			std::ios_base::fmtflags fmt;
			char fChar;
		};

	
	// now create the operator to print a Binder object:
	template<typename T>
		typename std::enable_if<std::is_arithmetic<T>::value, std::ostream&>::type
		operator<<(std::ostream& os, const Binder<T>& binder);

	std::ostream& operator<<(std::ostream& os, const Binder<std::string>& binder);

	// now create the operator to print a Binder object:
	/*
	template<typename T>
	typename std::enable_if<std::is_arithmetic<T>::value, std::ostream&>::type
	operator<<(std::ostream& os, const Binder<T>& binder)
	{
		std::ostringstream oss;
		oss.precision(binder.f.prc);
		oss.width(binder.f.width);
		oss.fill(binder.f.fChar);
		oss.setf(binder.f.fmt, binder.f.ffmt);
		oss << binder.val;
		os << oss.str();
		return os;
	}

	std::ostream& operator<<(std::ostream& os, const Binder<std::string>& binder)
	{
		std::ostringstream oss;
		oss.width(binder.f.width);
		oss.fill(binder.f.fChar);
		oss.setf(binder.f.fmt, std::ios_base::adjustfield);
		oss << binder.val;
		os << oss.str();
		return os;
		}*/

} // namespace expenses
#endif
