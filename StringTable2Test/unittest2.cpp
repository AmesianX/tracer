#include "stdafx.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#define PTR(p) ((ULONG_PTR)(p))
#define LEN(String) ((LONG)((STRING)(String)).Length)

MAKE_STRING(Foo);
MAKE_STRING(Bar);

#define DELIM ';'

#define DELIMITED_TABLE(String)                              \
    StringTable = Api->CreateStringTableFromDelimitedString( \
        Rtl,                                                 \
        Allocator,                                           \
        Allocator,                                           \
        String,                                              \
        DELIM                                                \
    );                                                       \
    Assert::IsNotNull(StringTable)

#define ASSERT_FAILED_DELIMITED_TABLE(String)                \
    StringTable = Api->CreateStringTableFromDelimitedString( \
        Rtl,                                                 \
        Allocator,                                           \
        Allocator,                                           \
        String,                                              \
        DELIM                                                \
    );                                                       \
    Assert::IsNull(StringTable)

#define ASSERT_SIZE(Size)                                \
    Assert::AreEqual(                                    \
        (ULONG)(Size),                                   \
        (ULONG)(__popcnt16(StringTable->OccupiedBitmap)) \
    )

#define NtfsIsPrefixInTable(Prefix)                      \
    (NTFS_RESERVED_NAME_ID)Api->IsPrefixOfStringInTable( \
        StringTable,                                     \
        (PSTRING)Prefix,                                 \
        NULL                                             \
    )

#define TEST_NTFS_NAME(N)                         \
    NameId = NtfsIsPrefixInTable(&Ntfs##N##Name); \
    Assert::IsTrue(NameId == Ntfs##N)

#define NtfsIsPrefixInTable4(Prefix)                       \
    (NTFS_RESERVED_NAME_ID)Api->IsPrefixOfStringInTable_4( \
        StringTable,                                       \
        (PSTRING)Prefix,                                   \
        NULL                                               \
    )

#define TEST_NTFS_NAME4(N)                         \
    NameId = NtfsIsPrefixInTable4(&Ntfs##N##Name); \
    Assert::IsTrue(NameId == Ntfs##N)


extern PRTL Rtl;
extern PALLOCATOR Allocator;
extern PSTRING_TABLE_FUNCTIONS Api;

namespace TestStringTable
{
    TEST_CLASS(UnitTest2)
    {
    public:

        TEST_METHOD(TestCreateStringTableFromDelimitedString1)
        {
            MAKE_STRING(Foo);
            PSTRING_TABLE StringTable;

            StringTable = Api->CreateStringTableFromDelimitedString(
                Rtl,
                Allocator,
                Allocator,
                &Foo,
                ';'
            );

            Assert::IsNotNull(StringTable);

            Api->DestroyStringTable(Allocator,
                                    Allocator,
                                    StringTable);

        }

        TEST_METHOD(TestCreateStringTableFromDelimitedString2)
        {
            PSTRING_TABLE StringTable;

            STRING Test1 = RTL_CONSTANT_STRING(";Foo;Bar");
            STRING Test2 = RTL_CONSTANT_STRING(";Foo;;Bar");
            STRING Test3 = RTL_CONSTANT_STRING(";;Foo;Bar");
            STRING Test4 = RTL_CONSTANT_STRING("Foo;Bar;;;");
            STRING Test5 = RTL_CONSTANT_STRING(";;;;;Foo;B");
            STRING Test6 = RTL_CONSTANT_STRING("Foo;;;;;;;B;");

            DELIMITED_TABLE(&Test1);
            ASSERT_SIZE(2);
            DESTROY_TABLE(StringTable);

            DELIMITED_TABLE(&Test2);
            ASSERT_SIZE(2);
            DESTROY_TABLE(StringTable);

            DELIMITED_TABLE(&Test3);
            ASSERT_SIZE(2);
            DESTROY_TABLE(StringTable);

            DELIMITED_TABLE(&Test4);
            ASSERT_SIZE(2);
            DESTROY_TABLE(StringTable);

            DELIMITED_TABLE(&Test5);
            ASSERT_SIZE(2);
            DESTROY_TABLE(StringTable);

            DELIMITED_TABLE(&Test6);
            ASSERT_SIZE(2);
            DESTROY_TABLE(StringTable);

        }

        TEST_METHOD(TestCreateStringTableFromDelimitedString3)
        {
            PSTRING_TABLE StringTable;

            STRING Test7 = RTL_CONSTANT_STRING(";;;");

            ASSERT_FAILED_DELIMITED_TABLE(&Test7);
        }

        TEST_METHOD(TestCreateStringTableFromDelimitedString4)
        {
            PSTRING_TABLE StringTable;

            STRING Test8 = RTL_CONSTANT_STRING("foo");

            DELIMITED_TABLE(&Test8);
            ASSERT_SIZE(1);
            DESTROY_TABLE(StringTable);
        }

        TEST_METHOD(TestNtfsNames)
        {
            STRING Dummy = RTL_CONSTANT_STRING("Dummy");
            PSTRING_TABLE StringTable;
            NTFS_RESERVED_NAME_ID NameId;

            DELIMITED_TABLE(&NtfsReservedNames);
            ASSERT_SIZE(16);

            NameId = NtfsIsPrefixInTable(&NtfsAttrDefName);
            Assert::IsTrue(NameId == NtfsAttrDef);

            TEST_NTFS_NAME(AttrDef);
            TEST_NTFS_NAME(Mft);
            TEST_NTFS_NAME(MftMirr);
            TEST_NTFS_NAME(LogFile);
            TEST_NTFS_NAME(Volume);
            TEST_NTFS_NAME(Bitmap);
            TEST_NTFS_NAME(Boot);
            TEST_NTFS_NAME(BadClus);
            TEST_NTFS_NAME(Extend);
            TEST_NTFS_NAME(Cairo);
            TEST_NTFS_NAME(UpCase);
            TEST_NTFS_NAME(IndexAllocation);
            TEST_NTFS_NAME(Data);
            TEST_NTFS_NAME(Unknown);
            TEST_NTFS_NAME(Dot);

            NameId = NtfsIsPrefixInTable(&Dummy);
            Assert::IsTrue(NameId == NotNtfsReservedName);
        }

        TEST_METHOD(TestNtfsNames2)
        {
            STRING Dummy = RTL_CONSTANT_STRING("Dummy");
            PSTRING_TABLE StringTable;
            NTFS_RESERVED_NAME_ID NameId;

            DELIMITED_TABLE(&NtfsReservedNames);
            ASSERT_SIZE(16);

            NameId = NtfsIsPrefixInTable4(&NtfsAttrDefName);
            Assert::IsTrue(NameId == NtfsAttrDef);

            TEST_NTFS_NAME4(AttrDef);
            TEST_NTFS_NAME4(Mft);
            TEST_NTFS_NAME4(MftMirr);
            TEST_NTFS_NAME4(LogFile);
            TEST_NTFS_NAME4(Volume);
            TEST_NTFS_NAME4(Bitmap);
            TEST_NTFS_NAME4(Boot);
            TEST_NTFS_NAME4(BadClus);
            TEST_NTFS_NAME4(Extend);
            TEST_NTFS_NAME4(Cairo);
            TEST_NTFS_NAME4(UpCase);
            TEST_NTFS_NAME4(IndexAllocation);
            TEST_NTFS_NAME4(Data);
            TEST_NTFS_NAME4(Unknown);
            TEST_NTFS_NAME4(Dot);

            NameId = NtfsIsPrefixInTable(&Dummy);
            Assert::IsTrue(NameId == NotNtfsReservedName);
        }

    };
}


