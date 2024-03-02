#pragma once

#include <string>

#define MAKE_MEMBER(a) a,

#define FOR_ALL_ACCESS_PATTERNS(apply) \
    apply(ALWAYS_THE_SAME) \
    apply(FULLY_RANDOM) \
    apply(RANDOM_PATTERN) \
    apply(COHERENT_REGION) \
    apply(COHERENT_REGION_UNFAVOURABLE_TRAVERSAL) \
    apply(COUNT)
    
enum class AccessPattern {
    FOR_ALL_ACCESS_PATTERNS(MAKE_MEMBER)
};

AccessPattern operator++(AccessPattern& value) 
{
    return value = static_cast<AccessPattern>(static_cast<int>(value) + 1);
}

const std::string toString(AccessPattern value)
{
    switch (value)
    {
        #define CASE_STRINGIFY(a) case AccessPattern::a: return #a;
        FOR_ALL_ACCESS_PATTERNS(CASE_STRINGIFY)
        default: return "Error";
        #undef CASE_STRINGIFY
    }
}

#define FOR_ALL_CACHE_SHAPES(apply) \
    apply(SQUARE) \
    apply(LINE) \
    apply(COUNT)

enum class CacheShape {
    FOR_ALL_CACHE_SHAPES(MAKE_MEMBER)
};

CacheShape operator++(CacheShape& value) 
{
    return value = static_cast<CacheShape>(static_cast<int>(value) + 1);
}

const std::string toString(CacheShape value)
{
    switch (value)
    {
        #define CASE_STRINGIFY(a) case CacheShape::a: return #a;
        FOR_ALL_CACHE_SHAPES(CASE_STRINGIFY)
        default: return "Error";
        #undef CASE_STRINGIFY
    }
}

#define FOR_ALL_LAYOUTS(apply) \
    apply(ALIGNED) \
    apply(VERTICAL_OFFSET) \
    apply(HORIZONTAL_OFFSET) \
    apply(OFFSET) \
    apply(COUNT)

enum class Layout {
    FOR_ALL_LAYOUTS(MAKE_MEMBER)
};

Layout operator++(Layout& value) 
{
    return value = static_cast<Layout>(static_cast<int>(value) + 1);
}

const std::string toString(Layout value)
{
    switch (value)
    {
        #define CASE_STRINGIFY(a) case Layout::a: return #a;
        FOR_ALL_LAYOUTS(CASE_STRINGIFY)
        default: return "Error";
        #undef CASE_STRINGIFY
    }
}

#define FOR_ALL_CACHE_CHUNK_SIZES(apply) \
    apply(SMALL) \
    apply(MEDIUM) \
    apply(LARGE) \
    apply(COUNT) 

enum class CacheChunkSize {
    FOR_ALL_CACHE_CHUNK_SIZES(MAKE_MEMBER)
};

CacheChunkSize operator++(CacheChunkSize& value) 
{
    return value = static_cast<CacheChunkSize>(static_cast<int>(value) + 1);
}

const std::string toString(CacheChunkSize value)
{
    switch (value)
    {
        #define CASE_STRINGIFY(a) case CacheChunkSize::a: return #a;
        FOR_ALL_CACHE_CHUNK_SIZES(CASE_STRINGIFY)
        default: return "Error";
        #undef CASE_STRINGIFY
    }
}

#define FOR_ALL_CACHE_LIMITS(apply) \
    apply(TOO_LOW_FACTOR_0_1) \
    apply(TOO_LOW_FACTOR_0_5) \
    apply(TOO_LOW_FACTOR_0_9) \
    apply(ENOUGH_FACTOR_1) \
    apply(ENOUGH_FACTOR_5) \
    apply(COUNT) 

enum class CacheLimit {
    FOR_ALL_CACHE_LIMITS(MAKE_MEMBER)
};

const std::string toString(CacheLimit value)
{
    switch (value)
    {
        #define CASE_STRINGIFY(a) case CacheLimit::a: return #a;
        FOR_ALL_CACHE_LIMITS(CASE_STRINGIFY)
        default: return "Error";
        #undef CASE_STRINGIFY
    }
}

CacheLimit operator++(CacheLimit& value) 
{
    return value = static_cast<CacheLimit>(static_cast<int>(value) + 1);
}

#define FOR_ALL_EVICTION_STRATEGIES(apply) \
    apply(FIFO) \
    apply(LRU) \
    apply(COUNT)

enum class EvictionStrategy {
    FOR_ALL_EVICTION_STRATEGIES(MAKE_MEMBER)
};

const std::string toString(EvictionStrategy value)
{
    switch (value)
    {
        #define CASE_STRINGIFY(a) case EvictionStrategy::a: return #a;
        FOR_ALL_EVICTION_STRATEGIES(CASE_STRINGIFY)
        default: return "Error";
        #undef CASE_STRINGIFY
    }
}

EvictionStrategy operator++(EvictionStrategy& value) 
{
    return value = static_cast<EvictionStrategy>(static_cast<int>(value) + 1);
}