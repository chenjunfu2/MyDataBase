#pragma once
#include <stdint.h>
#include <stddef.h>
#include <limits.h>
#include <utility>
#include <type_traits>
#include <concepts>
#include <bit>

namespace CRC_TOOLS
{
	template<size_t>
	inline constexpr bool dependent_false_v = false;

	template<size_t BitsOfType>
	struct TypeSelector
	{
		static_assert(dependent_false_v<BitsOfType>, "Unsupported bit width");// msvc 19.40起才修复，目前只能套一层
	};

	template<>
	struct TypeSelector<64>
	{
		using Type = uint64_t;
	};

	template<>
	struct TypeSelector<32>
	{
		using Type = uint32_t;
	};

	template<>
	struct TypeSelector<16>
	{
		using Type = uint16_t;
	};

	template<>
	struct TypeSelector<8>
	{
		using Type = uint8_t;
	};

	template<size_t BitsOfType>
	struct TypeSwitch : public TypeSelector<BitsOfType>
	{
		constexpr static size_t szTypeBits = BitsOfType;
	};


	template <typename T, size_t Shift>
	requires (std::unsigned_integral<T> &&std::has_single_bit(sizeof(T) * 8) && std::has_single_bit(Shift))
		static constexpr T MaskR = []() -> T
	{
		constexpr size_t szBits = sizeof(T) * 8;

		T tValue = 0;
		T tBitOn = 1;

		for (size_t i = 0; i < szBits; ++i)
		{
			size_t szGroup = i / Shift;
			if (szGroup % 2 == 0)//交替组
			{
				tValue |= tBitOn;
			}

			tBitOn <<= 1;
		}

		return tValue;
	}();

	template <typename T, size_t Shift>
	requires (std::unsigned_integral<T> &&std::has_single_bit(sizeof(T) * 8) && std::has_single_bit(Shift))
		static constexpr T MaskL = []() -> T
	{
		constexpr size_t szBits = sizeof(T) * 8;

		T tValue = 0;
		T tBitOn = 1;

		for (size_t i = 0; i < szBits; ++i)
		{
			size_t szGroup = i / Shift;
			if (szGroup % 2 == 1)//交替组
			{
				tValue |= tBitOn;
			}

			tBitOn <<= 1;
		}

		return tValue;
	}();

	/*
	bits reverse
	12345678
	
	1|2 3|4 5|6 7|8
	2|1 4|3 6|5 8|7
	
	21|43 65|87
	43|21 87|65
	
	4321|8765
	8765|4321
	
	87654321
	*/
	template<typename T>
	requires (std::integral<T> &&std::has_single_bit(sizeof(T) * 8))
	static auto ReverseBits(T tBits)
	{
		using UT = typename std::make_unsigned_t<T>;
		UT uTmp = std::bit_cast<UT>(tBits);

		constexpr size_t szBits = sizeof(T) * 8;//确定有多少bits
		constexpr size_t szExponent = std::countr_zero(szBits);//确定数值由多少个2组成（2的指数）

		[&] <size_t... i>(std::index_sequence<i...>) -> void
		{
			((uTmp = (uTmp & MaskR<UT, (1ULL << i)>) << (1ULL << i) | (uTmp & MaskL<UT, (1ULL << i)>) >> (1ULL << i)), ...);
		}(std::make_index_sequence<szExponent>{});

		return uTmp;
	}

	/*
	static uint64_t ReverseBits64(uint64_t u64Bits)
	{

		static constexpr uint64_t u64MaskR1 = 0x55'55'55'55'55'55'55'55ULL;//0101
		static constexpr uint64_t u64MaskL1 = u64MaskR1 << 1;//1010

		static constexpr uint64_t u64MaskR2 = 0x33'33'33'33'33'33'33'33ULL;//0011
		static constexpr uint64_t u64MaskL2 = u64MaskR2 << 2;//1100

		static constexpr uint64_t u64MaskR4 = 0x0F'0F'0F'0F'0F'0F'0F'0FULL;
		static constexpr uint64_t u64MaskL4 = u64MaskR4 << 4;

		static constexpr uint64_t u64MaskR8 = 0x00'FF'00'FF'00'FF'00'FFULL;
		static constexpr uint64_t u64MaskL8 = u64MaskR8 << 8;

		static constexpr uint64_t u64MaskR16 = 0x00'00'FF'FF'00'00'FF'FFULL;
		static constexpr uint64_t u64MaskL16 = u64MaskR16 << 16;

		static constexpr uint64_t u64MaskR32 = 0x00'00'00'00'FF'FF'FF'FFULL;
		static constexpr uint64_t u64MaskL32 = u64MaskR32 << 32;

		uint64_t u64Tmp = u64Bits;
		u64Tmp = (u64Tmp & u64MaskR1) << 1 | (u64Tmp & u64MaskL1) >> 1;
		u64Tmp = (u64Tmp & u64MaskR2) << 2 | (u64Tmp & u64MaskL2) >> 2;
		u64Tmp = (u64Tmp & u64MaskR4) << 4 | (u64Tmp & u64MaskL4) >> 4;
		u64Tmp = (u64Tmp & u64MaskR8) << 8 | (u64Tmp & u64MaskL8) >> 8;
		u64Tmp = (u64Tmp & u64MaskR16) << 16 | (u64Tmp & u64MaskL16) >> 16;
		u64Tmp = (u64Tmp & u64MaskR32) << 32 | (u64Tmp & u64MaskL32) >> 32;

		return u64Tmp;
	}
	*/
}

