#include <iostream>
#include <regex>

int main() {
    if ( std::regex_search("one\ntwo", std::regex{"one$"}) ) {
        std::cout << "Should match, doesn't." << std::endl;
        std::cout << "^ above is wrong. should not match." << std::endl;
    }

    if ( std::regex_search("one\ntwo", std::regex{"two$"}
                         , std::regex_constants::match_not_eol) ) {
        std::cout << "Shouldn't match, does." << std::endl;
        std::cout << "^ above is wrong. it should match." << std::endl;
    }

    return 0;
}
