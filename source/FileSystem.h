//
// Copyright SpiRITlab - Computations on Encrypted Data
// https://gitlab.com/SpiRITlab/coed
//

#ifndef LYNX_FILESYSTEM_H
#define LYNX_FILESYSTEM_H

#include <iostream>
#include <fstream>

namespace COED {
    class FileSystem {
    public:
        FileSystem(const std::string &);

        void open_output_stream();

        void open_output_stream(const std::ios_base::openmode &);

        std::ofstream &get_output_stream();

        void close_output_stream();

        void open_input_stream();

        void open_input_stream(const std::ios_base::openmode &);

        std::ifstream &get_input_stream();

        void close_input_stream();

    private:
        std::string filesystem_path;
        std::ifstream input_file_stream;
        std::ofstream output_file_stream;
    };

}

#endif //LYNX_FILESYSTEM_H
