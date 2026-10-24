#include <gtest/gtest.h>

#include <core/containers/forward_list.h>

TEST(ForwardListTest, DefaultConstructorTest) {
	xstd::forward_list<int> lst;
	EXPECT_EQ(lst.empty(), true);
	EXPECT_EQ(lst.size(), 0);
}

TEST(ForwardListTest, InitializerListConstructorTest) {
	xstd::forward_list<int> lst{1, 2, 3, 4, 5};
	EXPECT_EQ(lst.size(), 5);
	EXPECT_EQ(lst.empty(), false);

	auto it = lst.begin();
	EXPECT_EQ(*it, 1);
	++it;
	EXPECT_EQ(*it, 2);
}

TEST(ForwardListTest, RangeConstructorTest) {
	std::vector<int> vec{1, 2, 3, 4, 5};
	xstd::forward_list<int> lst(vec.begin(), vec.end());

	EXPECT_EQ(lst.size(), 5);
	auto it = lst.begin();
	for (int i = 1; i <= 5; ++i, ++it) {
		EXPECT_EQ(*it, i);
	}
}

TEST(ForwardListTest, CopyConstructorTest) {
	xstd::forward_list<int> lst1{1, 2, 3};
	xstd::forward_list<int> lst2(lst1);

	EXPECT_EQ(lst1.size(), lst2.size());
	auto it1 = lst1.begin();
	auto it2 = lst2.begin();
	while (it1 != lst1.end()) {
		EXPECT_EQ(*it1, *it2);
		++it1;
		++it2;
	}
}

TEST(ForwardListTest, MoveConstructorTest) {
	xstd::forward_list<int> lst1{1, 2, 3};
	xstd::forward_list<int> lst2(std::move(lst1));

	EXPECT_EQ(lst1.empty(), true);
	EXPECT_EQ(lst2.size(), 3);
	EXPECT_EQ(*lst2.begin(), 1);
}