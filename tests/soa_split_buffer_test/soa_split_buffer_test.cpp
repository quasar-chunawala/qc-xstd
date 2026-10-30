#include <core/containers/construction_transaction.h>
#include <core/containers/soa_split_buffer.h>
#include <gtest/gtest.h>
#include <memory>

TEST(SoaSplitBufferTest, DefaultConstructor) {
	struct Point2D {
		double x_;
		double y_;
	};
	xstd::detail::soa_split_buffer<Point2D> buffer;
	EXPECT_EQ(buffer.size(), 0);
	EXPECT_EQ(buffer.capacity(), 0);
}

TEST(SoaSplitBufferTest, CapacityAndBeginOffsetConstructor) {
	struct Vec3D {
		double x_;
		double y_;
		double z_;
	};
	xstd::detail::soa_split_buffer<Vec3D> split_buffer(1000, 250);
	EXPECT_EQ(split_buffer.size(), 0);
	EXPECT_EQ(split_buffer.capacity(), 1000);
}