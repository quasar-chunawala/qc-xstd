#include <core/containers/construction_transaction.h>
#include <core/containers/soa_split_buffer.h>
#include <core/containers/soa_utils.h>
#include <gtest/gtest.h>
#include <memory>

namespace {
	template <std::size_t Size, std::size_t Align> struct alignas(Align) Blob {
		std::byte bytes[Size];
	};

	struct Vec3 {
		double x_;
		double y_;
		double z_;
	};

	// Members are ordered from largest to smallest
	struct Particle {
		Vec3 position;
		Vec3 velocity;
		float lifetime;
		std::uint8_t color[4];
		std::uint16_t texture_id;
		bool alive;
	};

	struct Mixed {
		Blob<1, 1> a;
		Blob<16, 16> b;
		Blob<3, 1> c;
		Blob<8, 8> d;
		Blob<2, 2> e;
		Blob<4, 4> f;
	};
} // namespace
TEST(SoaSplitBufferUtilsTest, MemberLayoutsOfVec3D) {
	auto layouts = xstd::detail::member_layouts<Vec3>;
	for (auto i{0}; i < 3; ++i) {
		EXPECT_EQ(layouts[i].size, sizeof(double));
		EXPECT_EQ(layouts[i].alignment, alignof(double));
	}
}

TEST(SoaSplitBufferUtilsTest, MemberOffsetsOfParticle) {
	auto layouts = xstd::detail::member_layouts<Particle>;
	EXPECT_EQ(layouts[0].size, sizeof(double) * 3);
	EXPECT_EQ(layouts[1].size, sizeof(double) * 3);
	EXPECT_EQ(layouts[2].size, sizeof(float));
	EXPECT_EQ(layouts[3].size, sizeof(uint8_t) * 4);
	EXPECT_EQ(layouts[4].size, sizeof(uint16_t));
	EXPECT_EQ(layouts[5].size, sizeof(bool));

	// Since the members of Particle are ordered from largest to smallest
	// we don't expect any padding.
	auto member_offsets = xstd::detail::calculate_member_offsets<Particle>(100);
	size_t allocation_offset{0};
	for (auto i{0}; i < 6; ++i) {
		EXPECT_EQ(member_offsets[i].offset, allocation_offset);
		EXPECT_EQ(member_offsets[i].trailing_padding_bytes, 0);
		allocation_offset += layouts[i].size * 100;
	}
}

TEST(SoaSplitBufferUtilsTest, MemberOffsetsOfMixed) {
	// Capacity 7:
	//   a    0 ..   7  -> pad 9 to reach 16 (b is 16-aligned)
	//   b   16 .. 128
	//   c  128 .. 149  -> pad 3 to reach 152 (d is 8-aligned)
	//   d  152 .. 208
	//   e  208 .. 222  -> pad 2 to reach 224 (f is 4-aligned)
	//   f  224 .. 252
	auto member_offsets = xstd::detail::calculate_member_offsets<Mixed>(7);
	EXPECT_EQ(member_offsets[0].alignment, 1);
	EXPECT_EQ(member_offsets[0].size, 1);
	EXPECT_EQ(member_offsets[0].offset, 0);
	EXPECT_EQ(member_offsets[0].trailing_padding_bytes, 9);

	EXPECT_EQ(member_offsets[1].alignment, 16);
	EXPECT_EQ(member_offsets[1].size, 16);
	EXPECT_EQ(member_offsets[1].offset, 16);
	EXPECT_EQ(member_offsets[1].trailing_padding_bytes, 0);

	EXPECT_EQ(member_offsets[2].alignment, 1);
	EXPECT_EQ(member_offsets[2].size, 3);
	EXPECT_EQ(member_offsets[2].offset, 128);
	EXPECT_EQ(member_offsets[2].trailing_padding_bytes, 3);

	EXPECT_EQ(member_offsets[3].alignment, 8);
	EXPECT_EQ(member_offsets[3].size, 8);
	EXPECT_EQ(member_offsets[3].offset, 152);
	EXPECT_EQ(member_offsets[3].trailing_padding_bytes, 0);

	EXPECT_EQ(member_offsets[4].alignment, 2);
	EXPECT_EQ(member_offsets[4].size, 2);
	EXPECT_EQ(member_offsets[4].offset, 208);
	EXPECT_EQ(member_offsets[4].trailing_padding_bytes, 2);

	EXPECT_EQ(member_offsets[5].alignment, 4);
	EXPECT_EQ(member_offsets[5].size, 4);
	EXPECT_EQ(member_offsets[5].offset, 224);
	EXPECT_EQ(member_offsets[5].trailing_padding_bytes, 0);

	EXPECT_EQ(xstd::detail::calculate_allocation_size<Mixed>(7), 252);
}

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
	xstd::detail::soa_split_buffer<Vec3> split_buffer(1000, 250);
	EXPECT_EQ(split_buffer.size(), 0);
	EXPECT_EQ(split_buffer.capacity(), 1000);
}

TEST(SoaSplitBufferTest, ConstructAtEndWithSizeTest) {
	xstd::detail::soa_split_buffer<Vec3> buf(1000, 250);
	EXPECT_EQ(buf.size(), 0);
	EXPECT_EQ(buf.capacity(), 1000);

	Vec3 i_hat{.x_ = 1.0, .y_ = 0.0, .z_ = 0.0};
	buf.copy_construct_at_end_with_size(1, i_hat);
	EXPECT_EQ(buf.size(), 1);
	EXPECT_EQ(buf[0], i_hat);

	Vec3 j_hat{.x_ = 0.0, .y_ = 1.0, .z_ = 0.0};
	buf.copy_construct_at_end_with_size(1, j_hat);
	EXPECT_EQ(buf.size(), 2);
	EXPECT_EQ(buf[1], j_hat);
}