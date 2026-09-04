#include "CRC.hpp"

#include <stdio.h>
#include <string>
#include <charconv>
#include <variant>
#include <format>

static_assert(
	SimpleCrc<CRC32>
	(
		std::span<const char, 9>{"123456789", 9},
		0x04C11DB7u,
		0xFFFFFFFFu,
		0xFFFFFFFFu,
		true,
		true
	) == 0xCBF43926u
);

constexpr bool TestCRC32Residue()
{
	constexpr uint8_t data[] = {
		'1','2','3','4','5','6','7','8','9'
	};

	constexpr CRC32::CRCType poly = 0x04C11DB7u;
	constexpr CRC32::CRCType init = 0xFFFFFFFFu;
	constexpr CRC32::CRCType xorOut = 0xFFFFFFFFu;

	constexpr CRC32::CRCType check = 0xCBF43926u;
	constexpr CRC32::CRCType residue = 0xDEBB20E3u;

	CRC32 crc{};

	crc.ResetCrcTable(poly, true);
	crc.ResetCrcState(init);

	// "123456789"
	crc.UpdateCrcState(std::span{ data });

	// 验证 Check
	if (crc.GetCrcState(true, xorOut) != check)
		return false;

	// CRC-32/ISO-HDLC 为 RefIn=true / RefOut=true，
	// 因此最终 CRC 按 little-endian 顺序追加到数据后面。
	crc.UpdateCrcState(static_cast<uint8_t>((check >> 0) & 0xFF));
	crc.UpdateCrcState(static_cast<uint8_t>((check >> 8) & 0xFF));
	crc.UpdateCrcState(static_cast<uint8_t>((check >> 16) & 0xFF));
	crc.UpdateCrcState(static_cast<uint8_t>((check >> 24) & 0xFF));

	// 此时：
	// bRefOut == bRefIn == true
	// XorOut   == 0
	//
	// 所以 GetCrcState() 就等效于内部 raw state。
	return crc.GetCrcState(true, 0) == residue;
}

static_assert(TestCRC32Residue());

void Test(void)
{
	constexpr char arr[] = "Hello World!";
	printf("str = %s\n\n", arr);

	{
		//CRC64 crc;
		//crc.ResetCrcTable(0x42F0E1EBA9EA3693ULL);
		//crc.ResetCrcState(0xFFFFFFFFFFFFFFFFULL);
		//crc.UpdateCrcState(arr, sizeof(arr) - 1);
		//printf("CRC64 = 0x%llX\n", crc.GetCrcState(true, 0xFFFFFFFFFFFFFFFFULL));
		constexpr auto c = SimpleCrc<CRC64>(std::span{ arr, sizeof(arr) - 1 }, 0x42F0E1EBA9EA3693ULL, 0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL);
		printf("CRC64 = 0x%llX\n", c);
	}

	{
		CRC32 crc;
		crc.ResetCrcTable(0x04C11DB7U);
		crc.ResetCrcState(0xFFFFFFFFU);
		crc.UpdateCrcState(std::span{ arr, sizeof(arr) - 1 });
		printf("CRC32 = 0x%X\n", crc.GetCrcState(true, 0xFFFFFFFFU));
	}

	{
		CRC16 crc;
		crc.ResetCrcTable(0x8005);
		crc.ResetCrcState(0);
		crc.UpdateCrcState(std::span{ arr, sizeof(arr) - 1 });
		printf("CRC16 = 0x%X\n", crc.GetCrcState());

	}

	{
		CRC8 crc;
		crc.ResetCrcTable(0x07);
		crc.ResetCrcState(0);
		crc.UpdateCrcState(std::span{ arr, sizeof(arr) - 1 });
		printf("CRC8 = 0x%X\n", crc.GetCrcState());
	}
}

