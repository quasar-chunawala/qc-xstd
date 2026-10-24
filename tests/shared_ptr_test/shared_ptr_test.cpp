#include <atomic>
#include <chrono>
#include <gtest/gtest.h>
#include <thread>

#include <core/smart_pointers/shared_ptr.h>

using namespace std::chrono_literals;

// make_shared<T>(constructor_args) test
TEST(SharedPtrTest, MakeSharedTest) {
	struct Point2D {
		double x;
		double y;

		Point2D(double x_, double y_) : x{x_}, y{y_} {}

		~Point2D() = default;
	};

	xstd::shared_ptr<Point2D> sptr = xstd::make_shared<Point2D>(3.0, 5.0);
	EXPECT_NE(sptr, nullptr);
	EXPECT_EQ(sptr->x, 3.0);
	EXPECT_EQ(sptr->y, 5.0);
	EXPECT_EQ(sptr.use_count(), 1);
}

TEST(SharedPtrTest, ResetSharedPtr) {
	struct X {
		X(int n_) : n(n_) {}

		int get_n() { return n; }

		~X() = default;
		int n;
	};

	xstd::shared_ptr<X> sptr(new X(100));
	EXPECT_NE(sptr, nullptr);
	EXPECT_EQ(sptr.use_count(), 1);
	EXPECT_EQ(sptr->get_n(), 100);

	// Reset the shared_ptr handing it a fresh instance of X
	sptr.reset(new X(200));
	EXPECT_NE(sptr, nullptr);
	EXPECT_EQ(sptr.use_count(), 1);
	EXPECT_EQ(sptr->get_n(), 200);
}

TEST(SharedPtrTest, ResetSharedPtrMultipleOwnership) {
	struct X {
		X(int n_) : n(n_) {}

		int get_n() { return n; }

		~X() = default;
		int n;
	};

	xstd::shared_ptr<X> sptr1(new X(100));
	xstd::shared_ptr sptr2 = sptr1;
	xstd::shared_ptr sptr3 = sptr2;

	EXPECT_EQ(sptr1->get_n(), 100);
	EXPECT_EQ(sptr2->get_n(), 100);
	EXPECT_EQ(sptr3->get_n(), 100);
	EXPECT_EQ(sptr1.use_count(), 3);

	// Reset the shared_ptr sptr1. Hand it a new instance of X.
	// The old instance will stay shared between sptr2 and sptr3.
	sptr1.reset(new X(200));
	EXPECT_EQ(sptr1->get_n(), 200);
	EXPECT_EQ(sptr2->get_n(), 100);
	EXPECT_EQ(sptr3->get_n(), 100);
	EXPECT_EQ(sptr1.use_count(), 1);
	EXPECT_EQ(sptr2.use_count(), 2);
}

TEST(SharedPtrTest, ResetArrayVersion) {
	int *a = new int[3];
	a[0] = 1;
	a[1] = 2;
	a[2] = 3;
	int *b = new int[3];
	b[0] = 4;
	b[1] = 5;
	b[2] = 6;

	xstd::shared_ptr<int[]> sptr1(a);
	xstd::shared_ptr<int[]> sptr2(sptr1);
	xstd::shared_ptr<int[]> sptr3(sptr2);

	EXPECT_EQ(sptr1.use_count(), 3);
	EXPECT_EQ(sptr1[0], 1);
	EXPECT_EQ(sptr1[1], 2);
	EXPECT_EQ(sptr1[2], 3);

	sptr1.reset(b);
	EXPECT_EQ(sptr1.use_count(), 1);
	EXPECT_EQ(sptr2.use_count(), 2);
	EXPECT_EQ(sptr1[0], 4);
	EXPECT_EQ(sptr1[1], 5);
	EXPECT_EQ(sptr1[2], 6);

	EXPECT_EQ(sptr2[0], 1);
	EXPECT_EQ(sptr2[1], 2);
	EXPECT_EQ(sptr2[2], 3);
}

TEST(SharedPtrTest, ParametrizedCTorTestScalarVersion) {
	auto ptr = new int(17);
	xstd::shared_ptr<int> s_ptr{ptr};
	EXPECT_EQ(*s_ptr, 17);
	EXPECT_NE(s_ptr.get(), nullptr);
	EXPECT_EQ(s_ptr.get(), ptr);
}

