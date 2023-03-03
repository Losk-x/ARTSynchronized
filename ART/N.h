//
// Created by florian on 05.08.15.
//

#ifndef ARTVERSION1_ART_N_H
#define ARTVERSION1_ART_N_H
//#define ART_NOREADLOCK
//#define ART_NOWRITELOCK
#include <stdint.h>
#include <atomic>
#include <string.h>
#include "../Key.h"
#include "../Epoche.h"
#include <tbb/tbb.h>

using namespace ART;

using TID = uint64_t; //Losk?: 这个是类似typedef吗? 

namespace ART_unsynchronized {
/*
 * SynchronizedTree
 * LockCouplingTree
 * LockCheckFreeReadTree
 * UnsynchronizedTree
 */

    enum class NTypes : uint8_t {
        N4 = 0,
        N16 = 1,
        N48 = 2,
        N256 = 3
    };

    static constexpr uint32_t maxStoredPrefixLength = 10;

    using Prefix = uint8_t[maxStoredPrefixLength];

    class N {
    protected: //Losk?: protected的语义? 为什么用?
        N(NTypes type, const uint8_t *prefix, uint32_t prefixLength) {
            setType(type);
            setPrefix(prefix, prefixLength);
        }

        N(const N &) = delete;

        N(N &&) = delete; //Losk?: 给我带来了C++恐惧症,但是还是需要再查吧

        //Losk?: 似乎是用来做同步的? 但实际看setPrefix好像prefixCount=prefixLength
        // version 1, unlocked, not obsolete
        uint32_t prefixCount = 0;

        NTypes type;
    public:
        //Losk?: 这个count啥作用?
        uint8_t count = 0;
    protected:
        Prefix prefix;

        void setType(NTypes type);

    public:

        NTypes getType() const;

        uint32_t getCount() const;

        bool hasPrefix() const;

        const uint8_t *getPrefix() const;

        void setPrefix(const uint8_t *prefix, uint32_t length);

        void addPrefixBefore(N *node, uint8_t key);

        uint32_t getPrefixLength() const;

        // Static Methods
        
        static N *getChild(const uint8_t k, N *node);

        //Losk?: 不太懂这个奇怪的函数, 为啥还有keyParent? keyParent是用来定位parentNode中的位置吗还是啥?
        //Losk: InsertA即insert+grow, 如果满了就grow. 所以需要parentNode以及当前node在parentNode的keyParent.
        static void insertA(N *node, N *parentNode, uint8_t keyParent, uint8_t key, N *val);

        //Losk: change的语义是, 更改node中对应key的slot, 赋值其为val.
        static void change(N *node, uint8_t key, N *val);

        static void removeA(N *node, uint8_t key, N *parentNode, uint8_t keyParent);

        static TID getLeaf(const N *n);

        static bool isLeaf(const N *n);

        static N *setLeaf(TID tid);

        static N *getAnyChild(const N *n);

        static TID getAnyChildTid(N *n);

        static void deleteChildren(N *node);

        static void deleteNode(N *node);

        static std::tuple<N *, uint8_t> getSecondChild(N *node, const uint8_t k);

        template<typename curN, typename biggerN>
        static void insertGrow(curN *n, N *parentNode, uint8_t keyParent, uint8_t key, N *val);

        template<typename curN, typename smallerN>
        static void removeAndShrink(curN *n, N *parentNode, uint8_t keyParent, uint8_t key);

        static void getChildren(const N *node, uint8_t start, uint8_t end, std::tuple<uint8_t, N *> children[],
                                uint32_t &childrenCount);

        static long size(N *node);
    };

    class N4 : public N {
    public:
        //TODO
        //atomic??
        uint8_t keys[4];
        N *children[4] = {nullptr, nullptr, nullptr, nullptr};

    public:
        N4(const uint8_t *prefix, uint32_t prefixLength) : N(NTypes::N4, prefix,
                                                                             prefixLength) { }

        bool insert(uint8_t key, N *n);

        template<class NODE>
        void copyTo(NODE *n) const;

        void change(uint8_t key, N *val);

        N *getChild(const uint8_t k) const;

        bool remove(uint8_t k, bool force);

        N *getAnyChild() const;

        std::tuple<N *, uint8_t> getSecondChild(const uint8_t key) const;

        void deleteChildren();

        void getChildren(uint8_t start, uint8_t end, std::tuple<uint8_t, N *> *&children,
                         uint32_t &childrenCount) const;

        long size();
    };

    class N16 : public N {
    public:
        uint8_t keys[16];
        N *children[16];

        static uint8_t flipSign(uint8_t keyByte) {
            // Flip the sign bit, enables signed SSE comparison of unsigned values, used by Node16
            return keyByte ^ 128;
        }

        static inline unsigned ctz(uint16_t x) {
            // Count trailing zeros, only defined for x>0
#ifdef __GNUC__
            return __builtin_ctz(x);
#else
            // Adapted from Hacker's Delight
   unsigned n=1;
   if ((x&0xFF)==0) {n+=8; x=x>>8;}
   if ((x&0x0F)==0) {n+=4; x=x>>4;}
   if ((x&0x03)==0) {n+=2; x=x>>2;}
   return n-(x&1);
#endif
        }

        N *const *getChildPos(const uint8_t k) const;

    public:
        N16(const uint8_t *prefix, uint32_t prefixLength) : N(NTypes::N16, prefix,
                                                                              prefixLength) {
            memset(keys, 0, sizeof(keys));
            memset(children, 0, sizeof(children));
        }

        bool insert(uint8_t key, N *n);

        template<class NODE>
        void copyTo(NODE *n) const;

        void change(uint8_t key, N *val);

        N *getChild(const uint8_t k) const;

        bool remove(uint8_t k, bool force);

        N *getAnyChild() const;

        void deleteChildren();

        void getChildren(uint8_t start, uint8_t end, std::tuple<uint8_t, N *> *&children,
                         uint32_t &childrenCount) const;

        long size();
    };

    class N48 : public N {
        uint8_t childIndex[256];
        N *children[48];
    public:
        static const uint8_t emptyMarker = 48;

        N48(const uint8_t *prefix, uint32_t prefixLength) : N(NTypes::N48, prefix,
                                                                              prefixLength) {
            memset(childIndex, emptyMarker, sizeof(childIndex));
            memset(children, 0, sizeof(children));
        }

        bool insert(uint8_t key, N *n);

        template<class NODE>
        void copyTo(NODE *n) const;

        void change(uint8_t key, N *val);

        N *getChild(const uint8_t k) const;

        bool remove(uint8_t k, bool force);

        N *getAnyChild() const;

        void deleteChildren();

        void getChildren(uint8_t start, uint8_t end, std::tuple<uint8_t, N *> *&children,
                         uint32_t &childrenCount) const;

        long size();
    };

    class N256 : public N {
        N *children[256];

    public:
        N256(const uint8_t *prefix, uint32_t prefixLength) : N(NTypes::N256, prefix,
                                                                               prefixLength) {
            memset(children, '\0', sizeof(children));
        }

        bool insert(uint8_t key, N *val);

        template<class NODE>
        void copyTo(NODE *n) const;

        void change(uint8_t key, N *n);

        N *getChild(const uint8_t k) const;

        bool remove(uint8_t k, bool force);

        N *getAnyChild() const;

        void deleteChildren();

        void getChildren(uint8_t start, uint8_t end, std::tuple<uint8_t, N *> *&children,
                         uint32_t &childrenCount) const;

        long size();
    };
}
#endif //ARTVERSION1_ARTVERSION_H
