#include <gtest/gtest.h>

#include <core/string/string.h>

// ----------------------------------------------------------------
// default constructor
// ----------------------------------------------------------------
TEST(DefaultConstructor, IsEmpty) {
	xstd::string s;
	EXPECT_EQ(s.size(), 0);
	EXPECT_TRUE(s.empty());
	EXPECT_GT(s.capacity(), 0);
}

// ----------------------------------------------------------------
// from literal constructor
// ----------------------------------------------------------------
TEST(FromLiteralConstructor, ShortString) {
	xstd::string s("hello");
	EXPECT_FALSE(s.empty());
	EXPECT_EQ(s.size(), 5);
	EXPECT_GT(s.capacity(), 0);
}

TEST(FromLiteralConstructor, LongString) {
	xstd::string s("C++ is a general-purpose programming language");
	EXPECT_FALSE(s.empty());
	EXPECT_EQ(s.size(), 45);
	EXPECT_GT(s.capacity(), 0);
}

TEST(FromLiteralConstructor, EmbeddedNull) {
	xstd::string s("hello\0world");
	EXPECT_FALSE(s.empty());
	EXPECT_EQ(s.size(), 5);
	EXPECT_GT(s.capacity(), 0);
}

// ----------------------------------------------------------------
// copy constructor
// ----------------------------------------------------------------
TEST(CopyConstructor, ShortString) {
	xstd::string s1("Short");
	xstd::string c1(s1);
	EXPECT_EQ(c1.size(), 5);
}

TEST(CopyConstructor, LongString) {
	xstd::string s2("Long string for copying over the heap");
	xstd::string c2(s2);
	EXPECT_EQ(c2.size(), s2.size());
}

TEST(CopyConstructor, EmbeddedNull) {
	char data[] = {'a', '\0', 'b'};
	xstd::string s3(data, data + 3);
	xstd::string c3(s3);
	EXPECT_EQ(c3.size(), 3);
	EXPECT_EQ(c3[1], '\0');
}

// ----------------------------------------------------------------
// copy assignment
// ----------------------------------------------------------------
TEST(CopyAssignment, SSOToSSO) {
	xstd::string s1("A");
	xstd::string s2("B");
	s1 = s2;
	EXPECT_EQ(s1.size(), 1);
	EXPECT_EQ(s1[0], 'B');
	EXPECT_NE(s1.data(), s2.data());
}

TEST(CopyAssignment, LongToLong) {
	xstd::string s1("This is a very long string that currently lives on the heap.");
	xstd::string s2("And this is another long string, also on the heap, "
	                "but different.");
	const char *old_ptr = s1.data();
	s1 = s2;
	EXPECT_EQ(s1.size(), s2.size());
	EXPECT_EQ(std::strcmp(s1.data(), s2.data()), 0);
	EXPECT_NE(s1.data(), old_ptr);
}

TEST(CopyAssignment, LongToShort) {
	xstd::string s1("Long strings take up heap memory.");
	xstd::string s2("Short");
	s1 = s2;
	EXPECT_EQ(s1.size(), 5);
}

TEST(CopyAssignment, ShortToLong) {
	xstd::string s1("Short");
	xstd::string s2("Long strings take up heap memory.");
	s1 = s2;
	EXPECT_EQ(s1.size(), s2.size());
}

TEST(CopyAssignment, EmbeddedNull) {
	char data[] = {'x', '\0', 'y', 'z'};
	xstd::string s1(data, data + 4);
	xstd::string s2("Placeholder");
	s2 = s1;
	EXPECT_EQ(s2.size(), 4);
	EXPECT_EQ(s2[1], '\0');
	EXPECT_EQ(s2[2], 'y');
}

TEST(CopyAssignment, SelfAssignment) {
	xstd::string s1("Self Assignment Test");
	xstd::string *ptr = &s1;
	s1 = *ptr;
	EXPECT_EQ(s1.size(), 20);
	EXPECT_EQ(std::strcmp(s1.data(), "Self Assignment Test"), 0);
}

// ----------------------------------------------------------------
// move constructor
// ----------------------------------------------------------------
TEST(MoveConstructor, ShortSSO) {
	xstd::string source("Short SSO");
	const char *original_data_addr = source.data();
	xstd::string destination(std::move(source));
	EXPECT_EQ(destination.size(), 9);
	EXPECT_EQ(std::strcmp(destination.data(), "Short SSO"), 0);
	EXPECT_NE(destination.data(), original_data_addr);
	EXPECT_EQ(source.size(), 0);
}

TEST(MoveConstructor, LongHeap) {
	xstd::string source("This is a very long string that lives on the heap.");
	size_t original_size = source.size();
	const char *original_heap_ptr = source.data();
	xstd::string destination(std::move(source));
	EXPECT_EQ(destination.size(), original_size);
	EXPECT_EQ(destination.data(), original_heap_ptr);
	EXPECT_TRUE(source.data() == nullptr || source.size() == 0);
}

