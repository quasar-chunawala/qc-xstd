#include <gtest/gtest.h>

#include <core/smart_pointers/unique_ptr.h>

TEST(UniquePtrTest, CreateAndAccessTest) {
	int *raw_ptr = new int(42);
	xstd::unique_ptr<int> p1(raw_ptr);

	EXPECT_EQ(*p1 == 42, true);
	EXPECT_EQ(p1.get(), raw_ptr);

	xstd::unique_ptr<int> p2(new int(17));
	EXPECT_EQ(*p2 == 17, true);
	EXPECT_EQ(p2.get() != nullptr, true);
}

/* Move constructor - Transfer of ownership */
TEST(UniquePtrTest, MoveConstructorTest) {
	xstd::unique_ptr p{xstd::unique_ptr(new int(17))};

	EXPECT_EQ(*p, 17);
	EXPECT_EQ(p != nullptr, true);
}

/* Move assignment */
TEST(UniquePtrTest, MoveAssignmentTest) {
	xstd::unique_ptr<int> p1(new int(42));
	p1 = xstd::unique_ptr<int>(new int(17));

	EXPECT_EQ(p1 != nullptr, true);
	EXPECT_EQ(*p1 == 17, true);
}

// Modifiers
/* release() : Returns the pointer to resource and releases ownership*/
TEST(UniquePtrTest, ReleaseTest) {
	xstd::unique_ptr<double> ptr(new double(3.14));
	double *rawPtr = ptr.release();

	EXPECT_EQ(ptr == nullptr, true);
	EXPECT_EQ(rawPtr != nullptr, true);
	EXPECT_EQ(*rawPtr == 3.14, true);

	delete rawPtr;
	rawPtr = nullptr;
}

/* reset() :  replaces the managed object */
TEST(UniquePtrTest, ResetUniquePtr) {
	xstd::unique_ptr<int> ptr(new int(10));
	ptr.reset(new int(20));
	EXPECT_EQ(ptr != nullptr, true);
	EXPECT_EQ(*ptr == 20, true);

	// Self-reset test
	ptr.reset(ptr.get());
}

/* swap() : swap the managed objects */
TEST(UniquePtrTest, SwapTest) {
	int *first = new int(42);
	int *second = new int(17);

	xstd::unique_ptr<int> p1(first);
	xstd::unique_ptr<int> p2(second);

	swap(p1, p2);

	EXPECT_EQ(p2.get() == first && p1.get() == second, true);
	EXPECT_EQ(((*p1) == 17) && ((*p2) == 42), true);
}

// Observers
/* get() : Returns a pointer to the managed object or nullptr*/
TEST(UniquePtrTest, GetTest) {
	double *resource = new double(0.50);
	xstd::unique_ptr p(resource);

	EXPECT_EQ(p.get() == resource, true);
	EXPECT_EQ(*(p.get()) == 0.50, true);
}

/* operator bool() : Checks whether *this owns an object*/
/*TEST(UniquePtrTest, OperatorBoolTest){
    int* resource = new int(28);
    xstd::unique_ptr<int> p1;
    xstd::unique_ptr<int> p2(resource);

    EXPECT_EQ(p1, false);
    EXPECT_EQ(p2, true);
}*/

// Pointer-like functions
TEST(UniquePtrTest, IndirectionOperatorTest) {
	/* indirection operator* to dereference pointer to managed object,
	   member access operator -> to call member function*/
	struct X {
		int _n;

		X() = default;
		X(int n) : _n{n} {}
		~X() = default;
		int foo() { return _n; }
	};

	xstd::unique_ptr<X> ptr(new X(10));
	EXPECT_EQ((*ptr)._n == 10, true);
	EXPECT_EQ(ptr->foo() == 10, true);
}

TEST(UniquePtrTest, PointerToArrayOfTConstructionAndAccess) {
	/* Constructing unique_ptr<T[]> and access */
	{
		xstd::unique_ptr<int[]> p(new int[5]{1, 2, 3, 4, 5});
		EXPECT_EQ(p != nullptr, true);
		EXPECT_EQ(*p == 1, true);
		EXPECT_EQ(p[2] == 3, true);
		int *raw_ptr = p.release();
		EXPECT_EQ(p == nullptr, true);

		delete[] raw_ptr;
		raw_ptr = nullptr;
	}
}
