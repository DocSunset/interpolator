#pragma once

namespace GL::LL
{
    class Program
    {
        Program();
        Program(const Program& other) = delete;
        Program(Program&& other);
        ~Program();
    };
}