TEST(MoveConstructor, EmbeddedNull) {
	char null_data[] = {'m', '\0', 'v', 'e'};
	xstd::string source(null_data, null_data + 4);
	xstd::string destination(std::move(source));
	EXPECT_EQ(destination.size(), 4);
	EXPECT_EQ(destination[1], '\0');
	EXPECT_EQ(std::memcmp(destination.data(), null_data, 4), 0);
}

// ----------------------------------------------------------------
// move assignment
// ----------------------------------------------------------------
TEST(MoveAssignment, SSOToSSO) {
	xstd::string s1("Original");
	xstd::string s2("New");
	s1 = std::move(s2);
	EXPECT_EQ(s1.size(), 3);
	EXPECT_EQ(std::strcmp(s1.data(), "New"), 0);
	EXPECT_EQ(s2.size(), 0);
}

TEST(MoveAssignment, LongToLong) {
	xstd::string s1("This is a very long string currently occupying the heap memory.");
	xstd::string s2("Another extremely long string that we will move into "
	                "the first one.");
	const char *s2_heap_ptr = s2.data();
	size_t s2_size = s2.size();
	s1 = std::move(s2);
	EXPECT_EQ(s1.size(), s2_size);
	EXPECT_EQ(s1.data(), s2_heap_ptr);
	EXPECT_TRUE(s2.data() == nullptr || s2.size() == 0);
}

TEST(MoveAssignment, LongToShort) {
	xstd::string s1("A long string that will be overwritten by a short moved string.");
	xstd::string s2("Short");
	s1 = std::move(s2);
	EXPECT_EQ(s1.size(), 5);
}

TEST(MoveAssignment, EmbeddedNull) {
	char data[] = {'m', '\0', 'v', 'e'};
	xstd::string s1(data, data + 4);
	xstd::string s2("Temporary");
	s2 = std::move(s1);
	EXPECT_EQ(s2.size(), 4);
	EXPECT_EQ(s2[1], '\0');
}

TEST(MoveAssignment, SelfMove) {
	xstd::string s1("Don't break me");
	xstd::string *ptr = &s1;
	s1 = std::move(*ptr);
	EXPECT_EQ(s1.size(), 14);
	EXPECT_EQ(std::strcmp(s1.data(), "Don't break me"), 0);
}

// ----------------------------------------------------------------
// reserve
// ----------------------------------------------------------------
TEST(Reserve, ShortNoOp) {
	xstd::string s("A");
	s.reserve(10);
	EXPECT_EQ(s.capacity(), 22);
}

TEST(Reserve, LongGrowth) {
	xstd::string s("A");
	s.reserve(50);
	EXPECT_GE(s.capacity(), 50);
	EXPECT_FALSE(s.empty());
	EXPECT_EQ(s.size(), 1);
	EXPECT_EQ(s[0], 'A');
}

TEST(Reserve, EmbeddedNull) {
	char data[] = {'x', '\0', 'y'};
	xstd::string s(data, data + 3);
	s.reserve(100);
	EXPECT_EQ(s.size(), 3);
	EXPECT_EQ(s[1], '\0');
}

// ----------------------------------------------------------------
// push_back
// ----------------------------------------------------------------
TEST(PushBack, ShortSSO) {
	xstd::string s;
	s.push_back('A');
	s.push_back('B');
	s.push_back('C');
	EXPECT_EQ(s.size(), 3);
	EXPECT_EQ(s[0], 'A');
	EXPECT_EQ(s[2], 'C');
	EXPECT_EQ(s.data()[3], '\0');
}

TEST(PushBack, SSOToHeapTransition) {
	xstd::string s;
	for (int i = 0; i < 23; ++i)
		s.push_back('x');
	EXPECT_EQ(s.size(), 23);
	s.push_back('!');
	EXPECT_EQ(s.size(), 24);
	EXPECT_GT(s.capacity(), 23);
	EXPECT_EQ(s[0], 'x');
	EXPECT_EQ(s[23], '!');
	EXPECT_EQ(s.data()[24], '\0');
}

TEST(PushBack, EmbeddedNull) {
	xstd::string s("Start");
	s.push_back('\0');
	s.push_back('Z');
	EXPECT_EQ(s.size(), 7);
	EXPECT_EQ(s[5], '\0');
	EXPECT_EQ(s[6], 'Z');
	EXPECT_EQ(s.data()[7], '\0');
}

// ----------------------------------------------------------------
// pop_back
// ----------------------------------------------------------------
TEST(PopBack, ShortSSO) {
	xstd::string s("ABC");
	s.pop_back();
	EXPECT_EQ(s.size(), 2);
	EXPECT_EQ(s[0], 'A');
	EXPECT_EQ(s[1], 'B');
	EXPECT_EQ(s.data()[2], '\0');
}

