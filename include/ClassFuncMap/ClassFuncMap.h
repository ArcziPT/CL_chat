#ifndef CLASS_FUNC_MAP_H
#define CLASS_FUNC_MAP_H

#include <map>
#include <vector>

template <typename Key, typename Class, typename ...Args>
class ClassFuncMap{
public:
    void add_entry(const Key& key, void (Class::*ptr)(Args...)){
         func.insert(std::pair<Key, void (Class::*)(Args...)>(key, ptr));
         keys.push_back(key);
    };

    void remove_entry(const Key& key){
        func.erase(key);
        keys.erase(find(keys.begin(), keys.end(), key));
    };

    void operator()(Class* c, const Key& key, Args && ... args){
        (c->*func[key])(args...);
    };

    std::vector<Key> get_keys(){
        return keys;
    };

    bool has_key(const Key& key){
        return (find(keys.begin(), keys.end(), key) != keys.end());
    };

private:
    std::map<Key, void (Class::*)(Args...)> func; 
    std::vector<Key> keys;
};

#endif