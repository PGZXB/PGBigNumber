//
// Created by PGZXB on 2021/4/21.
//
#ifndef PGDEBUG_H
#define PGDEBUG_H

#define PGZXB_PASS (void(0))
#define PGZXB_UNUSED(val) (void(val))

#if defined(PGZXB_DEBUG)

#include <bitset>
#include <iostream>
#define PGZXB_DEBUG_INFO_HEADER "[DEBUG]In " << __FILE__ << " in " << __func__ << " at " << __LINE__ << " : "
#define PGZXB_DEBUG_Print(msg) std::cout << PGZXB_DEBUG_INFO_HEADER << (msg) << "\n"
#define PGZXB_DEBUG_PrintVar(val) std::cout << PGZXB_DEBUG_INFO_HEADER << #val" : " << (val) << "\n"
#define PGZBX_DEBUG_PrintBin(val) std::cout << PGZXB_DEBUG_INFO_HEADER << #val" : " << std::bitset<sizeof(val) * 8>(val) << "\n"
#define PGZXB_DEBUG_CallFunc(func) PGZXB_DEBUG_PrintVar(func)

#else

#define PGZXB_DEBUG_INFO_HEADER ""
#define PGZXB_DEBUG_PrintVar(val) PGZXB_UNUSED(val)
#define PGZBX_DEBUG_PrintBin(val) PGZXB_UNUSED(val)
#define PGZXB_DEBUG_CallFunc(func) PGZXB_UNUSED(val)

#endif

#endif // PGDEBUG_H

#ifndef __TOSTRING_UTIL_H__
#define __TOSTRING_UTIL_H__

#include <cstring>
#include <string>
#include <type_traits>
#include <iterator>
#include <memory>

#include <typeinfo>

namespace pg::util::stringUtil::__IN_fmtUtil {
#if __cplusplus >= 201703L
    using std::void_t;
#else
    template<typename...> using void_t = void;
#endif

    template <typename _Type, typename _Iter = void_t<>>
    struct has_iterator : public std::false_type { };

    template <typename _Type>
    struct has_iterator<_Type, void_t<typename _Type::iterator>> : public std::true_type { };

    // function-delarations
    template <typename _First, typename _Second>
    std::string transToString(const std::pair<_First, _Second> & ele, const std::string &);

    template<typename _Type>
    std::enable_if_t<has_iterator<_Type>::value, std::string> transToString(const _Type& ele, const std::string & limit);

    template<typename _Type>
    std::enable_if_t<!has_iterator<_Type>::value, std::string> transToString(const _Type& ele, const std::string &);

    // function-definitions
    inline std::string transToString(const int & ele, const std::string & limit) {
        char buf[20] = { 0 };
        char fmt[10] = { 0 };
        sprintf(fmt, "%%%sd", limit.c_str());
        int len = sprintf(buf, fmt, ele);
        return std::string().assign(buf, len);
    }

    inline std::string transToString(const bool & ele, const std::string &) {
        return ele ? "true" : "false";
    }

    inline std::string transToString(const char& ele, const std::string &) {
        return std::string() + ele;
    }

    inline std::string transToString(const short & ele, const std::string & limit) {
        char buf[20] = { 0 };
        char fmt[10] = { 0 };
        sprintf(fmt, "%%%shd", limit.c_str());
        int len = sprintf(buf, fmt, ele);
        return std::string().assign(buf, len);
    }

    inline std::string transToString(const long & ele, const std::string & limit) {
        char buf[40] = { 0 };
        char fmt[20] = { 0 };
        sprintf(fmt, "%%%sld", limit.c_str());
        int len = sprintf(buf, fmt, ele);
        return std::string().assign(buf, len);
    }

    inline std::string transToString(const long long & ele, const std::string & limit) {
        char buf[40] = { 0 };
        char fmt[20] = { 0 };
        sprintf(fmt, "%%%slld", limit.c_str());
        int len = sprintf(buf, fmt, ele);
        return std::string().assign(buf, len);
    }

    inline std::string transToString(const float & ele, const std::string & limit) {
        char buf[40] = { 0 };
        char fmt[20] = { 0 };
        sprintf(fmt, "%%%sf", limit.c_str());
        int len = sprintf(buf, fmt, ele);
        return std::string().assign(buf, len);
    }

    inline std::string transToString(const double & ele, const std::string & limit) {
        char buf[40] = { 0 };
        char fmt[20] = { 0 };
        sprintf(fmt, "%%%slf", limit.c_str());
        int len = sprintf(buf, fmt, ele);
        return std::string().assign(buf, len);
    }