TEST(SharedPtrTest, ParametrizedCTorTestArrayVersion) {
	auto ptr = new int[10]();
	for (int i{0}; i < 10; ++i) {
		ptr[i] = i + 1;
	}
	xstd::shared_ptr<int[]> s_ptr(ptr);
	EXPECT_NE(s_ptr, nullptr);
	EXPECT_EQ(s_ptr.get(), ptr);
	for (int i{0}; i < 10; ++i) {
		EXPECT_EQ(ptr[i], i + 1);
		EXPECT_EQ(s_ptr[i], ptr[i]);
	}
}

TEST(SharedPtrTest, RefCountingTest) {
	int *raw_ptr = new int(42);
	{
		xstd::shared_ptr<int> ptr1{raw_ptr};
		EXPECT_EQ(ptr1.use_count(), 1);
		EXPECT_EQ(ptr1.get(), raw_ptr);
		{
			xstd::shared_ptr ptr2 = ptr1;
			EXPECT_EQ(ptr1.use_count(), 2);
			EXPECT_EQ(ptr1.get(), raw_ptr);
			{
				xstd::shared_ptr ptr3 = ptr2;
				EXPECT_EQ(ptr1.use_count(), 3);
				EXPECT_EQ(ptr1.get(), raw_ptr);
			}
			EXPECT_EQ(ptr1.use_count(), 2);
			EXPECT_EQ(ptr1.get(), raw_ptr);
		}
		EXPECT_EQ(ptr1.use_count(), 1);
		EXPECT_EQ(ptr1.get(), raw_ptr);
	}
}

TEST(SharedPtrTest, RefCountingTestArrayVersion) {
	int *raw_ptr = new int[5];
	{
		xstd::shared_ptr<int[]> ptr1{raw_ptr};
		EXPECT_EQ(ptr1.use_count(), 1);
		EXPECT_EQ(ptr1.get(), raw_ptr);
		{
			xstd::shared_ptr ptr2 = ptr1;
			EXPECT_EQ(ptr1.use_count(), 2);
			EXPECT_EQ(ptr1.get(), raw_ptr);
			{
				xstd::shared_ptr ptr3 = ptr2;
				EXPECT_EQ(ptr1.use_count(), 3);
				EXPECT_EQ(ptr1.get(), raw_ptr);
			}
			EXPECT_EQ(ptr1.use_count(), 2);
			EXPECT_EQ(ptr1.get(), raw_ptr);
		}
		EXPECT_EQ(ptr1.use_count(), 1);
		EXPECT_EQ(ptr1.get(), raw_ptr);
	}
}

TEST(SharedPtrTest, MultithreadedConstructionAndDestructionTest) {
	using namespace std::chrono_literals;
	xstd::shared_ptr ptr{new int(42)};
	std::atomic<bool> go{false};
	EXPECT_EQ(ptr.use_count() == 1, true);

	std::thread t1([&] {
		xstd::shared_ptr<int> ptr1 = ptr;
		while (!go.load())
			;
		std::cout << "\nRef Count = " << ptr.use_count();
		std::this_thread::sleep_for(1s);
	});

	std::thread t2([&] {
		xstd::shared_ptr<int> ptr2 = ptr;
		while (!go.load())
			;
		std::cout << "\nRef Count = " << ptr.use_count();
		std::this_thread::sleep_for(1s);
	});

	std::this_thread::sleep_for(1s);
	go.store(true);
	t1.join();
	t2.join();
	EXPECT_EQ(ptr.use_count() == 1, true);
}

TEST(SharedPtrTest, CopyConstructorTest) {
	/* Copy constructor */
	int *raw_ptr = new int(42);
	xstd::shared_ptr<int> p1(raw_ptr);

	xstd::shared_ptr<int> p2 = p1;
	EXPECT_EQ(p1.get(), raw_ptr);
	EXPECT_EQ(p2, p1);
	EXPECT_EQ(*p2, 42);
	EXPECT_EQ(p2.get(), raw_ptr);
}

