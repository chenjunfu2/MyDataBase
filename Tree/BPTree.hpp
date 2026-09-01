#pragma once
#include <stdint.h>

#include <vector>
#include <span>
#include <type_traits>
#include <bit>

//全局强制小端序
static_assert(std::endian::native == std::endian::little, "Database requires little-endian platform");

template<uint64_t u64PageSize = 4096>
struct Page
{
public:
	uint8_t u8PageData[u64PageSize];

	using SpanType = std::span<uint8_t>;
	using CSpanType = std::span<const uint8_t>;

	constexpr static const uint64_t u64MagicNumber = 0x43'4A'46'32'4D'59'44'42;// "CJF2MYDB"
	constexpr static const uint64_t u64CurrentPageVersion = 0x00'00'00'00'00'00'00'00;// 0.0版本，高32bit为major version，低32bit为minor version

	enum class PageType : uint8_t
	{
		INVALID = 0,
		RECORD_DATA = 1,
		INTERNAL_NODE = 2,
		LEAF_NODE = 3,
	};

#pragma pack(push, 1)
	struct PageHeader
	{
		uint64_t u64MagicNumber;//判断是否是此数据库页面
		uint64_t u64PageVersion;//页面自身版本号
		uint64_t u64PageLSN;//页面序列号
		uint64_t u64PagePayloadSize;//实际数据大小
		uint64_t u64Checksum;//校验值
		uint8_t u8PageType;//页面存储的数据类型

		uint8_t u8Padding[3];//对齐4bytes边界，无用
	};
#pragma pack(pop)
	constexpr static const uint64_t u64CurrentPageHeaderSize = sizeof(PageHeader);


public:





	SpanType ToSpan() noexcept
	{
		return SpanType{ u8PageData };
	}

	CSpanType ToSpan() const noexcept
	{
		return CSpanType{ u8PageData };
	}
};

template <typename Page, typename Key, typename NodeRef, typename RecordRef>
class NodeCodec
{
public:
	using PageType = Page;
	using KeyType = Key;
	using NodeRefType = NodeRef;
	using RecordRefType = RecordRef;

public:
	class InternalNodeSpanView
	{
	public:
		using KeySpanType = std::span<Key>;
		using NodeRefSpanType = std::span<NodeRef>;

	public:
		KeySpanType spanKey;
		NodeRefSpanType spanNodeRef;
	};

	class LeafNodeSpanView
	{
	public:
		using KeySpanType = std::span<Key>;
		using RecordRefSpanType = std::span<RecordRef>;

	public:
		NodeRef nodePrev;
		NodeRef nodeNext;

		KeySpanType spanKey;
		RecordRefSpanType spanRecordRef;
	};

public:











};



template<typename NodeRef>
class NodeRefList
{
public:
	std::vector<NodeRef> listNodeRef;

public:



};



template
<
	typename Node,
	typename NodeRef,
	typename NodeRefList
>
class BPTree
{
public:
	using NodeType = typename Node;
	using NodeRefType = typename NodeRef;
	using NodeRefListType = typename NodeRefList;

protected:
	struct 








public:





};


using MemBPTree = BPTree<MemNode, MemNodeRef<MemNode>, NodeRefList<MemNodeRef<MemNode>>>;
