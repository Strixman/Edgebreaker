#include "loader.h"
#include "converter.h"

int main(int argc, char* argv[]) {
    auto [vert, tri] = Loader::load(argv[1], Loader::EXT::OBJ);
    auto t = Converter::toOVX(vert, tri);
    return 0;
}