template<size_t CRCBits>
requires (std::has_single_bit(CRCBits) && CRCBits >= 8)//2的次方并且至少8
class CRC
{
protected:
	static_assert(CHAR_BIT == 8, "Unsupported platform");
	using CRCTypeSwitch = CRC_TOOLS::TypeSwitch<CRCBits>;

public:
	using CRCType = CRCTypeSwitch::Type;
	constexpr static size_t szTypeBits = CRCTypeSwitch::szTypeBits;

protected:
	bool bRefIn = false;
	CRCType tCrcState = 0;
	CRCType tCrcTable[UINT8_MAX + 1] = {};

protected:
	void UpdateCrcStateRefIn(uint8_t u8Data)
	{
		uint8_t u8Index = (uint8_t)tCrcState ^ u8Data;
		tCrcState >>= 8;
		tCrcState ^= tCrcTable[u8Index];
	}

	void UpdateCrcStateNoRefIn(uint8_t u8Data)
	{
		uint8_t u8Index = (uint8_t)(tCrcState >> (szTypeBits - 8)) ^ u8Data;
		tCrcState <<= 8;
		tCrcState ^= tCrcTable[u8Index];
	}

public:
	//预计算8bit crc状态迁移表
	void ResetCrcTable(CRCType tPoly, bool _bRefIn = true)
	{
		bRefIn = _bRefIn;

		if (bRefIn)
		{
			tPoly = CRC_TOOLS::ReverseBits(tPoly);
			for (uint64_t i = 0; i <= UINT8_MAX; ++i)
			{
				CRCType tCrcStateTmp = i & UINT8_MAX;

				for (uint8_t j = 0; j < 8; ++j)
				{
					tCrcStateTmp = (tCrcStateTmp & 0x01ULL) == 0x01ULL
						? (tCrcStateTmp >> 1) ^ tPoly
						: (tCrcStateTmp >> 1);
				}

				tCrcTable[i] = tCrcStateTmp;
			}
		}
		else
		{
			static constexpr CRCType tUpperBit = ((CRCType)1) << (szTypeBits - 1);
			for (uint64_t i = 0; i <= UINT8_MAX; ++i)
			{
				CRCType tCrcStateTmp = (i & UINT8_MAX) << (szTypeBits - 8);

				for (uint8_t j = 0; j < 8; ++j)
				{
					tCrcStateTmp = (tCrcStateTmp & tUpperBit) == tUpperBit
						? (tCrcStateTmp << 1) ^ tPoly
						: (tCrcStateTmp << 1);
				}

				tCrcTable[i] = tCrcStateTmp;
			}
		}
	}

	//指定初始状态开始计算
	void ResetCrcState(CRCType tInit)
	{
		tCrcState = tInit;
	}

	//迭代（字节)
	void UpdateCrcState(uint8_t u8Data)
	{
		if (bRefIn)
		{
			UpdateCrcStateRefIn(u8Data);
		}
		else
		{
			UpdateCrcStateNoRefIn(u8Data);
		}
	}

	//迭代（内存）
	void UpdateCrcState(const void *pData, size_t szDataSize)
	{
		const uint8_t *pu8Data = (const uint8_t *)pData;

		if (bRefIn)
		{
			for (size_t i = 0; i < szDataSize; ++i)
			{
				UpdateCrcStateRefIn(pu8Data[i]);
			}
		}
		else
		{
			for (size_t i = 0; i < szDataSize; ++i)
			{
				UpdateCrcStateNoRefIn(pu8Data[i]);
			}
		}
	}

	//迭代（数组）
	template<typename T, size_t N>
	void UpdateCrcState(const T(&arrData)[N])
	{
		UpdateCrcState((const void *)&arrData[0], sizeof(arrData));
	}

	//得到当前迭代的CRC结果
	[[nodiscard]]
	CRCType GetCrcState(bool bRefOut = true, CRCType tXorOut = 0)
	{
		if (bRefOut != bRefIn)//等效于bRefOut ^ bRefIn
		{
			return CRC_TOOLS::ReverseBits(tCrcState) ^ tXorOut;
		}
		else
		{
			return tCrcState ^ tXorOut;
		}
	}
};

using CRC64 = CRC<64>;
using CRC32 = CRC<32>;
using CRC16 = CRC<16>;
using CRC8 = CRC<8>;
