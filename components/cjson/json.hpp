/**
 * @file json.hpp
 * @author Dariusz Krempa
 * @brief C++ wrapper for cjson API
 * @version 1.0
 * @date 2022
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#pragma once 

#include "cJSON.h"


class JSON
{
private:
    cJSON *root;
    int type() { return root->type; }

public:
    /**
     * @brief Construct a new JSON object from json string
     * 
     * @param obj 
     */
    JSON(const char* obj);
    /**
     * @brief Make a duplicate of cJSON object
     * 
     * @param obj 
     */
    JSON(cJSON* obj);
    /**
     * @brief Construct a new empty JSON object of type
     * 
     * @param type 
     */
    JSON(uint8_t type = cJSON_Object);
    ~JSON();

public:
    /**
     * @brief Duplicate cJSON object
     * 
     * @param obj 
     * @return cJSON* 
     */
    static cJSON* createObject(cJSON* obj);
    /**
     * @brief Add bool value to root object
     * 
     * @param key 
     * @param val 
     * @return cJSON* 
     */
    cJSON* add(const char *key, bool val);
    /**
     * @brief Add integer value to root object
     * 
     * @param key 
     * @param val 
     * @return cJSON* 
     */
    cJSON* add(const char *key, int val);
    /**
     * @brief Add float value to root object
     * 
     * @param key 
     * @param val 
     * @return cJSON* 
     */
    cJSON* add(const char *key, float val);
    /**
     * @brief Add double value to root object
     * 
     * @param key 
     * @param val 
     * @return cJSON* 
     */
    cJSON* add(const char *key, double val);
    /**
     * @brief Add long value to root object
     * 
     * @param key 
     * @param val 
     * @return cJSON* 
     */
    cJSON* add(const char *key, long val);
    /**
     * @brief Add i
     * 
     * @param key 
     * @param val 
     * @param raw 
     * @return cJSON* 
     */
    cJSON* add(const char *key, const char* val, bool raw = false);
    /**
     * @brief Add cJSON object to root object
     * 
     * @param key 
     * @param item 
     * @return true 
     * @return false 
     */
    bool add(const char *key, cJSON *item);
    /**
     * @brief Create empty array within root object
     * 
     * @param key 
     * @return cJSON* 
     */
    cJSON* addArray(const char *key);

    /**
     * @brief Change the valuestring of a cJSON_String object, only takes effect when type of object is cJSON_String
     * 
     * @param val 
     * @param obj 
     */
    void set(char* val, cJSON* obj = NULL);
    /**
     * @brief Change valueint and valuedouble in object or this root object
     * 
     * @param val 
     * @param obj 
     */
    void set(int val, cJSON* obj = NULL);
    /**
     * @brief Change valuedouble in object or this root object
     * 
     * @param val 
     * @param obj 
     */
    void set(double val, cJSON* obj = NULL);
    /**
     * @brief Change valueint in object or this root object
     * 
     * @param val 
     * @param obj 
     */
    void set(bool val, cJSON* obj = NULL);
    /**
     * @brief Change valuedouble in object or this root object
     * 
     * @param val 
     * @param obj 
     */
    void set(float val, cJSON* obj = NULL);
    /**
     * @brief Get the main/root object
     * 
     * @return this cJSON object
     */
    cJSON* get() { return root; }

    /**
     * @brief Add this root object to array object
     * 
     * @param array 
     * @return true 
     * @return false 
     */
    bool addToArray(cJSON* array);

    bool appendToArray(cJSON *obj);
    bool appendToArray(float val);

    /**
     * @brief Get the String value from json object
     * 
     * @param key 
     * @return char* 
     */
    char* getString(const char* key = NULL);
    /**
     * @brief Get the Int value from json object
     * 
     * @param key 
     * @return int 
     */
    int getInt(const char* key = NULL);
    /**
     * @brief Get the Double value from json object
     * 
     * @param key 
     * @return double 
     */
    double getDouble(const char* key = NULL);
    /**
     * @brief Get the cJSON Object, for example to retrieve arrays
     * 
     * @param key 
     * @param sensitive case sensitive key
     * @return cJSON* 
     */
    cJSON* getObject(const char* key, bool sensitive = false);
    /**
     * @brief Check if value with key exist in json onject
     * 
     * @param key 
     * @return true 
     * @return false 
     */
    bool hasKey(const char* key);
    /**
     * @brief Convert json object to char string, char* should be free() after use to avoid memory leak
     * 
     * @param obj 
     * @return char* 
     */
    char* print(cJSON* obj = NULL);
    /**
     * @brief Minify a strings, remove blank characters(such as ' ', '\\t', '\\r', '\\n') from strings.
     * 
     * @param val 
     */
    static void minify(char* val);
    // bool compare();
};

