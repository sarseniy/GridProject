#include <iostream>
#include <string>

class Handler {
public:
	virtual void* data() = 0;
	virtual void const* data() const = 0;
	virtual ~Handler()
	{}
};

template <typename T>
class TipicalHandler : public Handler {
public:
	TipicalHandler(T value) : value(value)
	{}

	void* data() override {
		return static_cast<void*>(&value);
	}

	void const* data() const override {
		return static_cast<void const*>(&value);
	}
private:
	T value;
};

class Any {
public:
	Any() : data_(nullptr)
	{}

	template <typename T>
	Any(T value) : data_(new TipicalHandler<T>(value)) 
	{}

	~Any() {
		delete data_;
	}

	template <typename T>
	void replace(T value) {
		delete data_;
		data_ = new TipicalHandler<T>(value);
	}

	template <typename T>
	T& as() {
		auto w = dynamic_cast<TipicalHandler<std::decay_t<T>>&>(*data_);
		return *static_cast<std::decay_t<T>*>(w.data());
	}

	template <typename T>
	T const& as() const {
		auto w = dynamic_cast<TipicalHandler<std::decay_t<T>> const&>(*data_);
		return *static_cast<std::decay_t<T> const*>(w.data());
	}

private:
	Handler* data_;
};

template <typename T>
class Grid {
public:
	Grid(size_t x_size, size_t y_size) : x_size(x_size), y_size(y_size) {
		memory = new T [x_size * y_size];
		for (size_t i = 0; i < x_size * y_size; i++)
		{
			memory[i] = 0;
		}
	}
	
	Grid(const Grid& ref) : x_size(ref.x_size), y_size(ref.y_size) {
		memory = new T[x_size * y_size];
		for (size_t i = 0; i < x_size * y_size; i++)
		{
			memory[i] = ref.memory[i];
		}
	}

	~Grid() {
		delete[] memory;
	}

	Grid& operator=(const Grid& ref) {
		if (this == &ref)
		{
			return *this;
		}
		delete[] this->memory;
		x_size = ref.get_xsize();
		y_size = ref.get_ysize();

		memory = new T[x_size * y_size];
		for (size_t i = 0; i < x_size * y_size; i++)
		{
			memory[i] = ref.memory[i];
		}
		return *this;
	}

	T const& operator()(size_t x_idx, size_t y_idx) const {
		return memory[x_idx * y_size + y_idx];
	}

	T& operator()(size_t x_idx, size_t y_idx) {
		return memory[x_idx * y_size + y_idx];
	}

	size_t get_xsize() const {
		return x_size;
	}

	size_t get_ysize() const {
		return y_size;
	}

	Grid& operator=(T value) {
		for (size_t i = 0; i < x_size * y_size; i++)
		{
			memory[i] = value;
		}
		return *this;
	}

private:
	T* memory;
	size_t x_size, y_size;
};

template <typename T>
std::istream& operator>>(std::istream& f, Grid<T>& g) {
	for (size_t i = 0; i < g.get_xsize() * g.get_ysize(); i++)
	{
		f >> g(i / g.get_ysize(), i % g.get_ysize());
	}
	return f;
}

template <typename T>
std::ostream& operator<<(std::ostream& f, Grid<T> const& g) {
	for (size_t i = 0; i < g.get_xsize(); i++)
	{
		for (size_t j = 0; j < g.get_ysize(); j++)
		{
			f << g(i, j) << '\t';
		}
		f << '\n';
	}
	f << '\n';
	return f;
}

class foo {
public:
	void print() {
		std::cout << "print()";
	}

	void print() const {
		std::cout << "print() const";
	}
};

int main()
{
	/*Grid<double> my_grid(4, 3);
	std::cout << my_grid;
	my_grid = 8;
	std::cout << my_grid;

	std::cin >> my_grid;
	std::cout << my_grid;

	Grid<double> second = my_grid;
	Grid<double> third(5, 5);
	third = 10;
	second = third;*/

	char* f = new char [3];
	f[0] = 'H';
	f[1] = 'i';
	f[2] = '\0';

	Any num(36);
	Any str(f);
	Any fact(true);

	std::cout << num.as<int>() << ' ' << str.as<char*>() << ' ' << fact.as<bool>();

	Any cls(foo{});
	cls.as<foo>().print();

	Any const& rcls = cls;
	rcls.as<foo>().print();


	delete[] f;

	return 0;
}