uint64_t GetBits(const char *pcInput)
{
	uint64_t u64Length = strlen(pcInput);

	if (u64Length < 1)
	{
		printf("Error: empty bits param\n");
		exit(-1);
	}

	uint64_t u64Ret = 0;
	auto [ptr, ec] = std::from_chars(&pcInput[0], &pcInput[u64Length], u64Ret, 10);

	if (ec != std::errc{} || ptr != &pcInput[u64Length])
	{
		printf("Error bits param: \"%s\"\n", pcInput);
		exit(-1);
	}

	switch (u64Ret)
	{
	case 64: case 32:
	case 16: case 8:
		return u64Ret;
		break;
	default:
		printf("Unsupported bit width: \"%s\"\n", pcInput);
		exit(-1);
		break;
	}

	//不应该执行到
	printf("Fatal error!\n");
	exit(-1);
}

uint64_t GetHex(const char *pcInput)
{
	uint64_t u64Length = strlen(pcInput);

	if (u64Length < 1)
	{
		printf("Error: empty hex param\n");
		exit(-1);
	}

	uint64_t u64Ret = 0;
	auto [ptr, ec] = std::from_chars(&pcInput[0], &pcInput[u64Length], u64Ret, 16);

	if (ec != std::errc{} || ptr != &pcInput[u64Length])
	{
		printf("Error hex param: \"%s\"\n", pcInput);
		exit(-1);
	}

	return u64Ret;
}

bool GetBool(const char *pcInput)
{
	uint64_t u64Length = strlen(pcInput);

	if (u64Length < 1)
	{
		printf("Error: empty bool param\n");
		exit(-1);
	}

	if (u64Length != 1)
	{
		printf("Error bool param: \"%s\"\n", pcInput);
		exit(-1);
	}

	char c = pcInput[0];
	switch (c)
	{
	case 't':
	case 'T':
	case '1':
		return true;
		break;
	case 'f':
	case 'F':
	case '0':
		return false;
		break;
	default:
		printf("Unknown bool param: \"%s\"\n", pcInput);
		exit(-1);
		break;
	}

	//不应该执行到
	printf("Fatal error!\n");
	exit(-1);
}


int main(int argc, char *argv[])
{
	if (argc != 8)
	{
		printf("Use: \"%s\" [Bits(8/16/32/64)] [Poly(Hex)] [Init(Hex)] [XorOut(Hex)] [RefIn(t/f)] [RefOut(t/f)] [FilePath]\n", argv[0]);
		return 0;
	}
	
	auto bits = GetBits(argv[1]);
	auto poly = GetHex(argv[2]);
	auto init = GetHex(argv[3]);
	auto xorout = GetHex(argv[4]);
	auto refin = GetBool(argv[5]);
	auto refout = GetBool(argv[6]);
	auto file = argv[7];


	FILE *pFile = fopen(file, "rb");
	if (pFile == NULL)
	{
		printf("Open file fail: \"%s\"\n", file);
		exit(-1);
	}

	std::variant<CRC64, CRC32, CRC16, CRC8> vCRC;
	switch (bits)
	{
		case 64:	vCRC.emplace<CRC64>();	break;
		case 32:	vCRC.emplace<CRC32>();	break;
		case 16:	vCRC.emplace<CRC16>();	break;
		case 8:		vCRC.emplace<CRC8>();	break;
	}

	std::visit
	(
		[&]<typename T>(T & crc) -> void
		{
			crc.ResetCrcTable(poly, refin);
			crc.ResetCrcState(init);
		},
		vCRC
	);

	constexpr size_t szBlockSize = 1024*1024*64;//64mb: 1024byte = 1kb  1024kb = 1mb     
	uint8_t *u8DataBlock = new uint8_t[szBlockSize];
	size_t szRead = 0;

	printf("File = \"%s\"\n", file);

	do
	{
		szRead = fread(u8DataBlock, 1, szBlockSize, pFile);

		std::visit
		(
			[&]<typename T>(T &crc) -> void
			{
				crc.UpdateCrcState(&u8DataBlock[0], szRead);
			},
			vCRC
		);

	} while (szRead == szBlockSize);//直到某次处理不为szBlockSize，说明读完

	delete[] u8DataBlock;
	u8DataBlock = NULL;
	fclose(pFile);
	pFile = NULL;

	std::visit
	(
		[&]<typename T>(T & crc) -> void
		{
			typename T::CRCType crcVal = crc.GetCrcState(refout, xorout);
			printf("%s", std::format("CRC{} = {:X}\n", bits, crcVal).c_str());
		},
		vCRC
	);

	return 0;
}
