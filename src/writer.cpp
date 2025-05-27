#include "writer.h"

void Writer::write_OVX(
    const std::string& outfile,
    const std::vector<Vertex>& vert,
    const std::vector<std::tuple<std::vector<int>, std::vector<int>, std::vector<Dummy>>> &ovx
) {
    std::ofstream out(outfile);
    if(!out) throw WriterException(std::format("Cannot write to file {}!", outfile));

    out << ovx.size() << "\n";
    for(auto& [V, O, dummy] : ovx){
        out << (O.size() / 3) << "\n";
        for(int i = 0; i < O.size(); ++i){
            out << V[i] << " " << O[i] << "\n";
        }
        out << dummy.size() << "\n";
        for(auto d : dummy){
            out << d.first << "\n";
        }
    }
    out << vert.size() << "\n";
    for(auto& v : vert){
        out << v[0] << " " << v[1] << " " << v[2] << "\n";
    }
}

void Writer::write_Compressed(
    const std::string &outfile, 
    const std::vector<std::tuple<std::vector<Vertex>, std::pair<int, std::vector<CLERS>>, std::vector<Handle>, std::vector<Dummy>>> &compressed
) {
    std::ofstream out(outfile);
    if(!out) throw WriterException(std::format("Cannot write to file {}!", outfile));

    out << compressed.size() << "\n";
    for(auto& [vertices, clers, handles, dummy] : compressed){
        out << vertices.size() << "\n";
        for(const auto& v : vertices){
            out << std::fixed << std::setprecision(6) << v[0] << " " << v[1] << " " << v[2] << "\n";
        }
        out << clers.second.size() << "\n";
        out << clers.first << "\n";
        for(const auto c : clers.second){
            switch (c)
            {
            case CLERS::C:
                out << "C";
                break;
            case CLERS::L:
                out << "L";
                break;
            case CLERS::E:
                out << "E";
                break;
            case CLERS::R:
                out << "R";
                break;
            case CLERS::S:
                out << "S";
                break;
            }
            out << "\n";
        }
        out << handles.size() << "\n";
        for(const auto& h : handles){
            out << h[0] << " " << h[1] << "\n";
        }
        out << dummy.size() << "\n";
        for(const auto& d : dummy){
            out << d.first << "\n";
        }
    }
}

void Writer::write_Compressed_BIN(
    const std::string &outfile, 
    const std::vector<std::tuple<std::vector<Vertex>, std::pair<int, std::vector<CLERS>>, std::vector<Handle>, std::vector<Dummy>>> &compressed
) {
    std::ofstream out(outfile, std::ios::binary);
    if (!out) throw WriterException(std::format("Cannot write to file {}!", outfile));

    // Write the number of compressed entries
    size_t count = compressed.size();
    out.write(reinterpret_cast<const char*>(&count), sizeof(count));

    for (auto& [vertices, clers, handles, dummy] : compressed) {
        // Write vertices
        size_t vertex_count = vertices.size();
        out.write(reinterpret_cast<const char*>(&vertex_count), sizeof(vertex_count));
        for (auto& v : vertices) {
            float x = v[0], y = v[1], z = v[2];
            out.write(reinterpret_cast<const char*>(&x), sizeof(x));
            out.write(reinterpret_cast<const char*>(&y), sizeof(y));
            out.write(reinterpret_cast<const char*>(&z), sizeof(z));
        }

        // Write CLERS data
        size_t clers_count = clers.second.size();
        out.write(reinterpret_cast<const char*>(&clers_count), sizeof(clers_count));
        out.write(reinterpret_cast<const char*>(&clers.first), sizeof(clers.first));

        unsigned char bit_buffer = 0;
        int bit_pos = 7; // Start at the highest bit

        for (auto c : clers.second) {
            uint32_t code_value;
            uint8_t code_length;

            switch (c) {
                case CLERS::C:
                    code_value = 0b0;
                    code_length = 1;
                    break;
                case CLERS::L:
                    code_value = 0b110;
                    code_length = 3;
                    break;
                case CLERS::E:
                    code_value = 0b111;
                    code_length = 3;
                    break;
                case CLERS::R:
                    code_value = 0b101;
                    code_length = 3;
                    break;
                case CLERS::S:
                    code_value = 0b100;
                    code_length = 3;
                    break;
                default:
                    throw WriterException("Invalid CLERS value");
            }

            for (int i = code_length - 1; i >= 0; --i) {
                int bit = (code_value >> i) & 1;
                if (bit) {
                    bit_buffer |= (1 << bit_pos);
                }

                --bit_pos;

                if (bit_pos < 0) {
                    out.write(reinterpret_cast<const char*>(&bit_buffer), sizeof(bit_buffer));
                    bit_buffer = 0;
                    bit_pos = 7;
                }
            }
        }

        // Flush any remaining bits in the buffer
        if (bit_pos != 7) {
            out.write(reinterpret_cast<const char*>(&bit_buffer), sizeof(bit_buffer));
        }

        // Write handles
        size_t handle_count = handles.size();
        out.write(reinterpret_cast<const char*>(&handle_count), sizeof(handle_count));
        for (auto& h : handles) {
            int h0 = h[0], h1 = h[1];
            out.write(reinterpret_cast<const char*>(&h0), sizeof(h0));
            out.write(reinterpret_cast<const char*>(&h1), sizeof(h1));
        }

        // Write dummy data
        size_t dummy_count = dummy.size();
        out.write(reinterpret_cast<const char*>(&dummy_count), sizeof(dummy_count));
        for (auto& d : dummy) {
            int val = d.first;
            out.write(reinterpret_cast<const char*>(&val), sizeof(val));
        }
    }
}

void Writer::write_OBJ(
    const std::string &outfile, 
    const std::vector<Vertex> &vert, 
    const std::vector<Indices> &tri
) {
    std::ofstream out(outfile);
    if(!out) throw WriterException(std::format("Cannot write to file {}!", outfile));

    for(auto& v : vert){
        out << "v " << v[0] << " " << v[1] << " " << v[2] << "\n";
    }

    for(auto& t : tri){
        out << "f " << (t[0] + 1) << " " << (t[1] + 1) << " " << (t[2] + 1) << "\n";
    }
}

void Writer::write_OFF(
    const std::string &outfile, 
    const std::vector<Vertex> &vert, 
    const std::vector<Indices> &tri
) {
    std::ofstream out(outfile);
    if (!out) throw WriterException(std::format("Cannot write to file {}!", outfile));

    // Write the header for OFF
    out << "OFF" << "\n";
    
    out << vert.size() << " " << tri.size() << " 0" << "\n";
    
    // Write vertex coordinates
    for (const auto& v : vert) {
        out << v[0] << " " << v[1] << " " << v[2] << "\n";
    }

    // Write faces
    for (const auto& t : tri) {
        out << "3 " << t[0] << " " << t[1] << " " << t[2] << "\n";
    }
}