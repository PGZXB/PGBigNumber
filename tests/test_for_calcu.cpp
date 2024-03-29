// Test +, -, *, /, mod... of pgbn::BigIntegerImpl.
// Depends on: Python3.
// Workflow: 
//      Generate examples -> Generate Python script from template -> Run[Check & Output testing info]
#include "pgdebug.h" // format
#include "BigIntegerImpl.h"

#include <iostream>
#include <fstream>
#include <random>
#include <unordered_map>
#include <tuple>
#include <chrono>
#include <ctime>
#include <cstring>

#ifndef _MSC_VER
#include <sys/types.h>
#include <unistd.h>
#endif // !_MSC_VER

/** Python code template:
 * 
 * test_log_file = <log-file>
 * log_file = open(test_log_file, 'w')
 * test_count = <M>
 * err_count = 0
 * test_val = [
 *      '<test-example-string 0>'
 *      '<test-example-string 1>'
 *      ...
 *      '<test-example-string m-1>'
 *      '<test-example-string m>'
 * ]
 * err_val = []
 * for e in test_val:
 *     if eval(e) == False:
 *         err_count += 1
 *         err_val.append(e)
 * 
 * log_file.write("Test Finished, err_count/test_count = {0}/{1}\n".format(err_count, test_count))
 * log_file.write("err-examples :\n");
 * for e in err_val:
 *     log_file.write("\t{0}\n".format(e))
 * 
 * log_file.close()
 */

struct ResultInfo {
    std::string msg = "";
    std::uint32_t cnt = 0;
    std::uint32_t nanosec = 0;
};

namespace pg::util::stringUtil::__IN_fmtUtil {

template<>
inline std::enable_if_t<has_iterator<std::vector<std::string>>::value, std::string>
    transToString<std::vector<std::string>>(const std::vector<std::string> & ele, const std::string &) {
    
    std::string res = "[\n";

    for (const auto & e : ele) {
        res.append("    \'")
           .append(e)
           .append("\',\n");
    }

    return res.append("]\n");
}

}

inline std::string strToPythonHex(std::string && other) {
    PGZXB_DEBUG_ASSERT(other.size() > 0);
    // ab0192ff -> 0xab0192ff
    // -ab0192ff -> -0xab0192ff
    if (other.front() == '-') {
        other.insert(1, "0x");
    } else {
        other.insert(0, "0x");
    }

    return std::move(other);
}

