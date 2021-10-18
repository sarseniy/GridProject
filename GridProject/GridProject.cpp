#include <iostream>
#include <string>
#include <typeinfo>

class Handler {
public:
	virtual void* data() = 0;
	virtual void const* data() const = 0;
	virtual std::type_info const& get_type() const = 0;
	virtual ~Handler()
	{}
};

template <typename T>
class TypicalHandler : public Handler {
public:
	TypicalHandler(T value) : value(value)
	{}

	void* data() override {
		return static_cast<void*>(&value);
	}

	void const* data() const override {
		return static_cast<void const*>(&value);
	}

	std::type_info const& get_type() const override {
		return typeid(value);
	}

private:
	T value;
};

class Any {
public:
	Any() : data_(nullptr)
	{}

	template <typename T>
	Any(T value) : data_(new TypicalHandler<T>(value)) 
	{}

	template <typename T>
	Any(const Any& ref) {
		delete[] data_;
		data_ = new TypicalHandler<T>(ref.as<T>());
	}

	~Any() {
		delete data_;
	}

	template <typename T>
	void replace(const Any& ref) {
		delete data_;
		data_ = new TypicalHandler<T>(ref.as<T>());
	}

	template <typename T>
	void replace(const T& value) {
		delete data_;
		data_ = new TypicalHandler<T>(value);
	}

	template <typename T>
	T& as() {
		auto& w = dynamic_cast<TypicalHandler<std::decay_t<T>>&>(*data_);
		return *static_cast<std::decay_t<T>*>(w.data());
	}

	template <typename T>
	T const& as() const {
		auto const& w = dynamic_cast<TypicalHandler<std::decay_t<T>> const&>(*data_);
		return *static_cast<std::decay_t<T> const*>(w.data());
	}

	template<typename T>
	bool contains() const {
		return typeid(T) == data_->get_type();
	}

private:
	Handler* data_;
};

template <typename T>
class Grid {
public:
	Grid(size_t x_size, size_t y_size) : x_size(x_size), y_size(y_size) {
		memory = new Any [x_size * y_size];
		for (size_t i = 0; i < x_size * y_size; i++)
		{
			memory[i].replace((T)0);
		}
	}
	
	Grid(const Grid& ref) : x_size(ref.x_size), y_size(ref.y_size) {
		memory = new Any [x_size * y_size];
		for (size_t i = 0; i < x_size * y_size; i++)
		{
			memory[i].replace<T>(ref.memory[i]);
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

		memory = new Any[x_size * y_size];
		for (size_t i = 0; i < x_size * y_size; i++)
		{
			memory[i].replace<T>(ref.memory[i]);
		}
		return *this;
	}

	T operator()(size_t x_idx, size_t y_idx) const {
		if (this->is_subgrid(x_idx, y_idx)) {
			return memory[x_idx * y_size + y_idx].as<Grid<T>>().average();
		}
		return memory[x_idx * y_size + y_idx].as<T>();
	}

	T& operator()(size_t x_idx, size_t y_idx) {
		if (this->is_subgrid(x_idx, y_idx)) {
			return memory[0].as<T>(); // UNDEFINED BEHAVIOUR
		}
		return memory[x_idx * y_size + y_idx].as<T>();
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
			if (!this->is_subgrid(i / y_size, i % y_size)) {
				memory[i].replace(value);
			}
			else
			{
				memory[i].as<Grid<T>>() = value;
			}
		}
		return *this;
	}

	Grid& make_subgrid(size_t x_idx, size_t y_idx, size_t x_sub_size, size_t y_sub_size) {
		auto old_val = this->is_subgrid(x_idx, y_idx) ? (T)0 : (*this)(x_idx, y_idx);
		Grid<T> tmp(x_sub_size, y_sub_size);
		tmp = old_val;
		memory[x_idx * y_size + y_idx].replace<Grid<T>>(tmp);
		return *this;
	}

	Grid& collapse_subgrid(size_t x_idx, size_t y_idx) {
		if (!this->is_subgrid(x_idx, y_idx))
		{
			return *this;
		}
		memory[x_idx * y_size + y_idx].replace<T>(memory[x_idx * y_size + y_idx].as<Grid<T>>().average());
		return *this;
	}

	Grid& get_subgrid(size_t x_idx, size_t y_idx) {
		return memory[x_idx * y_size + y_idx].as<Grid<T>>();
	}