TEST(SharedPtrTest, CopyConstructorTestArrayVersion) {
	/* Copy constructor */
	int *raw_ptr = new int[3];
	raw_ptr[0] = 42;
	raw_ptr[1] = 5;
	raw_ptr[2] = 17;

	xstd::shared_ptr<int[]> p1(raw_ptr);

	xstd::shared_ptr p2{p1};
	EXPECT_EQ(p1.get(), raw_ptr);
	EXPECT_EQ(p2, p1);
	EXPECT_EQ(p2.get(), raw_ptr);

	EXPECT_EQ(p2[0], 42);
	EXPECT_EQ(p2[1], 5);
	EXPECT_EQ(p2[2], 17);
}

TEST(SharedPtrTest, MoveConstructorTest) {
	/* Move constructor*/
	auto raw_ptr = new int(28);
	xstd::shared_ptr<int> p1(raw_ptr);
	xstd::shared_ptr<int> p2 = std::move(p1);
	xstd::shared_ptr<int> p3 = std::move(p2);
	EXPECT_EQ(p1.get(), nullptr);
	EXPECT_EQ(p1.use_count(), 0);
	EXPECT_EQ(p2.get(), nullptr);
	EXPECT_EQ(p2.use_count(), 0);
	EXPECT_NE(p3, nullptr);
	EXPECT_EQ(p3.get(), raw_ptr);
	EXPECT_EQ(p3.use_count(), 1);
	EXPECT_EQ(*p3, 28);
}

TEST(SharedPtrTest, CopyAssignmentTest) {
	/* Copy Assignment */
	xstd::shared_ptr<double> p1(new double(2.71828));
	xstd::shared_ptr<double> p2(new double(3.14159));

	EXPECT_EQ(*p2 == 3.14159, true);
	p2 = p1;
	EXPECT_EQ(p2.get() == p1.get(), true);
	EXPECT_EQ(*p2 == *p1, true);
}

TEST(SharedPtrTest, MoveAssignmentTest) {
	/* Move Assignment */
	xstd::shared_ptr<int> p1(new int(42));
	xstd::shared_ptr<int> p2(new int(28));
	p2 = std::move(p1);
	EXPECT_EQ(p2.get() != nullptr, true);
	EXPECT_EQ(*p2 == 42, true);
}

/* swap() : swap the managed objects */
TEST(SharedPtrTest, SwapTest) {
	int *first = new int(42);
	int *second = new int(17);

	xstd::shared_ptr<int> p1(first);
	xstd::shared_ptr<int> p2(second);

	swap(p1, p2);

	EXPECT_EQ(p2.get() == first && p1.get() == second, true);
	EXPECT_EQ(((*p1) == 17) && ((*p2) == 42), true);
}

// Observers
/* get() : Returns a pointer to the managed object or nullptr*/
TEST(SharedPtrTest, GetTest) {
	double *resource = new double(0.50);
	xstd::shared_ptr p(resource);

	EXPECT_EQ(p.get() == resource, true);
	EXPECT_EQ(*(p.get()) == 0.50, true);
}

// Pointer-like functions
TEST(SharedPtrTest, IndirectionOperatorTest) {
	/* indirection operator* to dereference pointer to managed object,
	   member access operator -> to call member function*/
	struct X {
		int _n;

		X() = default;
		X(int n) : _n{n} {}
		~X() = default;
		int foo() { return _n; }
	};

	xstd::shared_ptr<X> ptr(new X(10));
	EXPECT_EQ((*ptr)._n == 10, true);
	EXPECT_EQ(ptr->foo() == 10, true);
}

// Custom deleter test
TEST(SharedPtrTest, CustomDeleterTest) {
	struct Point2D {
		double x;
		double y;

		Point2D(double x_, double y_) : x{x_}, y{y_} {}

		~Point2D() = default;
	};

	auto custom_deleter = [](Point2D *ptr) {
		std::cout << "\n" << "custom_deleter invoked";
		ptr->~Point2D();        // Call the destructor
		::operator delete(ptr); // Deallocate memory
	};

	{
		xstd::shared_ptr<Point2D> ptr1(new Point2D(3.0, 5.0), custom_deleter);
		EXPECT_EQ(ptr1.use_count(), 1);
		{
			xstd::shared_ptr<Point2D> ptr2 = ptr1;
			EXPECT_EQ(ptr2.use_count(), 2);
		}

		EXPECT_EQ(ptr1.use_count(), 1);
	}
}