/*
 * Copyright (c) 2020, Andreas Kling <kling@serenityos.org>
 * Copyright (c) 2020, Linus Groh <mail@linusgroh.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include <LibJS/Runtime/Object.h>

namespace JS {

class ArrayPrototype final : public Object {
public:
    ArrayPrototype(GlobalObject&);
    virtual void initialize(Interpreter&, GlobalObject&) override;
    virtual ~ArrayPrototype() override;

private:
    virtual const char* class_name() const override { return "ArrayPrototype"; }

    JS_DECLARE_NATIVE_FUNCTION(filter);
    JS_DECLARE_NATIVE_FUNCTION(for_each);
    JS_DECLARE_NATIVE_FUNCTION(map);
    JS_DECLARE_NATIVE_FUNCTION(pop);
    JS_DECLARE_NATIVE_FUNCTION(push);
    JS_DECLARE_NATIVE_FUNCTION(shift);
    JS_DECLARE_NATIVE_FUNCTION(to_string);
    JS_DECLARE_NATIVE_FUNCTION(to_locale_string);
    JS_DECLARE_NATIVE_FUNCTION(unshift);
    JS_DECLARE_NATIVE_FUNCTION(join);
    JS_DECLARE_NATIVE_FUNCTION(concat);
    JS_DECLARE_NATIVE_FUNCTION(slice);
    JS_DECLARE_NATIVE_FUNCTION(index_of);
    JS_DECLARE_NATIVE_FUNCTION(reduce);
    JS_DECLARE_NATIVE_FUNCTION(reduce_right);
    JS_DECLARE_NATIVE_FUNCTION(reverse);
    JS_DECLARE_NATIVE_FUNCTION(last_index_of);
    JS_DECLARE_NATIVE_FUNCTION(includes);
    JS_DECLARE_NATIVE_FUNCTION(find);
    JS_DECLARE_NATIVE_FUNCTION(find_index);
    JS_DECLARE_NATIVE_FUNCTION(some);
    JS_DECLARE_NATIVE_FUNCTION(every);
    JS_DECLARE_NATIVE_FUNCTION(splice);
    JS_DECLARE_NATIVE_FUNCTION(fill);
};

}
