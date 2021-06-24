# Dynx
Dynx is a dynamic variable engine. 
Dynamic values are self-contained objects which automatically track dependencies between themselves.

# Example
```c++
#include <ostream>
#include <dynx-0.0.0/dynx.hpp>

using namespace dynx;

int main(){
    Dynx<int> x = 5;
    Dynx<int> y([&](){
        return x * x;
    });
    while(x < 10){
        std::cout << x << " " << y << std::endl;
        x = x + 1;
    }
    //produces:
    //    5 25
    //    6 36
    //    7 49
    //    8 64
    //    9 81
}
```

# Installation
```bash
git clone xyz
cd dynx-cpp
mkdir cmake-build-release
cd cmake-build-release
cmake ..
sudo cmake --build . --target install
```

# Usage
```cmake
# ...
project(my-project)

# ...

find_package(dynx REQUIRED)

# ...

add_executable(my-project ${SOURCES})
target_link_libraries(my-project dynx)
```