	Grid const& get_subgrid(size_t x_idx, size_t y_idx) const {
		return memory[x_idx * y_size + y_idx].as<Grid<T>>();
	}

	bool is_subgrid(size_t x_idx, size_t y_idx) const {
		return memory[x_idx * y_size + y_idx].contains<Grid<T>>();
	}

private:
	T average() {
		T sum = 0;
		for (size_t i = 0; i < x_size * y_size; i++)
		{
			sum += (*this)(i / y_size, i % y_size);
		}
		return sum / (T)(x_size * y_size);
	}

	Any* memory;
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

class Error
{
public:
	void virtual perr() = 0;
};

class ConstructorAndIOSError : public Error
{
public:
	void perr() override {
		std::cout << "Something wrong with Constructor or IOS\n";
	}
};

class CopyError : public Error
{
public:
	void perr() override {
		std::cout << "Something wrong with Copy\n";
	}
};

class SubgridError : public Error
{
public:
	void perr() override {
		std::cout << "Something wrong with Subgrid\n";
	}
};

void testConstructorAndIOS() {
	bool flag = true;
	Grid<double> a(2, 2);
	a = 9;
	a(1, 1) = 98.56;
	if (a(0, 0) != 9 or a(1, 1) != 98.56) flag = false;

	Grid<double> b(1, 2);
	std::cin >> b;
	double* b_ = new double[2];
	for (size_t i = 0; i < 2; i++)
	{
		std::cin >> b_[i];
	}
	if (b(0, 0) != b_[0] or b(0, 1) != b_[1]) flag = false;
	delete[] b_;

	ConstructorAndIOSError err;
	if (!flag) throw err;
}

void testCopy() {
	bool flag = true;

	Grid<double> a(2, 2);
	a = 9;
	a(1, 1) = 98.56;

	Grid<double> c = a;
	if (c(0, 0) != 9 or c(1, 1) != 98.56 or c(1, 0) != 9) flag = false;

	Grid<double> b(1, 2);
	b = 6;
	a = b;
	if (b(0, 0) != a(0, 0) or b(0, 1) != a(0, 1) or b.get_xsize() != a.get_xsize()) flag = false;

	CopyError err;
	if (!flag) throw err;
}

void testSubgrid() {
	bool flag = true;

	Grid<double> a(2, 2);
	a = 9;
	a(1, 1) = 98.56;
	a.make_subgrid(1, 0, 5, 5);
	Grid<double>& sg_a = a.get_subgrid(1, 0);
	sg_a = 0;
	sg_a(0, 0) = 50;
	if (!a.is_subgrid(1, 0) or sg_a(4, 3) != 0 or sg_a(0, 0) != 50) flag = false;

	a.collapse_subgrid(1, 0);
	if (a(1, 0) != 2 or a.is_subgrid(1, 0)) flag = false;

	a = 1;
	a.make_subgrid(1, 0, 5, 5);
	a.make_subgrid(1, 0, 2, 2);
	Grid<double>& sg_a_ = a.get_subgrid(1, 0);
	if (sg_a_(0, 0) != 0 or sg_a_(1, 1) != 0) flag = false;

	SubgridError err;
	if (!flag) throw err;
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
	try
	{
		testConstructorAndIOS();
		testCopy();
		testSubgrid();
	}
	catch (ConstructorAndIOSError err)
	{
		err.perr();
	}
	catch (CopyError err)
	{
		err.perr();
	}
	catch (SubgridError err)
	{
		err.perr();
	}
	catch (...)
	{
		std::cout << "Something else went wrong\n";
	}

	/*Grid<double> my_grid(4, 3);
	std::cout << my_grid;
	my_grid = 8;
	my_grid(2, 0) = 123;
	std::cout << my_grid;

	std::cin >> my_grid;
	std::cout << my_grid;

	Grid<double> second = my_grid;
	std::cout << second(0, 1) << '\n';

	Grid<double> third(5, 5);
	third = 10;
	second = third;

	second.make_subgrid(1, 1, 5, 5);

	std::cout << second.is_subgrid(1, 1) << '\n';

	second.make_subgrid(1, 1, 5, 5);
	second.collapse_subgrid(1, 1);

	std::cout << second.is_subgrid(1, 1) << '\n';*/

	return 0;
}
