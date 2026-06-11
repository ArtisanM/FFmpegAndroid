
#ifndef NEXT_DICTIONARY_H
#define NEXT_DICTIONARY_H

#include <vector>

enum ValueType {
    VALUE_TYPE_UNKNOWN = 0,
    VALUE_TYPE_INT32   = 1,
    VALUE_TYPE_INT64   = 2,
    VALUE_TYPE_STRING  = 3
};

class NextDictionary {
public:
    NextDictionary() = default;

    virtual ~NextDictionary();

    void clear();

    void setInt64(const char *name, int64_t value);

    bool findInt64(const char *name, int64_t *value) const;

    int64_t getInt64(const char *name, int64_t defaultValue) const;

    void setString(const char *name, const std::string &s);

    std::string getString(const char *name, std::string *defaultValue) const;

    size_t getSize() const;

    const char *getEntryNameAt(size_t index, ValueType *type) const;

protected:
    struct Item {
        // TODO: memory align
//        union {
            int64_t val_int64;
            std::string *val_string;
//        } u;
        const char *item_name;
        size_t name_len;
        ValueType type;

        void setName(const char *name, size_t len);
    };

    std::vector<Item *> m_items;

    Item *allocateItem(const char *name);

    static void freeItemValue(Item *item);

    const Item *findItem(const char *name, ValueType type) const;

    size_t findItemIndex(const char *name, size_t len) const;
};

#endif
