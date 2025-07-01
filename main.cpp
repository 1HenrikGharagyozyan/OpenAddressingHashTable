#include <iostream>
#include <string>
#include "OpenAddressingHashTable.h"  

int main()
{
    using HashTable = OpenAddressingHashTable<int, std::string>;

    HashTable table;

    auto [it1, inserted1] = table.insert({ 1, "one" });
    auto [it2, inserted2] = table.insert({ 2, "two" });
    auto [it3, inserted3] = table.insert({ 3, "three" });

    std::cout << "Inserted 1: " << inserted1 << ", value: " << it1->second << '\n';
    std::cout << "Inserted 2: " << inserted2 << ", value: " << it2->second << '\n';
    std::cout << "Inserted 3: " << inserted3 << ", value: " << it3->second << '\n';

    auto found = table.find(2);
    if (found != table.end())
        std::cout << "Found key 2: " << found->second << '\n';
    else
        std::cout << "Key 2 not found\n";

    table[4] = "four";
    std::cout << "Key 4 via operator[]: " << table[4] << '\n';

    try
    {
        std::cout << "Key 3 via at(): " << table.at(3) << '\n';
        std::cout << "Key 10 via at(): " << table.at(10) << '\n';  // Нет такого ключа, должно кинуть
    }
    catch (const std::out_of_range& e) 
    {
        std::cout << "Exception caught: " << e.what() << '\n';
    }

    size_t erased = table.erase(2);
    std::cout << "Erased key 2, count: " << erased << '\n';
    std::cout << "Find key 2 after erase: " << (table.find(2) == table.end() ? "not found" : "found") << '\n';


    std::cout << "Size: " << table.size() << '\n';
    std::cout << "Is empty: " << std::boolalpha << table.empty() << '\n';

    using MultiHashTable = OpenAddressingHashTable<int, std::string, std::hash<int>, std::equal_to<int>, LinearProbing<int>, true>;
    MultiHashTable multiTable;
    multiTable.insert({ 5, "five" });
    multiTable.insert({ 5, "five duplicate" });
    std::cout << "MultiTable count for key 5: " << multiTable.count(5) << '\n';


    auto range = multiTable.equal_range(5);
    std::cout << "MultiTable equal_range for key 5:\n";
    for (auto it = range.first; it != range.second; ++it) 
        std::cout << "  Key: " << it->first << ", Value: " << it->second << '\n';

    return 0;
}
