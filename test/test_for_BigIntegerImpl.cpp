#define private public
#include "../src/BigIntegerImpl.h"

using namespace pgbn;

#undef PGZXB_DEBUG_INFO_HEADER
#define PGZXB_DEBUG_INFO_HEADER "[DEBUG] "

#define TEST_PRINT_BI(a) \
    PGZXB_DEBUG_Print("=============================================================["); \
    PGZXB_DEBUG_PrintVar(a.m_flags); \
    PGZXB_DEBUG_PrintVar(a.m_mag.m_data.get()); \
    std::cout << "Mag(little endian) : "; \
    if (a.m_mag.m_data.get()) { \
        for (auto iter = a.m_mag.end() - 1; iter >= a.m_mag.begin(); --iter) \
            std::cout << std::bitset<32>(*iter) << '\''; \
    } \
    std::cout << "\n"; \
    PGZXB_DEBUG_Print("=============================================================]\n")

#define TEST_ASSIGN_U64(u64, a) \
    PGZXB_DEBUG_Print("=============================================================["); \
    PGZXB_DEBUG_PrintVar(std::bitset<64>(u64)); \
    PGZXB_DEBUG_PrintVar(a.m_flags); \
    PGZXB_DEBUG_PrintVar(a.m_mag.m_data.get()); \
    std::cout << "Mag(little endian) : "; \
    for (auto iter = a.m_mag.end() - 1; iter >= a.m_mag.begin(); --iter) \
        std::cout << std::bitset<32>(*iter) << '\''; \
    std::cout << "\n"; \
    PGZXB_DEBUG_Print("=============================================================]\n")

#define TEST_ASSIGN_BIN(little, bin, len, a) { \
        PGZXB_DEBUG_Print("=============================================================[\n"); \
        const Byte * src = reinterpret_cast<const Byte*>(bin); \
        if (!(little)) { \
            for (SizeType i = 0; i < len; ++i) std::cout << std::bitset<8>(src[i]) << '\''; \
        } else { \
            for (int i = len - 1; i >= 0; --i) std::cout << std::bitset<8>(src[i]) << '\''; \
        } \
        std::cout << "\n"; \
        PGZXB_DEBUG_PrintVar(a.m_flags); \
        PGZXB_DEBUG_PrintVar(a.m_mag.m_data.get()); \
        std::cout << "Mag(little endian) : "; \
        for (auto iter = a.m_mag.end() - 1; iter >= a.m_mag.begin(); --iter) \
            std::cout << std::bitset<32>(*iter) << '\''; \
        std::cout << "\n"; \
        PGZXB_DEBUG_Print("=============================================================]\n"); \
    } PGZXB_PASS

// namespace BNFlag {

// constexpr Enum INVALID = 0;           // 0
// constexpr Enum ZERO = 1U;             // 1
// constexpr Enum POSITIVE = 1U << 1U;   // 2
// constexpr Enum NEGATIVE = 1U << 2U;   // 4

// }

