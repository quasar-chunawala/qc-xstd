#include <gtest/gtest.h>

#include <core/containers/vector.h>

struct copycounter {
	static inline std::size_t copycount{0};

	copycounter() = default;
	copycounter(const copycounter &) { ++copycount; }
	copycounter &operator=(const copycounter &) = default;
	copycounter(copycounter &&) = default;
	copycounter &operator=(copycounter &&) = default;
	~copycounter() = default;
};

struct ThrowOnValueConstruct {
	static inline int default_construct_count_{0};
	static inline int throw_after_{0};
	static inline int deallocation_count{0};

	ThrowOnValueConstruct() {
		if (default_construct_count_ >= throw_after_ && throw_after_ > 0)
			throw std::runtime_error("Exception in default ctor");

		++default_construct_count_;
	}

	static void reset() {
		default_construct_count_ = 0;
		throw_after_ = 0;
		deallocation_count = 0;
	}

	~ThrowOnValueConstruct() { ++deallocation_count; }
};

struct ThrowOnCopyConstruct {
	static inline int default_construct_count_{0};
	static inline int copy_construct_count_{0};
	static inline int throw_after_{0};
	static inline int deallocation_count{0};

	ThrowOnCopyConstruct() { ++default_construct_count_; }

	ThrowOnCopyConstruct(const ThrowOnCopyConstruct &other) {
		if (copy_construct_count_ >= throw_after_ && throw_after_ > 0)
			throw std::runtime_error("Exception in copy ctor");

		++copy_construct_count_;
	}

	static void reset() {
		default_construct_count_ = 0;
		copy_construct_count_ = 0;
		throw_after_ = 0;
		deallocation_count = 0;
	}

	~ThrowOnCopyConstruct() { ++deallocation_count; }
};

TEST(VectorTest, DefaultConstructorTest) {
	xstd::vector<int> v;
	EXPECT_EQ(v.empty(), true);
}

TEST(VectorTest, ThrowDuringDefaultConstructionTest) {
	ThrowOnValueConstruct::reset();
	EXPECT_EQ(ThrowOnValueConstruct::default_construct_count_, 0);
	EXPECT_EQ(ThrowOnValueConstruct::deallocation_count, 0);
	ThrowOnValueConstruct::throw_after_ = 2;

	EXPECT_THROW({ xstd::vector<ThrowOnValueConstruct> v(5); }, std::runtime_error);

	EXPECT_EQ(ThrowOnValueConstruct::default_construct_count_, 2);
	EXPECT_EQ(ThrowOnValueConstruct::deallocation_count, 2);
}

TEST(VectorTest, InitializerListTest) {
	xstd::vector<int> v{1, 2, 3, 4, 5};
	EXPECT_EQ(!v.empty(), true);
	EXPECT_EQ(v.size(), 5);
	EXPECT_TRUE(v.capacity() > 0);
	for (size_t i{0}; i < v.size(); ++i) {
		EXPECT_EQ(v.at(i), i + 1);
	}
}

TEST(VectorTest, ThrowDuringInitializerListCtorTest) {
	ThrowOnCopyConstruct::reset();
	ThrowOnCopyConstruct::throw_after_ = 2;

	EXPECT_THROW(
	    {
		    xstd::vector<ThrowOnCopyConstruct> v({ThrowOnCopyConstruct(),
		                                          ThrowOnCopyConstruct(),
		                                          ThrowOnCopyConstruct(),
		                                          ThrowOnCopyConstruct(),
		                                          ThrowOnCopyConstruct()});
	    },
	    std::runtime_error);

	EXPECT_EQ(ThrowOnCopyConstruct::default_construct_count_, 5);
	EXPECT_EQ(ThrowOnCopyConstruct::copy_construct_count_, 2);
	EXPECT_EQ(ThrowOnCopyConstruct::deallocation_count, 7);
}

TEST(VectorTest, ParameterizedConstructorTest) {
	xstd::vector v(10, 5.5);
	EXPECT_EQ(v.size(), 10);
	for (size_t i{0}; i < v.size(); ++i) {
		EXPECT_EQ(v[i], 5.5);
	}
}

