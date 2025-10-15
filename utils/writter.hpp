#include <sstream>
#include <iostream>

 
class Writer
{
    std::ostringstream buffer;
public:
    ~Writer()
    {
        std::cout << buffer.str();
    }

    template <typename T>
    Writer& operator<<(T&& input)
    {
        buffer << std::forward<T>(input);
        return *this;
    }

    Writer& operator<<(std::ostream& (*manip)(std::ostream&))
    {
        buffer << manip;
        return *this;
    }
};