TEST(PopBack, LongHeap) {
	xstd::string s("123456789012345678901234");
	size_t original_size = s.size();
	size_t original_capacity = s.capacity();
	for (size_t i{1}; i <= 23; ++i) {
		s.pop_back();
		EXPECT_EQ(s.size(), original_size - i);
		EXPECT_EQ(s.capacity(), original_capacity);
		EXPECT_EQ(s.data()[s.size()], '\0');
	}
}

TEST(PopBack, EmbeddedNull) {
	char null_data[] = {'a', 'b', '\0', 'c'};
	xstd::string s(null_data, null_data + 4);
	s.pop_back();
	EXPECT_EQ(s.size(), 3);
	EXPECT_EQ(s.back(), '\0');
	EXPECT_EQ(s.data()[3], '\0');
}

// ----------------------------------------------------------------
// insert
// ----------------------------------------------------------------
TEST(Insert, ShortSSOExternalRange) {
	xstd::string s("AC");
	char b = 'B';
	s.insert(s.begin() + 1, &b, &b + 1);
	EXPECT_EQ(s.size(), 3);
	EXPECT_EQ(std::strcmp(s.data(), "ABC"), 0);
}

TEST(Insert, LongHeapExternalRange) {
	xstd::string s("This is a long string.");
	const char *extra = " Indeed!";
	s.insert(s.end() - 1, extra, extra + std::strlen(extra));
	EXPECT_EQ(s.size(), 22 + 8);
	EXPECT_TRUE(std::strstr(s.data(), "Indeed!."));
}

TEST(Insert, EmbeddedNull) {
	xstd::string s("a");
	char nulls[] = {'\0', 'b'};
	s.insert(s.end(), nulls, nulls + 2);
	EXPECT_EQ(s.size(), 3);
	EXPECT_EQ(s[1], '\0');
	EXPECT_EQ(s[2], 'b');
}

TEST(Insert, SSOToHeapTransition) {
	xstd::string s("Small");
	xstd::string long_val(50, 'z');
	s.insert(s.begin() + 1, long_val.begin(), long_val.end());
	EXPECT_EQ(s.size(), 55);
	EXPECT_EQ(s[0], 'S');
	for (size_t i{1}; i <= 50; ++i)
		EXPECT_EQ(s[i], 'z');
	EXPECT_EQ(s[51], 'm');
	EXPECT_EQ(s[52], 'a');
	EXPECT_EQ(s[53], 'l');
	EXPECT_EQ(s[54], 'l');
	EXPECT_EQ(s[55], '\0');
}

TEST(Insert, SelfReferential) {
	xstd::string s("Hello");
	s.insert(s.begin(), s.begin(), s.end());
	EXPECT_EQ(s.size(), 10);
	EXPECT_EQ(std::strcmp(s.data(), "HelloHello"), 0);
}

TEST(Insert, SelfReferentialWithReallocation) {
	xstd::string s("Trigger");
	xstd::string padding(20, '!');
	s.insert(s.end(), padding.begin(), padding.end());
	s.insert(s.begin(), s.begin(), s.end());
	EXPECT_EQ(s.size(), 54);
}

// ----------------------------------------------------------------
// erase
// ----------------------------------------------------------------
TEST(Erase, ShortSSOMiddle) {
	xstd::string s("ABXCD");
	auto it = s.erase(s.begin() + 2, s.begin() + 3);
	EXPECT_EQ(s.size(), 4);
	EXPECT_EQ(std::strcmp(s.data(), "ABCD"), 0);
	EXPECT_EQ(*it, 'C');
	EXPECT_EQ(s.data()[4], '\0');
}

TEST(Erase, LongHeapEraseToEnd) {
	xstd::string s("This is a long string that we will truncate.");
	s.erase(s.begin() + 26, s.end());
	EXPECT_EQ(s.size(), 26);
	EXPECT_EQ(std::strcmp(s.data(), "This is a long string that"), 0);
	EXPECT_EQ(s.data()[26], '\0');
}

TEST(Erase, EmbeddedNull) {
	char data[] = {'a', 'b', '\0', 'c', 'd'};
	xstd::string s(data, data + 5);
	s.erase(s.begin() + 2, s.begin() + 3);
	EXPECT_EQ(s.size(), 4);
	EXPECT_EQ(s[2], 'c');
	EXPECT_EQ(std::memcmp(s.data(), "abcd", 4), 0);
}

TEST(Erase, FullErase) {
	xstd::string s("Clear me");
	s.erase(s.begin(), s.end());
	EXPECT_EQ(s.size(), 0);
	EXPECT_TRUE(s.empty());
	EXPECT_EQ(s.data()[0], '\0');
}