TEST(VectorTest, ThrowDuringFillDataTest) {
	ThrowOnCopyConstruct::reset();
	ThrowOnCopyConstruct::throw_after_ = 2;

	EXPECT_THROW({ xstd::vector<ThrowOnCopyConstruct> v(10, ThrowOnCopyConstruct()); }, std::runtime_error);

	EXPECT_EQ(ThrowOnCopyConstruct::default_construct_count_, 1);
	EXPECT_EQ(ThrowOnCopyConstruct::copy_construct_count_, 2);
	EXPECT_EQ(ThrowOnCopyConstruct::deallocation_count, 3);
}

TEST(VectorTest, CopyConstructorTest) {
	xstd::vector v1{1.0, 2.0, 3.0, 4.0, 5.0};
	xstd::vector v2(v1);

	EXPECT_EQ(v1.size() == v2.size(), true);

	for (int i{0}; i < v1.size(); ++i) {
		EXPECT_EQ(v2[i], i + 1);
		EXPECT_EQ(v1[i], v2[i]);
	}
}

TEST(VectorTest, ThrowDuringCopyConstructionTest) {
	ThrowOnCopyConstruct::reset();
	EXPECT_EQ(ThrowOnCopyConstruct::copy_construct_count_, 0);
	EXPECT_EQ(ThrowOnCopyConstruct::deallocation_count, 0);
	ThrowOnCopyConstruct::throw_after_ = 2;

	xstd::vector<ThrowOnCopyConstruct> v1(5);

	EXPECT_THROW({ xstd::vector<ThrowOnCopyConstruct> v2(v1); }, std::runtime_error);

	EXPECT_EQ(ThrowOnCopyConstruct::copy_construct_count_, 2);
	EXPECT_EQ(ThrowOnCopyConstruct::deallocation_count, 2);
}

TEST(VectorTest, MoveConstructorTest) {
	xstd::vector<int> v1{1, 2, 3};
	xstd::vector<int> v2(std::move(v1));
	EXPECT_EQ(v1.size(), 0);
	EXPECT_EQ(v1.capacity(), 0);
	EXPECT_EQ(v2.size(), 3);
	for (size_t i{0}; i < v2.size(); ++i)
		EXPECT_EQ(v2[i], i + 1);
}

TEST(VectorTest, CopyAssignmentTest) {
	xstd::vector<int> v1{1, 2, 3};
	xstd::vector<int> v2;
	v2 = v1;

	EXPECT_EQ(v1.size(), v2.size());
	EXPECT_EQ(v1.capacity(), v2.capacity());
	for (int i = 0; i < v1.size(); ++i) {
		EXPECT_EQ(v1[i], i + 1);
		EXPECT_EQ(v1[i], v2[i]);
	}
}

TEST(VectorTest, MoveAssignmentTest) {
	xstd::vector<int> v1{1, 2, 3};
	xstd::vector<int> v2;
	v2 = std::move(v1);

	EXPECT_EQ(v1.size(), 0);
	EXPECT_EQ(v1.capacity(), 0);
	EXPECT_EQ(v2.size(), 3);
	for (int i = 0; i < v1.size(); ++i) {
		EXPECT_EQ(v2[i], i + 1);
	}
}

TEST(VectorTest, AtTest) {
	xstd::vector<int> v{1, 2, 3};
	EXPECT_EQ(v.at(0), 1);
	EXPECT_EQ(v.at(1), 2);
	EXPECT_EQ(v.at(2), 3);

	EXPECT_THROW(v.at(3), std::out_of_range);
}

TEST(VectorTest, SubscriptOperatorTest) {
	xstd::vector<int> v{1, 2, 3};
	for (int i{0}; i < v.size(); ++i) {
		EXPECT_EQ(v[i], i + 1);
	}
}

TEST(VectorTest, FrontAndBackTest) {
	xstd::vector<int> v{1, 2, 3};
	EXPECT_EQ(v.front(), 1);
	EXPECT_EQ(v.back(), 3);
}

TEST(VectorTest, EmptyTest) {
	xstd::vector<int> v;
	EXPECT_EQ(v.empty(), true);

	v.push_back(42);
	EXPECT_EQ(v.empty(), false);
}

