
/*
    Copyright (c) 2020 Kristjan Kongas

    Permission to use, copy, modify, and/or distribute this software for any
    purpose with or without fee is hereby granted.

    THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
    REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
    AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
    INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
    LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
    OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
    PERFORMANCE OF THIS SOFTWARE.
*/

#include <iostream>
#include "../fire.hpp"

using namespace std;

int fired_main(bool flag_a = fire::arg({"a", "flag-a"}), bool flag_b = fire::arg({"b", "flag-b"})) {
    cout << "flag-a: " << (flag_a ? "true" : "false") << "   flag-b: " << (flag_b ? "true" : "false") << endl;
    return 0;
}

FIRE(fired_main)
