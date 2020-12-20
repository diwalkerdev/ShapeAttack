#include "errorcodes.hpp"
#include "gameevents.h"
#include "msgpack.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>

namespace serialisation {

// auto run_msgpack_example(void) -> int
// {
//     msgpack::type::tuple<int, bool, std::string> src(1, true, "example");

//     // serialize the object into the buffer.
//     // any classes that implements write(const char*,size_t) can be a buffer.
//     std::stringstream buffer;
//     msgpack::pack(buffer, src);

//     // send the buffer ...
//     buffer.seekg(0);

//     // deserialize the buffer into msgpack::object instance.
//     std::string str(buffer.str());

//     msgpack::object_handle oh = msgpack::unpack(str.data(), str.size());

//     // deserialized object is valid during the msgpack::object_handle instance is alive.
//     msgpack::object deserialized = oh.get();

//     // msgpack::object supports ostream.
//     std::cout << deserialized << std::endl;

//     // convert msgpack::object instance into the original type.
//     // if the type is mismatched, it throws msgpack::type_error exception.
//     msgpack::type::tuple<int, bool, std::string> dst;
//     deserialized.convert(dst);

//     // or create the new instance
//     msgpack::type::tuple<int, bool, std::string> dst2 = deserialized.as<msgpack::type::tuple<int, bool, std::string>>();

//     return 0;
// }


auto save(std::filesystem::path const& file_path, DevOptions& events) -> void
{
    std::cout << "Serialisation::Saving\n";

    std::ofstream ofile;
    ofile.open(file_path, std::ios::out | std::ios::trunc | std::ios::binary);
    if (!ofile.is_open())
    {
        exit(SERIALISATION_FAILED_TO_OPEN_SAVE_FILE);
    }

    msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, events);

    std::string_view data(sbuf.data(), sbuf.size());
    ofile << data;

    ofile.close();
}

auto load(std::filesystem::path const& file_path, DevOptions& events) -> void
{
    std::cout << "Serialisation::Loading\n";

    std::ifstream ifile;
    ifile.open(file_path, std::ios::in | std::ios::binary);

    if (!ifile.is_open())
    {
        exit(SERIALISATION_FAILED_TO_OPEN_LOAD_FILE);
    }

    std::string the_file;
    std::string the_line;

    while (std::getline(ifile, the_line))
    {
        the_file.append(the_line);
    }

    // std::cout << the_file << std::endl;
    // std::cout << "Size: " << the_file.size() << std::endl;

    msgpack::object_handle oh  = msgpack::unpack(the_file.c_str(), the_file.size());
    msgpack::object        obj = oh.get();

    obj.convert(events);

    // std::cout << new_events.draw_vectors << "\n";
    // std::cout << new_events.draw_minkowski << "\n";

    //events = new_events;
}


// auto run_load_save_example(std::filesystem::path const& exe_base_dir, GameEvents& events) -> void
// {
//     std::filesystem::path the_path = (exe_base_dir / "game_state.msgpack");
//     std::cout << "Game State Dir: " << the_path << "\n";
//     save(the_path, events);
//     load(the_path, events);
// }

}