    inline std::string transToString(const long double & ele, const std::string & limit) {
        char buf[50] = { 0 };
        char fmt[30] = { 0 };
        sprintf(fmt, "%%%sllf", limit.c_str());
        int len = sprintf(buf, fmt, ele);
        return std::string().assign(buf, len);
    }

    inline std::string transToString(const std::string & ele, const std::string & limit) {
        struct HeapCharArrayWrapper {  // temp-tool
            HeapCharArrayWrapper(char * arr) : ptr(arr) { }
            ~HeapCharArrayWrapper() { if (ptr != nullptr) delete [] ptr; }
            char * ptr = nullptr;
        };

        HeapCharArrayWrapper buf(new char[ele.size() << 1]);  // temp-processing ==> Buffer
        char fmt[50] = { 0 };

        sprintf(fmt, "%%%ss", limit.c_str());
        sprintf(buf.ptr, fmt, ele.c_str());
        return std::string(buf.ptr);
    }

    inline std::string transToString(const char * ele, const std::string & limit) {
        struct HeapCharArrayWrapper {  // temp-tool
            HeapCharArrayWrapper(char * arr) : ptr(arr) { }
            ~HeapCharArrayWrapper() { if (ptr != nullptr) delete [] ptr; }
            char * ptr = nullptr;
        };

        const size_t len = std::strlen(ele);
        HeapCharArrayWrapper buf(new char[len << 1]);  // temp-processing ==> Buffer
        char fmt[50] = { 0 };

        sprintf(fmt, "%%%ss", limit.c_str());
        sprintf(buf.ptr, fmt, ele);
        return std::string(buf.ptr);
    }

    template <typename _First, typename _Second>
    std::string transToString(const std::pair<_First, _Second> & ele, const std::string &) {
        const std::string NULL_STRING;
        return std::string()
            .append(transToString(ele.first, NULL_STRING))
            .append(" : ")
            .append(transToString(ele.second, NULL_STRING));
    }

    // string, const char*
    template<typename _Type>
    std::enable_if_t<has_iterator<_Type>::value, std::string> transToString(const _Type& ele, const std::string & limit) {
        typedef typename _Type::const_iterator Iter;
        
        const std::string NULL_STRING;
        std::string res("[");

        for (
            Iter iter = ele.begin(), end = ele.end();
            iter != end;
            ++iter
        )
            res.append( transToString(*iter, NULL_STRING) ).append(", ");  // useless second-param

        res.pop_back();
        res.back() = ']';
        return res;
    }

    template<typename _Type>
    std::enable_if_t<!has_iterator<_Type>::value, std::string> transToString(const _Type& ele, const std::string &) {  // temp-function to show
        return "<default-string>";
    }

} // namespace pg::util::stringUtl::__IN_fmtUtil


#endif

#ifndef __FMT_UTILS_H__
#define __FMT_UTILS_H__

#include <vector>
#include <string>
#include <algorithm>

namespace pg::util::stringUtil {
    namespace __IN_fmtUtil {
        // function-delaration(s)
        std::vector<std::pair<std::string::const_iterator, std::string::const_iterator>> 
            parseBracket(
                const std::string & str, const char * leftBracket, const char * rightBracket);
        
        template<typename..._Args>
        std::vector<std::string> parseArgs(
            const std::vector<std::string>& limits, const _Args&... args);

        template<typename _First, typename..._Args>
        void __IN_parseArgs(
            std::vector<std::string> & res, 
            const std::vector<std::string>& limits, 
            std::vector<std::string>::const_iterator & limStrIter,
            const _First& fArg, const _Args&... args);

        void __IN_parseArgs(
            std::vector<std::string> & res, 
            const std::vector<std::string> & limits, 
            std::vector<std::string>::const_iterator & limStrIter);
        
        // function-definitions
        template<typename..._Args>
        std::vector<std::string> parseArgs(
            const std::vector<std::string>& limits, const _Args&... args) {
            std::vector<std::string> res;
            std::vector<std::string>::const_iterator iter = limits.begin();
            __IN_parseArgs(res, limits, iter, args...);

            return res;
        }

        template<typename _First, typename..._Args>
        void __IN_parseArgs(
            std::vector<std::string> & res, 
            const std::vector<std::string>& limits, 
            std::vector<std::string>::const_iterator & limStrIter,
            const _First& fArg, const _Args&... args
            ) {
            res.push_back(transToString(fArg, *limStrIter));
            ++limStrIter;
            if (limStrIter == limits.end()) return;
            __IN_parseArgs(res, limits, limStrIter, args...);
        }

        inline void __IN_parseArgs(
            std::vector<std::string> & res,
            const std::vector<std::string>& limits,
            std::vector<std::string>::const_iterator & limStrIter
            ) { }