std::tuple<std::vector<std::string>, std::vector<ResultInfo>> 
    generateExamples(std::uint32_t randSeed, std::uint32_t n, std::uint32_t m, std::uint32_t minbytes, std::uint32_t maxbytes) {
    struct AutoDeleter {
        ~AutoDeleter() { free(ptr); }
        void * ptr = nullptr;
    };
    struct TimeCounter {
        TimeCounter(std::uint32_t & cntBinding) 
        : cnt(cntBinding), start(std::chrono::steady_clock::now().time_since_epoch().count()) {
        }

        ~TimeCounter() {
            cnt += (std::chrono::steady_clock::now().time_since_epoch().count() - start);
        }

        std::uint32_t & cnt;
        std::uint32_t start = 0;
    };
    struct BIImplAndStr {
        BIImplAndStr(const pgbn::BigIntegerImpl & bii, const std::string & str)
            : b(bii), s(str) { }
        pgbn::BigIntegerImpl b;
        std::string s;
    };

    std::vector<std::string> res;
    std::vector<BIImplAndStr> numbers;
    std::default_random_engine eng(randSeed);

    { // Generate n BigIntegerImpl(s) randomly
        AutoDeleter deleter;

        std::uniform_int_distribution<std::uint32_t> gLen(minbytes, maxbytes);
        std::uniform_int_distribution gByte(0, 255);

        pgbn::Byte * bytes = (pgbn::Byte *)std::malloc(maxbytes);
        deleter.ptr = bytes;

        for (std::uint32_t i = 0; i < n; ++i) {
            // Rand length[minbytes, maxbytes]
            std::uint32_t len = gLen(eng);
            // Rand bytes
            for (std::uint32_t j = 0; j < len; ++j)
                bytes[j] = gByte(eng);
            // Create BigIntegerImpl & Add it to numbers
            auto & temp = numbers.emplace_back(pgbn::BigIntegerImpl(bytes, len), "");
            // Cache hex-string
            temp.s = strToPythonHex(temp.b.toString(16));
        }
    }

    std::uniform_int_distribution<std::uint32_t> gPair(0, n - 1);
    std::uniform_int_distribution<std::uint64_t> gU64(0, 10000);

    // Test callbacks
#define DEFINE_RAND_GEN_BINARY_OPERATION(op, opN) \
    [&numbers, &eng, &gPair, &res] (auto getTimeCounter) { \
        auto & a = numbers[gPair(eng)]; \
        auto & b = numbers[gPair(eng)]; \
        pgbn::BigIntegerImpl copy(a.b); \
        { \
            auto tmCnt = getTimeCounter(); \
            copy.opN##Assign(b.b); \
        } \
        res.emplace_back(a.s) \
            .append(" " op " ") \
            .append(b.s) \
            .append(" == ") \
            .append(strToPythonHex(copy.toString(16))); \
    }
#define DEFINE_RAND_GEN_SHIFT_OPERATION(op, opN) \
    [&numbers, &eng, &gPair, &gU64, &res] (auto getTimeCounter) { \
        auto & a = numbers[gPair(eng)]; \
        auto u64 = gU64(eng); \
        pgbn::BigIntegerImpl copy(a.b); \
        { \
            auto tmCnt = getTimeCounter(); \
            copy.opN##Assign(u64); \
        } \
        res.emplace_back(a.s) \
            .append(" " op " ") \
            .append(std::to_string(u64)) \
            .append(" == ") \
            .append(strToPythonHex(copy.toString(16))); \
    }

    auto test_divide = [&numbers, &eng, &gPair, &res] (auto getTimeCounter) {
        // Python's divide-operation differ with PGBigNumber(C++ like)
        auto a = numbers[gPair(eng)];
        auto b = numbers[gPair(eng)];
        bool an = false, bn = false;
        if (a.b.flagsContains(pgbn::BNFlag::NEGATIVE)) {
            a.b.negate();
            an = true;
            if (&a == &b) bn = true;
        }
        if (b.b.flagsContains(pgbn::BNFlag::NEGATIVE)) { b.b.negate(); bn = true; }
        PGZXB_DEBUG_ASSERT(a.b.flagsContains(pgbn::BNFlag::ZERO) || (a.b.flagsContains(pgbn::BNFlag::POSITIVE) && !a.b.flagsContains(pgbn::BNFlag::NEGATIVE)));
        PGZXB_DEBUG_ASSERT(b.b.flagsContains(pgbn::BNFlag::ZERO) || (b.b.flagsContains(pgbn::BNFlag::POSITIVE) && !b.b.flagsContains(pgbn::BNFlag::NEGATIVE)));
        if (b.b.flagsContains(pgbn::BNFlag::ZERO)) return;
        pgbn::BigIntegerImpl q, r;
        {
            auto tmCnt = getTimeCounter();
            pgbn::BigIntegerImpl::div(q, r, a.b, b.b);
        }
        res.emplace_back((an ? a.s.substr(1) : a.s))
            .append(" // ")
            .append((bn ? b.s.substr(1) : b.s))
            .append(" == ")
            .append(strToPythonHex(q.toString(16)));
        res.emplace_back((an ? a.s.substr(1) : a.s))
            .append(" % ")
            .append((bn ? b.s.substr(1) : b.s))
            .append(" == ")
            .append(strToPythonHex(r.toString(16)));
    };

    pgbn::Byte * buf = reinterpret_cast<pgbn::Byte*>(::malloc(maxbytes + 1));
    std::memset(buf, 0, maxbytes + 1);
    AutoDeleter deleter = { buf };
    auto test_copyMagDataTo = [&numbers, &eng, &gPair, &res, &buf, maxbytes] (auto getTimeCounter) {
        auto a = numbers[gPair(eng)]; // Copy
        a.b.abs(); // Be positive
        a.s = a.s.front() == '-' ? a.s.substr(1) : a.s;  // be positive

        pgbn::BigIntegerImpl test;
        {   
            auto tmCnt = getTimeCounter();
            auto len = a.b.copyMagDataTo(buf, maxbytes); // Get bin
            buf[len] = 0; // Be positive
            test.assign(buf, len + 1, true); // From bin, 
        }
        PGZXB_DEBUG_ASSERT(test.flagsContains(pgbn::BNFlag::POSITIVE) || test.flagsContains(pgbn::BNFlag::ZERO));

        res.emplace_back(a.s)
            .append(" == ")
            .append(strToPythonHex(test.toString(16)));
    };

    std::function<void(std::function<TimeCounter()>)> operations[] = {
        DEFINE_RAND_GEN_BINARY_OPERATION("+", add),
        DEFINE_RAND_GEN_BINARY_OPERATION("-", sub),
        DEFINE_RAND_GEN_BINARY_OPERATION("*", mul),
        test_divide,
        DEFINE_RAND_GEN_SHIFT_OPERATION("<<", shiftLeft),
        DEFINE_RAND_GEN_SHIFT_OPERATION(">>", shiftRight),
        test_copyMagDataTo,
    };

    std::vector<ResultInfo> infos = {
        { "Add                 ", 0, 0 },
        { "Sub                 ", 0, 0 },
        { "Multi               ", 0, 0 },
        { "Floor-Divide And Mod", 0, 0 },
        { "ShiftLeft           ", 0, 0 },
        { "ShiftRight          ", 0, 0 },
        { "Get-Bin And From-Bin", 0, 0 },
    };

    auto getTimeCounterGenerator = [&infos] (std::uint32_t index) {
        return [index, &infos] () {
            return TimeCounter(infos[index].nanosec);
        };
    };

    const std::uint32_t opNum = sizeof(operations) / sizeof(*operations);
    PGZXB_DEBUG_ASSERT(opNum == infos.size());

    std::uniform_int_distribution<std::uint32_t> gOp(0, opNum - 1);

    // Create m examples randomly
    for (std::uint32_t i = 0; i < m; ++i) {

        std::uint32_t index = gOp(eng);
        ++infos[index].cnt;
        operations[index](getTimeCounterGenerator(index));
    }

    return {res, infos};
}