int main () {
    std::uint64_t u64 = 0xfffffffff;
    pgbn::BigIntegerImpl a;

    TEST_ASSIGN_U64(u64, a);

    u64 = 0xf0f0f0f0f;
    a.assign(-u64);
    TEST_ASSIGN_U64(u64, a);

    a.assign(0);
    TEST_ASSIGN_U64(0, a);

    a.assign(u64);
    TEST_PRINT_BI(a);
    pgbn::BigIntegerImpl b(a);
    TEST_PRINT_BI(b);

    b.assign(a);
    TEST_PRINT_BI(b);

    b.assign(-1);
    TEST_ASSIGN_U64(1, b);

    b.assign(b);
    TEST_PRINT_BI(b);

    b.assign(std::move(a));
    TEST_PRINT_BI(a);
    TEST_PRINT_BI(b);

    a.assign(-16);
    TEST_ASSIGN_U64(16, a);

    std::cout << "\n\n+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n\n";

    bool little = true;
    u64 = 0x103070F1F;
    SizeType len = 10;
    void * bin = std::malloc(len);
    Byte * s = reinterpret_cast<Byte *>(bin);
    std::memset(s, 0, len);
    s[0] = 0x0;
    s[1] = 0x1F;
    s[2] = 0xF;
    s[3] = 0x7;
    s[4] = 0x3;
    s[5] = 0x1;

    a.assign(bin, len, little);
    TEST_ASSIGN_BIN(little, bin, len, a);

    a.assign(bin, len, false);
    TEST_ASSIGN_BIN(false, bin, len, a);

    // test-result :
    // 11111111'11111111'11111111'11111111'11111111'00000000'00000001'00000011'00000111'00001111'00011111'00000000' 
    // 00000000'11111111'00000000'00000000'00000000'11111111'11111110'11111100'11111000'11110000'11100001'00000000' 
    s[9] = s[8] = s[7] = 0xff;
    a.assign(bin, len, little);
    TEST_ASSIGN_BIN(little, bin, len, a);

    // test-result :
    // 11111111'11111111'11111111'00011111'00001111'00000111'00000011'00000001'00000000'11111111'11111111'11111111'
    // 00000000'00000000'00000000'11100000'11110000'11111000'11111100'11111110'11111111'00000000'00000000'00000001'
    s[0] = 0xff;
    a.assign(bin, len, false);
    TEST_ASSIGN_BIN(false, bin, len, a);    
    
    // 0
    std::memset(s, 0, len);
    a.assign(bin, len, true);
    TEST_ASSIGN_BIN(true, bin, len, a);  

    a.assign(bin, len, false);
    TEST_ASSIGN_BIN(false, bin, len, a);

    // -1
    std::memset(s, -1, len);
    a.assign(bin, len, true);
    TEST_ASSIGN_BIN(true, bin, len, a);  

    a.assign(bin, len, false);
    TEST_ASSIGN_BIN(false, bin, len, a);

    std::int64_t i64 = -0xff;
    a.assign((void*)&i64, sizeof(i64), true);
    TEST_ASSIGN_BIN(true, bin, len, a);

    i64 = 0;
    a.assign((void*)&i64, sizeof(i64), true);
    TEST_ASSIGN_BIN(true, bin, len, a);

    i64 = -1;
    a.assign((void*)&i64, sizeof(i64), true);
    TEST_ASSIGN_BIN(true, bin, len, a);

    i64 = -128;
    a.assign((void*)&i64, sizeof(i64), true);
    TEST_ASSIGN_BIN(true, bin, len, a);

    i64 = std::numeric_limits<std::int32_t>::min();
    a.assign((void*)&i64, sizeof(i64), true);
    TEST_ASSIGN_BIN(true, bin, len, a);

    i64 = std::int64_t(std::numeric_limits<std::int32_t>::min()) << 1;
    a.assign((void*)&i64, sizeof(i64), true);
    TEST_ASSIGN_BIN(true, bin, len, a);

    i64 = std::numeric_limits<std::int32_t>::max();
    a.assign((void*)&i64, sizeof(i64), true);
    TEST_ASSIGN_BIN(true, bin, len, a);

    i64 = std::numeric_limits<std::uint32_t>::max();
    a.assign((void*)&i64, sizeof(i64), true);
    TEST_ASSIGN_BIN(true, bin, len, a);

    i64 = std::numeric_limits<std::uint32_t>::min();
    a.assign((void*)&i64, sizeof(i64), true);
    TEST_ASSIGN_BIN(true, bin, len, a);

    i64 = std::numeric_limits<std::int64_t>::max();
    a.assign((void*)&i64, sizeof(i64), true);
    TEST_ASSIGN_BIN(true, bin, len, a);

    i64 = std::numeric_limits<std::int64_t>::min();
    a.assign((void*)&i64, sizeof(i64), true);
    TEST_ASSIGN_BIN(true, bin, len, a);

    std::free(bin);

    return 0;
}