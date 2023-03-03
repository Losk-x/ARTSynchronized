//
// Created by florian on 18.11.15.
//

#ifndef ARTVERSION1_TREE_H
#define ARTVERSION1_TREE_H
#include "N.h"

using namespace ART; //Losk?: 这个命名空间又是啥, 命名空间用法有点忘了，改天回顾一下

namespace ART_unsynchronized { //Losk: 似乎是在ART空间中实现了通用的Node, 然后再分别实现同步.

    class Tree {
    public:
        using LoadKeyFunction = void (*)(TID tid, Key &key); //Losk?: 还不太清楚啥用法. 可能得看看example.cpp

    private:
        N *const root; //Losk?: 晕, 常量+指针有点忘了, 这个的读法应该是常量指针 (指针值不能变,不能乱指的意思吧), 初始化估计在构造函数

        TID checkKey(const TID tid, const Key &k) const; //Losk?: 还不太清楚是check啥和啥

        LoadKeyFunction loadKey;

        enum class CheckPrefixResult : uint8_t { //Losk?: C++的枚举类型,之前没用过,得查一下
            Match,
            NoMatch,
            OptimisticMatch
        };

        enum class CheckPrefixPessimisticResult : uint8_t {
            Match,
            NoMatch,
        };

        enum class PCCompareResults : uint8_t { //Losk?: 不太懂PC是啥意思
            Smaller,
            Equal,
            Bigger,
        };
        enum class PCEqualsResults : uint8_t { //Losk?: PCEquals与上面的PCCompare的差别?
            StartMatch,
            BothMatch,
            Contained,
            NoMatch,
        };
        static CheckPrefixResult checkPrefix(N* n, const Key &k, uint32_t &level);

        static CheckPrefixPessimisticResult checkPrefixPessimistic(N *n, const Key &k, uint32_t &level,
                                                                   uint8_t &nonMatchingKey,
                                                                   Prefix &nonMatchingPrefix,
                                                                   LoadKeyFunction loadKey);

        static PCCompareResults checkPrefixCompare(N* n, const Key &k, uint32_t &level, LoadKeyFunction loadKey);

        static PCEqualsResults checkPrefixEquals(N* n, uint32_t &level, const Key &start, const Key &end, LoadKeyFunction loadKey);

    public:

        Tree(LoadKeyFunction loadKey);

        Tree(const Tree &) = delete; //Losk?: 这个语法是什么来着? 好像是禁止自动生成拷贝构造

        Tree(Tree &&t) : root(t.root), loadKey(t.loadKey) { }  //Losk?: Tree&&是啥来着? 是啥移动拷贝吗?

        ~Tree();

        TID lookup(const Key &k) const;

        bool lookupRange(const Key &start, const Key &end, Key &continueKey, TID result[], std::size_t resultLen,
                         std::size_t &resultCount) const; //Losk?: continueKey不太懂有啥用, 包括resultLen(是说result[]数组的最大长度吗)

        void insert(const Key &k, TID tid); //Losk?: 似乎说明Value=TID

        bool update(const Key &k, TID tid);

        void remove(const Key &k);

        long size();
    };
}
#endif //ARTVERSION1_SYNCHRONIZEDTREE_H
