#include "converter.h"
#include "reader.h"
#include "writer.h"

int main(int argc, char* argv[]) {
    auto [vert, tri] = Reader::read_OBJ(argv[1]);
    auto ovx = Converter::toOVX(vert, tri);
    Writer::write_OVX("out.ov", vert, ovx);

    auto [v, o] = Reader::read_OVX("out.ov");
    Writer::write_OVX("out1.ov", v, o);

    return 0;
}