// ----------------------------------------------------------------
// operator[]
// ----------------------------------------------------------------
TEST(ElementAccess, ShortSSO) {
	xstd::string s("Short");
	EXPECT_EQ(s[0], 'S');
	EXPECT_EQ(s[4], 't');
	s[0] = 's';
	EXPECT_EQ(s[0], 's');
}

TEST(ElementAccess, LongHeap) {
	xstd::string s("This is a very long string for indexing.");
	EXPECT_EQ(s[0], 'T');
	EXPECT_EQ(s[10], 'v');
	s[10] = 'V';
	EXPECT_EQ(s[10], 'V');
}

TEST(ElementAccess, EmbeddedNull) {
	char data[] = {'\0', 'x', '\0', 'y'};
	xstd::string s(data, data + 4);
	EXPECT_EQ(s[0], '\0');
	EXPECT_EQ(s[1], 'x');
	EXPECT_EQ(s[2], '\0');
}

// ----------------------------------------------------------------
// front and back
// ----------------------------------------------------------------
TEST(FrontAndBack, ShortSSO) {
	xstd::string s("Hi");
	EXPECT_EQ(s.front(), 'H');
	EXPECT_EQ(s.back(), 'i');
}

TEST(FrontAndBack, LongHeap) {
	xstd::string s("Long strings need front/back too");
	EXPECT_EQ(s.front(), 'L');
	EXPECT_EQ(s.back(), 'o');
}

TEST(FrontAndBack, EmbeddedNull) {
	char data[] = {'\0', 'a', '\0'};
	xstd::string s(data, data + 3);
	EXPECT_EQ(s.front(), '\0');
	EXPECT_EQ(s.back(), '\0');
}

// ----------------------------------------------------------------
// data()
// ----------------------------------------------------------------
TEST(DataFunction, ShortSSO) {
	xstd::string s("SSO");
	const char *ptr = s.data();
	EXPECT_EQ(ptr[0], 'S');
	EXPECT_EQ(ptr[3], '\0');
}

TEST(DataFunction, LongHeap) {
	xstd::string s("A long string to check pointer consistency");
	const char *ptr = s.data();
	EXPECT_EQ(ptr[0], 'A');
	EXPECT_NE(ptr, reinterpret_cast<const char *>(&s));
}

TEST(DataFunction, EmbeddedNull) {
	char data[] = {'\0', 'A'};
	xstd::string s(data, data + 2);
	EXPECT_EQ(s.data()[0], '\0');
	EXPECT_EQ(s.data()[1], 'A');
}

// ----------------------------------------------------------------
// size()
// ----------------------------------------------------------------
TEST(SizeFunction, ZeroSize) {
	xstd::string s("");
	EXPECT_EQ(s.size(), 0);
}

TEST(SizeFunction, MaxSSOSize) {
	xstd::string s("1234567890123456789012");
	EXPECT_EQ(s.size(), 22);
	EXPECT_EQ(s.capacity(), 22);
}

TEST(SizeFunction, SSOToHeapTransition) {
	xstd::string s("1234567890123456789012");
	s.push_back('3');
	EXPECT_EQ(s.size(), 23);
	EXPECT_GE(s.capacity(), 24);
}

TEST(SizeFunction, EmbeddedNull) {
	char data[] = {'H', 'i', '\0', '!', '\0'};
	xstd::string s(data, data + 5);
	EXPECT_EQ(s.size(), 5);
}

// ----------------------------------------------------------------
// capacity()
// ----------------------------------------------------------------
TEST(CapacityFunction, SSOCapacity) {
	xstd::string s("Short");
	EXPECT_EQ(s.capacity(), 22);
}

TEST(CapacityFunction, LongCapacity) {
	xstd::string s("This string is long enough to be on the heap.");
	EXPECT_GE(s.capacity(), s.size());
}

TEST(CapacityFunction, GrowthOnPushBack) {
	xstd::string s("12345678901234567890123");
	size_t cap_before = s.capacity();
	s.push_back('!');
	EXPECT_GT(s.capacity(), cap_before);
}

// ----------------------------------------------------------------
// empty()
// ----------------------------------------------------------------
TEST(EmptyFunction, TrulyEmpty) {
	xstd::string s;
	EXPECT_TRUE(s.empty());
}

TEST(EmptyFunction, NotEmptySSO) {
	xstd::string s("a");
	EXPECT_FALSE(s.empty());
}

TEST(EmptyFunction, NotEmptyLong) {
	xstd::string s("This is a very long string");
	EXPECT_FALSE(s.empty());
}

TEST(EmptyFunction, SingleEmbeddedNull) {
	char null_char = '\0';
	xstd::string s(&null_char, &null_char + 1);
	EXPECT_EQ(s.size(), 1);
	EXPECT_FALSE(s.empty());
}
