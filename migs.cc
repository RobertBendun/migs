#include <iostream>
#include <array>
#include <fstream>
#include <cassert>
#include <algorithm>
#include <cmath>

namespace migs
{
	inline namespace sized_ints
	{
		using u8 = std::uint8_t;
		using u32 = std::uint32_t;
	}


	inline namespace int_types
	{
		using uint = unsigned int;
		constexpr uint Uint_Null = uint(-1);
	}

	struct [[gnu::packed]] RGB
	{
		union
		{
			struct { u8 r, g, b; };
			std::array<u8, 3> rgb;
		};

		constexpr RGB() : rgb{} {}
		constexpr RGB(u8 r, u8 g, u8 b) : rgb { r, g, b } {}
		constexpr RGB(u32 rgb) : r((rgb >> 16) & 0xff), g((rgb >> 8) & 0xff), b(rgb & 0xff) {}
	};

	namespace color_scheme
	{
		namespace gruvbox
		{
			inline namespace dark_mode
			{
				constexpr RGB Bg         = 0x282828;
				constexpr RGB Red_Dark   = 0xcc241d;
				constexpr RGB Green_Dark = 0x98971a;
			}
		}
	}

	// TODO Add explicit type restriction for pixels
	template<typename It, typename End_It = It>
	bool save_ppm(It begin, End_It end, std::string const& filename, uint w, uint h, uint x = 0, uint y = 0)
	{
		std::ofstream file(filename, std::ios::trunc | std::ios::out);
		if (!file)
			return false;

		file << "P6\n" << w << ' ' << h << " 255\n";

		std::advance(begin, w * y + x);

		for (; begin != end; ++begin)
			file.write((char*)begin->rgb.data(), begin->rgb.size());

		return true;
	}

	float distance(float ax, float ay, float bx, float by)
	{
		return std::sqrt((ax - bx) * (ax - bx) + (ay - by) * (ay - by));
	}

	uint digits10(uint x)
	{
		uint r = 1;
		for (;;) {
			if (x < 10) return r;
			if (x < 100) return r + 1;
			if (x < 1'000) return r + 2;
			if (x < 10'000) return r + 3;
			x /= 10'000; r += 4;
		}
	}

	std::string nf(uint value, uint aligment, char align_with = '0')
	{
		auto const len = digits10(value);
		if (len < aligment)
			return std::string(aligment - len, align_with) + std::to_string(value);
		return std::to_string(value);
	}

	RGB lerp(float v, RGB s, RGB e)
	{
		if (v >= 1) return e;
		if (v <= 0) return s;
		// TODO add vector arithmetics to rgb
		return RGB((e.r - s.r) * v + s.r, (e.g - s.g) * v + s.g, (e.b-s.g) * v + s.b);
	}

	RGB grayscale(float v)
	{
		return lerp(v, { 0 }, { 0xffffff });
	}

	struct V2f
	{
		float x, y;
	};

	V2f lerp(float p, V2f s, V2f e)
	{
		if (p >= 1) return e;
		if (p <= 0) return s;
		return { std::lerp(s.x, e.x, p), std::lerp(s.y, e.y, p) };
	}
}

constexpr unsigned Width = 400, Height = 400;
constexpr unsigned Frame_Count = 160;

namespace colors = migs::color_scheme::gruvbox::dark_mode;

struct Metaball
{
	migs::V2f pos;
	float r;
};

int main()
{
	std::array<migs::RGB, Width * Height> pixels = {};

	std::fill(std::begin(pixels), std::end(pixels), migs::color_scheme::gruvbox::dark_mode::Green_Dark);

	static const auto meta = std::array {
		Metaball { { -40., -40 }, 2 },
		Metaball { { -40, Height + 40 }, 2 },
		Metaball { { Width + 40, -40 }, 2 },
		Metaball { { Width + 40, Height + 40 }, 2 }
	};

	for (unsigned frame = 0; frame < Frame_Count; ++frame) {
		auto metaballs = meta;

		auto const percent = float(frame) / Frame_Count;
		auto p = percent;

		metaballs[0].pos = migs::lerp(p, meta[0].pos, meta[3].pos);
		metaballs[1].pos = migs::lerp(p, meta[1].pos, meta[2].pos);
		metaballs[2].pos = migs::lerp(p, meta[2].pos, meta[1].pos);
		metaballs[3].pos = migs::lerp(p, meta[3].pos, meta[0].pos);

		for (uint y = 0; y < Height; ++y) {
			for (uint x = 0; x < Width; ++x) {
				auto const index = Width * y + x;
				float sum = 0;
				for (Metaball const& m : metaballs) {
					float d = migs::distance(x, y, m.pos.x, m.pos.y);
					if (d == 0) {
						d = 0.000001;
					}
					sum += 10 * m.r / d;
				}
				pixels[index] = migs::grayscale(sum);
			}
		}

		// TODO automatically make directory
		migs::save_ppm(pixels.cbegin(), pixels.cend(), "output/metaballs-" + migs::nf(frame, 3) + ".ppm", Width, Height);
	}
}