        inline std::vector<std::pair<std::string::const_iterator, std::string::const_iterator>> 
            parseBracket(
                const std::string & str, const char * leftBracket, const char * rightBracket) {
                
                std::vector<std::pair<std::string::const_iterator, std::string::const_iterator>> res;

                size_t pos = 0, npos = std::string::npos;
                std::string::const_iterator begin = str.begin();
                while (true) {
                    size_t beginPos = str.find(leftBracket, pos);
                    if (beginPos == npos) break;
                    size_t endPos = str.find(rightBracket, beginPos);
                    if (endPos == npos) break;
                    res.push_back({begin + beginPos + 1, begin + endPos});
                    pos = endPos;
                }
                return res;

            }

        template <typename _Res>
        void pushStringIntoBracket(
            _Res & res,
            const std::string & fmt,
            const std::vector<std::string> & contents,
            const std::vector<std::pair<std::string::const_iterator, std::string::const_iterator>> & ranges,
            const std::vector<int> & nos) {
            res.append(fmt.begin(), ranges.front().first - 1);
            ssize_t cSize = contents.size();
            ssize_t idx = 0;
            for (ssize_t i = 0; i < static_cast<ssize_t>(nos.size()) - 1; ++i) {
                idx = nos.at(i);
                if (idx < cSize) res.append(contents.at(idx));
                res.append(ranges.at(i).second + 1, ranges.at(i + 1).first - 1);
            }
            if ((idx = nos.back()) < cSize) res.append(contents.at(idx));
            res.append(ranges.back().second + 1, fmt.end());
        }

    } // namespace __IN_fmtUtil


    template<typename..._Args>
    std::string format(const std::string & fmt, _Args&& ...args) {
        typedef std::vector<std::pair<std::string::const_iterator, std::string::const_iterator>>::iterator Iter;
        
        std::vector<std::pair<std::string::const_iterator, std::string::const_iterator>>
            contentRangeInBracket = __IN_fmtUtil::parseBracket(fmt, "{", "}");
        if (contentRangeInBracket.empty()) return fmt;

        std::vector<std::string> limits;
        std::vector<int> nos;
        
        for (
            Iter iter = contentRangeInBracket.begin(), end = contentRangeInBracket.end();
            iter != end; ++iter) {
            
            std::string::const_iterator partiIter =  std::find(iter->first, iter->second, ':');
            nos.push_back(std::atoi(fmt.c_str() + (iter->first - fmt.begin())));
            if (partiIter != iter->second)
                limits.push_back(std::string().assign(partiIter + 1, iter->second));
            else limits.emplace_back();
        }
        
        std::vector<std::string> contents =  __IN_fmtUtil::parseArgs(limits, args...);

        std::string res; res.append(fmt.begin(), contentRangeInBracket.front().first - 1);
        ssize_t cSize = contents.size();
        ssize_t idx = 0;
        for (ssize_t i = 0; i < static_cast<ssize_t>(nos.size()) - 1; ++i) {
            idx = nos.at(i);
            if (idx < cSize) res.append(contents.at(idx));
            res.append(contentRangeInBracket.at(i).second + 1, contentRangeInBracket.at(i + 1).first - 1);
        }
        if ((idx = nos.back()) < cSize) res.append(contents.at(idx));
        res.append(contentRangeInBracket.back().second + 1, fmt.end());

        return res;
    }

    template<typename _Res, typename..._Args>
    void formatAppend(_Res & res, const std::string & fmt, _Args&& ...args) {
        typedef std::vector<std::pair<std::string::const_iterator, std::string::const_iterator>>::iterator Iter;
        
        std::vector<std::pair<std::string::const_iterator, std::string::const_iterator>>
            contentRangeInBracket = __IN_fmtUtil::parseBracket(fmt, "{", "}");
        if (contentRangeInBracket.empty()) { res = fmt; return; }

        std::vector<std::string> limits;
        std::vector<int> nos;
        
        for (
            Iter iter = contentRangeInBracket.begin(), end = contentRangeInBracket.end();
            iter != end; ++iter) {
            
            std::string::const_iterator partiIter =  std::find(iter->first, iter->second, ':');
            nos.push_back(std::atoi(fmt.c_str() + (iter->first - fmt.begin())));
            if (partiIter != iter->second)
                limits.push_back(std::string().assign(partiIter + 1, iter->second));
            else limits.emplace_back();
        }
        
        std::vector<std::string> contents =  __IN_fmtUtil::parseArgs(limits, args...);

        __IN_fmtUtil::pushStringIntoBracket(res, fmt, contents, contentRangeInBracket, nos);
    }

} // namespace pg::util::stringUtil

namespace pgfmt {
    using ::pg::util::stringUtil::format;
    using ::pg::util::stringUtil::formatAppend;
}

#endif
