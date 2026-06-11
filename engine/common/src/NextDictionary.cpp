/**
 * Note: operation of dictionary
 * Date: 2025/12/2
 * Author: frank
 */

#include "NextDictionary.h"

#include <string>

NextDictionary::~NextDictionary() { clear(); }

void NextDictionary::clear() {
    for (auto item: m_items) {
        if (item) {
            if (item->item_name) {
                delete[] item->item_name;
                item->item_name = nullptr;
            }
            freeItemValue(item);
            delete item;
        }
    }
    m_items.clear();
}

void NextDictionary::setInt64(const char *name, int64_t value) {
    Item *item = allocateItem(name);
    if (item) {
        item->type = VALUE_TYPE_INT64;
        item->val_int64 = value;
    }
}

bool NextDictionary::findInt64(const char *name, int64_t *value) const {
    const Item *item = findItem(name, VALUE_TYPE_INT64);
        if (item) {
            *value = item->val_int64;
            return true;
        }
    return false;
}

int64_t NextDictionary::getInt64(const char *name, int64_t defaultValue) const {
    const Item *item = findItem(name, VALUE_TYPE_INT64);
    if (item) {
        return item->val_int64;
    }
    return defaultValue;
}


void NextDictionary::Item::setName(const char *name, size_t len) {
    name_len = len;
    item_name = new char[len + 1];
    memcpy((void *) item_name, name, len + 1);
}

NextDictionary::Item *NextDictionary::allocateItem(const char *name) {
    size_t len = strlen(name);
    size_t i = findItemIndex(name, len);
    Item *item;

    if (i < m_items.size()) {
        item = m_items[i];
        freeItemValue(item);
    } else {
        item = new Item();
        if (!item) {
            return item;
        }
        item->setName(name, len);
        m_items.emplace_back(item);
    }

    return item;
}

void NextDictionary::freeItemValue(Item *item) {
    switch (item->type) {
        case VALUE_TYPE_STRING:
            delete item->val_string;
            break;
        default:
            break;
    }
}

const NextDictionary::Item *NextDictionary::findItem(const char *name, ValueType type) const {
    size_t i = findItemIndex(name, strlen(name));

    if (i < m_items.size()) {
        const Item *item = m_items[i];
        return item->type == type ? item : nullptr;
    }

    return nullptr;
}

size_t NextDictionary::findItemIndex(const char *name, size_t len) const {
    size_t i = 0;
    for (; i < m_items.size(); i++) {
        if (len != m_items[i]->name_len) {
            continue;
        }
        if (!memcmp(m_items[i]->item_name, name, len)) {
            break;
        }
    }
    return i;
}

void NextDictionary::setString(const char *name, const std::string &s) {
    Item *item = allocateItem(name);
    if (item) {
        item->type = VALUE_TYPE_STRING;
        item->val_string = new std::string(s);
    }
}

std::string NextDictionary::getString(const char *name, std::string *defaultValue) const {
    const Item *item = findItem(name, VALUE_TYPE_STRING);
    if (item) {
        return *(item->val_string);
    }
    return defaultValue ? (*defaultValue) : "";
}

size_t NextDictionary::getSize() const {
    return m_items.size();
}

const char *NextDictionary::getEntryNameAt(size_t index, ValueType *type) const {
    if (index >= m_items.size()) {
        *type = VALUE_TYPE_INT32;

        return nullptr;
    }
    *type = m_items[index]->type;
    return m_items[index]->item_name;
}
