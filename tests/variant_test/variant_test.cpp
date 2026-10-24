#include <gtest/gtest.h>

#include <core/tagged_union/variant.h>

// ------------------- Converting constructor ------------------

TEST(VariantConstructor, DefaultConstructsFirstAlternative) {
	xstd::variant<int, float, double> v;
	EXPECT_EQ(xstd::get<0>(v), 0);
}

TEST(VariantConstructor, ExplicitInt) {
	xstd::variant<int, float, double> v(42);
	EXPECT_EQ(xstd::get<int>(v), 42);
}

TEST(VariantConstructor, ExplicitFloat) {
	xstd::variant<int, float, double> v(3.14f);
	EXPECT_EQ(xstd::get<float>(v), 3.14f);
}

TEST(VariantConstructor, ExplicitString) {
	xstd::variant<int, std::string, double> v(std::string("hello"));
	EXPECT_EQ(xstd::get<std::string>(v), "hello");
}

// ------------------- Copy constructor -----------------------

TEST(VariantCopyConstructor, CopiesInt) {
	xstd::variant<int, float, double> a(7);
	xstd::variant<int, float, double> b(a);
	EXPECT_EQ(xstd::get<int>(b), 7);
}

TEST(VariantCopyConstructor, CopiesStringAndIsIndependent) {
	xstd::variant<int, std::string> a(std::string("copy me"));
	xstd::variant<int, std::string> b(a);
	EXPECT_EQ(xstd::get<std::string>(b), "copy me");
	xstd::get<std::string>(b) += " mutated";
	EXPECT_EQ(xstd::get<std::string>(a), "copy me"); // original unchanged
}

// ------------------- Converting assignment ------------------

TEST(VariantConvertingAssignment, AssignsAcrossTypes) {
	xstd::variant<int, std::string, float> v;
	EXPECT_EQ(xstd::get<int>(v), 0);
	v = 42;
	EXPECT_EQ(xstd::get<int>(v), 42);
	v = std::string("hello");
	EXPECT_EQ(xstd::get<std::string>(v), "hello");
	v = 3.14159f;
	EXPECT_EQ(xstd::get<float>(v), 3.14159f);
}

// ------------------- swap -----------------------------------

TEST(VariantSwap, DifferentAlternatives) {
	xstd::variant<int, std::vector<int>> v1{
	    2
    },
	    v2{std::vector{1, 2, 3, 4}};
	EXPECT_EQ(xstd::get<0>(v1), 2);
	EXPECT_EQ(xstd::get<1>(v2), (std::vector<int>{1, 2, 3, 4}));
	v1.swap(v2);
	EXPECT_EQ(xstd::get<1>(v1), (std::vector<int>{1, 2, 3, 4}));
	EXPECT_EQ(xstd::get<0>(v2), 2);
}

TEST(VariantSwap, SameAlternative) {
	xstd::variant<int, std::string> v1{std::string("hello")}, v2{std::string("world")};
	v1.swap(v2);
	EXPECT_EQ(xstd::get<std::string>(v1), "world");
	EXPECT_EQ(xstd::get<std::string>(v2), "hello");
}

// ------------------- Copy assignment ------------------------

TEST(VariantCopyAssignment, SameIndex) {
	xstd::variant<std::string, float> v;
	EXPECT_EQ(xstd::get<std::string>(v), "");
	xstd::variant<std::string, float> w(std::string("hello"));
	v = w;
	EXPECT_EQ(xstd::get<0>(v), "hello");
	EXPECT_EQ(xstd::get<0>(w), "hello"); // source unchanged
}

TEST(VariantCopyAssignment, DifferentIndex) {
	xstd::variant<std::vector<int>, int> v1{
	    std::vector{1, 2, 3, 4, 5}
    },
	    v2{42};
	EXPECT_EQ(v1.index(), 0u);
	EXPECT_EQ(v2.index(), 1u);
	v1 = v2;
	EXPECT_EQ(v1.index(), 1u);
	EXPECT_EQ(xstd::get<1>(v1), 42);
}

// ------------------- Move assignment ------------------------

TEST(VariantMoveAssignment, SameIndex) {
	xstd::variant<std::vector<int>, std::string> v;
	EXPECT_TRUE(xstd::get<0>(v).empty());
	xstd::variant<std::vector<int>, std::string> w{
	    std::vector{1, 2, 3, 4, 5}
    };
	v = std::move(w);
	ASSERT_EQ(xstd::get<0>(v).size(), 5u);
	for (size_t i{0}; i < 5; ++i)
		EXPECT_EQ(xstd::get<0>(v)[i], static_cast<int>(i + 1));
	EXPECT_TRUE(xstd::get<0>(w).empty()); // moved-from is empty
}

TEST(VariantMoveAssignment, DifferentIndex) {
	xstd::variant<std::vector<int>, std::string> v{
	    std::vector{1, 2, 3, 4, 5}
    };
	EXPECT_EQ(v.index(), 0u);
	xstd::variant<std::vector<int>, std::string> w{std::string("hello")};
	EXPECT_EQ(w.index(), 1u);
	v = std::move(w);
	EXPECT_EQ(v.index(), 1u);
	EXPECT_EQ(xstd::get<1>(v), "hello");
	EXPECT_EQ(w.index(), 1u);
	EXPECT_TRUE(xstd::get<1>(w).empty()); // moved-from string is empty
}