#define DEFINE_SETFUNC_FOR_STRING(val) \
    {PCC::PARAM, [&val] (const char * arg) { val = arg; return true; }}
#define DEFINE_SETFUNC_FOR_SIZET(val) \
    {PCC::PARAM, [&val] (const char * arg) { \
        try { \
            val = std::stoull(std::string(arg)); \
        } catch(const std::out_of_range & e) { \
            std::cout << "Parse arg " << #val << " Failed!!\n"; \
            return false; \
        } \
        return true; \
    }}

#define HELP_INFO_LINE(opt, info) opt " : " info "\n"

#ifdef _MSC_VER
constexpr const char kDefaultPythonPath[] = "python"; // Try find from PATH
#else
constexpr const char kDefaultPythonPath[] = "/usr/bin/python3";
#endif // _MSC_VER

int main (int argc, char * argv[]) {

    const char * help_info =
        "Options and arguments :\n"
        HELP_INFO_LINE("-h", "show help infomation")
        HELP_INFO_LINE("-P", "python3_path, default : \"/usr/bin/python3\"(for win: \"python\")")
        HELP_INFO_LINE("-p", "python filename to be generated, default : \"test_for_calcu.py\"")
        HELP_INFO_LINE("-l", "log filename, default : \"test_for_calcu.log\"")
        HELP_INFO_LINE("-e", "examples filename, default : \"test_for_calcu.examples\"")
        HELP_INFO_LINE("-s", "random seed, default : std::time(nullptr)")
        HELP_INFO_LINE("-n", "the number of big-numbers to be generated randomly, default : 10")
        HELP_INFO_LINE("-m", "the number of examples to be generated randomly, default : 50")
        HELP_INFO_LINE("-b", "minbytes of big-number, default : 1")
        HELP_INFO_LINE("-B", "minbytes of big-number, default : 16");

    // Command args
                                                               // -h : help
    std::string python3_path      = kDefaultPythonPath;        // -P
    std::string python_filename   = "test_for_calcu.py";       // -p
    std::string log_file          = "test_for_calcu.log";      // -l
    std::string examples_filename = "test_for_calcu.examples"; // -e
    std::uint32_t seed              = std::time(nullptr);        // -s
    std::uint32_t n(10), m(50), minbytes(1), maxbytes(16);       // -n -m -b -B

    // Parse cmd args
    using PCC = pg::util::ParseCmdConfig;
    std::unordered_map<char, PCC> cmdArgsConfig = {
        {'h', {PCC::OPTION, [help_info](const char*) { std::cout << help_info; return false; }}},
        {'P', DEFINE_SETFUNC_FOR_STRING(python3_path)},
        {'p', DEFINE_SETFUNC_FOR_STRING(python_filename)},
        {'l', DEFINE_SETFUNC_FOR_STRING(log_file)},
        {'e', DEFINE_SETFUNC_FOR_STRING(examples_filename)},
        {'s', DEFINE_SETFUNC_FOR_SIZET(seed)},
        {'n', DEFINE_SETFUNC_FOR_SIZET(n)},
        {'m', DEFINE_SETFUNC_FOR_SIZET(m)},
        {'b', DEFINE_SETFUNC_FOR_SIZET(minbytes)},
        {'B', DEFINE_SETFUNC_FOR_SIZET(maxbytes)}
    };
    pg::util::parseCmdSimply(argc, argv, cmdArgsConfig);

    std::string format = 
        "test_log_file = '{0}'\n"
        "log_file = open(test_log_file, 'a')\n"
        "test_count = {1}\n"
        "err_count = 0\n"
        "test_val = {2}\n"
        "err_val = []\n"
        "for e in test_val:\n"
        "    if eval(e) == False:\n"
        "        err_count += 1\n"
        "        err_val.append(e)\n"
        "print('Test Time : ', end='')\n"
        "print('{3}\\n', end='')\n"
        "print('Test Finished, err_count/test_count = ', end='')\n"
        "print(str(err_count), end='')\n"
        "print('/', end='')\n"
        "print(str(test_count))\n"
        "log_file.write('Test Time : ')\n"
        "log_file.write('{3}\\n')\n"
        "log_file.write('Test Finished, err_count/test_count = ')\n"
        "log_file.write(str(err_count))\n"
        "log_file.write('/')\n"
        "log_file.write(str(test_count))\n"
        "log_file.write('\\nerr-examples :\\n')\n"
        "for e in err_val:\n"
        "    log_file.write('\\t')\n"
        "    log_file.write(e)\n"
        "    log_file.write('\\n')\n"
        "log_file.write('\\n')\n"
        "log_file.close()\n";

    { // Gen code & Dump to file
        std::cout << pgfmt::format("Generating Examples(With Seed {0})\n", seed);
        auto [temp, infos] = generateExamples(seed, n, m, minbytes, maxbytes);
        std::cout << "Generate Examples Successfully\n";
        std::uint32_t totalCnt = 0;
        double totalMicroSec = 0;
        std::cout << "+================== Examples Running Infomation ===================+\n";
        for (const auto & info : infos) {
            std::cout << pgfmt::format("+ Run {1} Examples Of [{0}] In {2}ns\n", info.msg, info.cnt, info.nanosec);
            totalCnt += info.cnt;
            totalMicroSec += info.nanosec / 1000.f;
        }
        std::cout << pgfmt::format("+ Summary : Run {0} Examples In {1:.3} micro sec\n", totalCnt, totalMicroSec);
        std::cout << "+================== Examples Running Infomation ===================+\n";

        std::cout << "Updating Examples-file\n";
        std::ofstream efile(examples_filename, std::ios::app);

        time_t rawtime;
        struct tm * timeinfo;
        char buffer [128];
        time (&rawtime);
        timeinfo = localtime (&rawtime);
        strftime (buffer,sizeof(buffer),"%Y/%m/%d %H:%M:%S",timeinfo);   

        efile << "test_time = \'" << buffer << "\'\n"
              << "random_seed = " << seed << "\n"
              << "numbers_count = " << n << "\n"
              << "minbytes = " << minbytes << "\n"
              << "maxbytes = " << maxbytes << "\n"
              << "examples = " << pgfmt::format("{0}\n", temp);
        std::cout << "Update Examples-file Successfully\n";
        
        std::cout << "Generating Python Code\n";
        std::string python_code = pgfmt::format(format, log_file, (std::uint32_t)temp.size(), temp, buffer);
        std::ofstream pfile(python_filename);
        pfile << python_code;
        std::cout << "Generate Python Code Successfully\n";
    }

    // Exec Python script
    std::cout << "Running Python-Script\n";
    auto cmd = pgfmt::format("{0} {1}", python3_path, python_filename);
    if (std::system(cmd.c_str()) != 0)
        std::cout << "Exec Python-Script Failed!!\n";

    return 0;
}
