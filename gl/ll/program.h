#pragma once

namespace GL::LL
{
    class Program
    {
    public:
        Program();
        Program(const Program& other) = delete;
        Program(Program&& other);
        ~Program();
    };
}