TEST(VectorTest, SizeAndCapacityTest) {
	xstd::vector<int> v;
	EXPECT_EQ(v.size(), 0);
	EXPECT_GE(v.capacity(), 0);

	v.push_back(42);
	EXPECT_EQ(v.size(), 1);
	EXPECT_GT(v.capacity(), 0);

	v.push_back(v.back());
	EXPECT_EQ(v.size(), 2);
	EXPECT_EQ(v[1], 42);
}

TEST(VectorTest, ReserveTest) {
	xstd::vector<int> v1;
	v1.reserve(10);
	EXPECT_GE(v1.capacity(), 10);
	EXPECT_EQ(v1.size(), 0);

	xstd::vector<int> v2{1, 2, 3, 4, 5, 6, 7};
	size_t old_capacity = v2.capacity();
	EXPECT_GE(v2.capacity(), 7);
	EXPECT_EQ(v2.size(), 7);
	size_t new_capacity = 2 * old_capacity;
	v2.reserve(new_capacity);
	EXPECT_GE(v2.capacity(), new_capacity);
	EXPECT_EQ(v2.size(), 7);
	for (size_t i{0}; i < v2.size(); ++i)
		EXPECT_EQ(v2[i], i + 1);
}

TEST(VectorTest, ResizeTest) {
	xstd::vector<int> v{1, 2, 3};
	v.resize(5);

	EXPECT_EQ(v.size(), 5);
	for (size_t i{0}; i < 3; ++i)
		EXPECT_EQ(v[i], i + 1);

	EXPECT_EQ(v[3], 0);
	EXPECT_EQ(v[4], 0);

	v.resize(2);
	EXPECT_EQ(v.size(), 2);
	EXPECT_EQ(v[0], 1);
	EXPECT_EQ(v[1], 2);
}

TEST(VectorTest, PushBackTest) {
	xstd::vector<int> v;
	v.push_back(1);
	v.push_back(2);
	v.push_back(3);

	EXPECT_EQ(v.size(), 3);
	for (size_t i{0}; i < v.size(); ++i)
		EXPECT_EQ(v[i], i + 1);
}

TEST(VectorTest, PushBackSelfReferenceTest) {
	// The design of push_back/insert is slightly hard to get right.
	// If the vector is full, then you reallocate(grow) the vector.
	// If the value to be added is a reference to an existing
	// vector element, then value in vec.push_back(value) may become
	// a dangling reference, if it refers to the old storage (an element of
	// the vector itself e.g. vec.back()). This test is meant for such an
	// edge case.
	xstd::vector<int> vec{1};
	for (size_t i{0}; i < 64; ++i) {
		vec.push_back(vec.back());
		EXPECT_EQ(vec.back(), 1);
	}
}

TEST(VectorTest, EmplaceBackTest) {
	struct Point {
		int x, y;
		Point(int a, int b) : x(a), y(b) {}
	};

	xstd::vector<Point> v;
	v.emplace_back(1, 2);
	v.emplace_back(3, 4);

	EXPECT_EQ(v.size(), 2);
	EXPECT_EQ(v[0].x, 1);
	EXPECT_EQ(v[0].y, 2);
	EXPECT_EQ(v[1].x, 3);
	EXPECT_EQ(v[1].y, 4);
}

TEST(VectorTest, PopBackTest) {
	xstd::vector<int> v = {1, 2, 3};
	EXPECT_EQ(v.size(), 3);
	v.pop_back();
	EXPECT_EQ(v.size(), 2);
	EXPECT_EQ(v, xstd::vector<int>({1, 2}));
}

TEST(VectorTest, InsertAtBeginning) {
	xstd::vector<int> v = {1, 2, 3};
	xstd::vector<int> src = {10, 20};

	auto it = v.insert(v.begin(), src.begin(), src.end());

	EXPECT_EQ(v.size(), 5);
	EXPECT_EQ(it, v.begin());
	xstd::vector<int> result{10, 20, 1, 2, 3};
	for (size_t i{0}; i < result.size(); ++i)
		EXPECT_EQ(v.at(i), result.at(i));
}

