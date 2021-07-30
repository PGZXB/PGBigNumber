// 利用Python3自带的大整数计算功能对pgbn::BigIntegerImpl的加减功能进行测试
// 本测试要求在Linux上进行

#if !defined(linux) && !defined(__GNUC__)
    #error "The Test Can Only Run On Linux!"
#endif

#include "../include/PGBigNumber/pgdebug.h" // format
#include "../src/BigIntegerImpl.h"

#include <iostream>
#include <fstream>
#include <random>
#include <unordered_map>
#include <ctime>
#include <sys/types.h>
#include <unistd.h>
#include <getopt.h>

/**
 * 思路 : 先随机构造N个BigIntegerImpl, 随机进行M次计算, 每次计算随机选取两个BigIntegerImpl
 * 随机进行加减法, 将每次计算的字符串形式记下(如 (0x1) + (0x2) == (0x3)), 
 * 生成test_for_calcu.py文件, 利用exec函数执行.py文件, .py格式如下
 * 
 * (缩进均为4个空格)
 * 
 * test_for_calcu.py :
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

std::vector<std::string> generateExamples(std::size_t randSeed, std::size_t n, std::size_t m, std::size_t minbytes, std::size_t maxbytes) {
    struct AutoDeleter {
        ~AutoDeleter() { free(ptr); }
        void * ptr = nullptr;
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

    { // 随机产生n个大整数Impl
        AutoDeleter deleter;

        std::uniform_int_distribution<std::size_t> gLen(minbytes, maxbytes);
        std::uniform_int_distribution gByte(0, 255);

        pgbn::Byte * bytes = (pgbn::Byte *)std::malloc(maxbytes);
        deleter.ptr = bytes;

        for (std::size_t i = 0; i < n; ++i) {
            // 随机确定长度[minbytes, maxbytes]
            std::size_t len = gLen(eng);
            // 随机设置各个byte的值(最高位的byte即确定了大整数的正负)
            for (std::size_t j = 0; j < len; ++j) {
                bytes[j] = gByte(eng);
            }
            // 构造大整数
            auto & temp = numbers.emplace_back(pgbn::BigIntegerImpl(bytes, len), "");
            // 缓存大整数的Pyhton字符串形式
            temp.s = strToPythonHex(temp.b.toString(16));
        }
    }

    std::uniform_int_distribution<std::size_t> gPair(0, n - 1);
    std::uniform_int_distribution<std::uint64_t> gU64(0, 10000);

#define DEFINE_RAND_GEN_BINARY_OPERATION(op, opN) \
    [&numbers, &eng, &gPair, &res] () { \
        auto & a = numbers[gPair(eng)]; \
        auto & b = numbers[gPair(eng)]; \
        pgbn::BigIntegerImpl copy(a.b); \
        copy.opN##Assign(b.b); \
        res.emplace_back(a.s) \
            .append(" " op " ") \
            .append(b.s) \
            .append(" == ") \
            .append(strToPythonHex(copy.toString(16))); \
    }
#define DEFINE_RAND_GEN_SHIFT_OPERATION(op, opN) \
    [&numbers, &eng, &gPair, &gU64, &res] () { \
        auto & a = numbers[gPair(eng)]; \
        auto u64 = gU64(eng); \
        pgbn::BigIntegerImpl copy(a.b); \
        copy.opN##Assign(u64); \
        res.emplace_back(a.s) \
            .append(" " op " ") \
            .append(std::to_string(u64)) \
            .append(" == ") \
            .append(strToPythonHex(copy.toString(16))); \
    }
    std::function<void()> operations[] = {
        DEFINE_RAND_GEN_BINARY_OPERATION("+", add),
        DEFINE_RAND_GEN_BINARY_OPERATION("-", sub),
        DEFINE_RAND_GEN_BINARY_OPERATION("*", mul),
        // DEFINE_BINARY_OPERATION("/", div),
        DEFINE_RAND_GEN_SHIFT_OPERATION("<<", shiftLeft),
        DEFINE_RAND_GEN_SHIFT_OPERATION(">>", shiftRight)
    };
    const std::size_t opNum = sizeof(operations) / sizeof(*operations);

    std::uniform_int_distribution<std::size_t> gOp(0, opNum - 1);

    // 随机构造m个计算
    for (std::size_t i = 0; i < m; ++i) {
        operations[gOp(eng)]();
    }

    return res;
}

#define DEFINE_SETFUNC_FOR_STRING(val) \
    [&val] (const char * arg) { val = arg; return true; }
#define DEFINE_SETFUNC_FOR_SIZET(val) \
    [&val] (const char * arg) { \
        try { \
            val = std::stoull(std::string(arg)); \
        } catch(const std::out_of_range & e) { \
            std::cout << "Parse arg " << #val << " Failed!!\n"; \
            return false; \
        } \
        return true; \
    }

#define HELP_INFO_LINE(opt, info) opt " : " info "\n"

#define GETOPT() ::getopt(argc, argv, "hP:p:l:e:s:n:m:b:B:")

int main (int argc, char * argv[]) {

    const char * help_info =
        "Options and arguments :\n"
        HELP_INFO_LINE("-h", "show help infomation")
        HELP_INFO_LINE("-P", "python3_path, default : \"/usr/bin/python3\"")
        HELP_INFO_LINE("-p", "python filename to be generated, default : \"test_for_calcu.py\"")
        HELP_INFO_LINE("-l", "log filename, default : \"test_for_calcu.log\"")
        HELP_INFO_LINE("-e", "examples filename, default : \"test_for_calcu.examples\"")
        HELP_INFO_LINE("-s", "random seed, default : std::time(nullptr)")
        HELP_INFO_LINE("-n", "the number of big-numbers to be generated randomly, default : 10")
        HELP_INFO_LINE("-m", "the number of examples to be generated randomly, default : 50")
        HELP_INFO_LINE("-b", "minbytes of big-number, default : 1")
        HELP_INFO_LINE("-B", "minbytes of big-number, default : 16");

    // args
                                                   // -h : help
    std::string python3_path = "/usr/bin/python3"; // -P
    std::string python_filename = "test_for_calcu.py"; // -p
    std::string log_file = "test_for_calcu.log"; // -l
    std::string examples_filename = "test_for_calcu.examples"; // -e
    std::size_t seed = std::time(nullptr); // -s
    std::size_t n(10), m(50), minbytes(1), maxbytes(16); // -n -m -b -B

    std::unordered_map<char, std::function<bool(const char *)>> helperMap = {
        {'h', [help_info] (const char *) { std::cout << help_info; return false; }},
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

    // 设置命令行参数
    int opt_ret = 0;
    while ((opt_ret = GETOPT()) != -1) {
        if (opt_ret == '?') {
            std::cout << "args-list invalid\n";
            return 0;
        } else {
            if (!helperMap[(char)opt_ret](::optarg)) return 0;
        }
    }

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

    { // 生成的code并写入文件
        std::cout << "Generating Examples\n";
        std::vector<std::string> temp = generateExamples(seed, n, m, minbytes, maxbytes);
        std::cout << "Generate Examples Successfully\n";

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
        std::string python_code = pgfmt::format(format, log_file, temp.size(), temp, buffer);
        std::ofstream pfile(python_filename);
        pfile << python_code;
        std::cout << "Generate Python Code Successfully\n";
    }

    // 执行python脚本
    std::cout << "Run Python-Script\n";
    ::execl(python3_path.c_str(), "python3", python_filename.c_str(), NULL);
    std::cout << "Exec Python-Script Failed!!\n";

    return 0;
}