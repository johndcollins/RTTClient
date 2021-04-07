#include "CTools.h"

Tools::Tools()
{

}

Tools::~Tools()
{
    //dtor
}

void Tools::debug(const std::string& str)
{
    std::cout << str.c_str() << std::endl;
}

void Tools::err(const std::string& str)
{
    std::cerr << str.c_str() << std::endl;
}