TEST(VectorTest, InsertAtEnd) {
	xstd::vector<int> v = {1, 2, 3};
	xstd::vector<int> src = {10, 20};

	auto it = v.insert(v.end(), src.begin(), src.end());

	EXPECT_EQ(v.size(), 5);
	EXPECT_EQ(v, (xstd::vector<int>{1, 2, 3, 10, 20}));
}

TEST(VectorTest, InsertInMiddle) {
	xstd::vector<int> v = {1, 2, 3, 4};
	xstd::vector<int> src = {10, 20};

	auto it = v.insert(v.begin() + 2, src.begin(), src.end());

	EXPECT_EQ(v.size(), 6);
	EXPECT_EQ(it, v.begin() + 2);
	EXPECT_EQ(v, (xstd::vector<int>{1, 2, 10, 20, 3, 4}));
}

TEST(VectorTest, InsertEmptyRange) {
	xstd::vector<int> v = {1, 2, 3};
	xstd::vector<int> src;

	auto it = v.insert(v.begin() + 1, src.begin(), src.end());

	EXPECT_EQ(v.size(), 3);
	EXPECT_EQ(v, (xstd::vector<int>{1, 2, 3}));
}

TEST(VectorTest, InsertIntoEmptyVector) {
	xstd::vector<int> v;
	xstd::vector<int> src = {1, 2, 3};

	auto it = v.insert(v.begin(), src.begin(), src.end());

	EXPECT_EQ(v.size(), 3);
	EXPECT_EQ(it, v.begin());
	EXPECT_EQ(v, (xstd::vector<int>{1, 2, 3}));
}

TEST(VectorTest, InsertWithinCapacity) {
	xstd::vector<int> v;
	v.reserve(10);
	EXPECT_GE(v.capacity(), 10);
	v.push_back(1);
	v.push_back(2);
	v.push_back(3);

	xstd::vector<int> src = {10, 20};
	v.insert(v.begin() + 1, src.begin(), src.end());

	EXPECT_EQ(v, (xstd::vector<int>{1, 10, 20, 2, 3}));
	EXPECT_GE(v.capacity(), 10);
}

TEST(VectorTest, InsertRequiresReallocation) {
	xstd::vector<int> v = {1, 2, 3};
	size_t old_cap = v.capacity();

	xstd::vector<int> src(100, 99);
	v.insert(v.begin() + 1, src.begin(), src.end());

	EXPECT_GT(v.capacity(), old_cap);
	EXPECT_EQ(v.size(), 103);
	EXPECT_EQ(v[0], 1);
	for (size_t i{1}; i < 101; ++i)
		EXPECT_EQ(v[i], 99);

	EXPECT_EQ(v[101], 2);
	EXPECT_EQ(v[102], 3);
}

TEST(VectorTest, SelfInsertFromBeginning) {
	xstd::vector<int> v = {1, 2, 3, 4, 5};

	// Insert first 3 elements at position 2
	v.insert(v.begin() + 2, v.begin(), v.begin() + 3);

	EXPECT_EQ(v, (xstd::vector<int>{1, 2, 1, 2, 3, 3, 4, 5}));
}

TEST(VectorTest, SelfInsertFromMiddle) {
	xstd::vector<int> v = {1, 2, 3, 4, 5};

	// Insert middle elements at beginning
	v.insert(v.begin(), v.begin() + 1, v.begin() + 4);

	EXPECT_EQ(v.size(), 8);
	EXPECT_EQ(v, (xstd::vector<int>{2, 3, 4, 1, 2, 3, 4, 5}));
}

TEST(VectorTest, SelfInsertFromEnd) {
	xstd::vector<int> v = {1, 2, 3, 4, 5};

	// Insert last 2 elements in middle
	v.insert(v.begin() + 2, v.end() - 2, v.end());

	EXPECT_EQ(v, (xstd::vector<int>{1, 2, 4, 5, 3, 4, 5}));
}

TEST(VectorTest, SelfInsertOverlappingRanges) {
	xstd::vector<int> v = {1, 2, 3, 4, 5};

	// Insert range that overlaps with insertion point
	v.insert(v.begin() + 2, v.begin() + 1, v.begin() + 4);

	EXPECT_EQ(v, (xstd::vector<int>{1, 2, 2, 3, 4, 3, 4, 5}));
}
