/*
 * Copyright (c) 2020, Andreas Kling <kling@serenityos.org>
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

#include <AK/Function.h>
#include <LibJS/Heap/Heap.h>
#include <LibJS/Interpreter.h>
#include <LibJS/Runtime/Error.h>
#include <LibJS/Runtime/ErrorPrototype.h>
#include <LibJS/Runtime/GlobalObject.h>
#include <LibJS/Runtime/PrimitiveString.h>
#include <LibJS/Runtime/Value.h>

namespace JS {

ErrorPrototype::ErrorPrototype(GlobalObject& global_object)
    : Object(global_object.object_prototype())
{
}

void ErrorPrototype::initialize(Interpreter& interpreter, GlobalObject& global_object)
{
    Object::initialize(interpreter, global_object);
    u8 attr = Attribute::Writable | Attribute::Configurable;
    define_native_property("name", name_getter, name_setter, attr);
    define_native_property("message", message_getter, nullptr, attr);
    define_native_function("toString", to_string, 0, attr);
}

ErrorPrototype::~ErrorPrototype()
{
}

JS_DEFINE_NATIVE_GETTER(ErrorPrototype::name_getter)
{
    auto* this_object = interpreter.this_value(global_object).to_object(interpreter, global_object);
    if (!this_object)
        return {};
    if (!this_object->is_error())
        return interpreter.throw_exception<TypeError>(ErrorType::NotAn, "Error");
    return js_string(interpreter, static_cast<const Error*>(this_object)->name());
}

JS_DEFINE_NATIVE_SETTER(ErrorPrototype::name_setter)
{
    auto* this_object = interpreter.this_value(global_object).to_object(interpreter, global_object);
    if (!this_object)
        return;
    if (!this_object->is_error()) {
        interpreter.throw_exception<TypeError>(ErrorType::NotAn, "Error");
        return;
    }
    auto name = value.to_string(interpreter);
    if (interpreter.exception())
        return;
    static_cast<Error*>(this_object)->set_name(name);
}

JS_DEFINE_NATIVE_GETTER(ErrorPrototype::message_getter)
{
    auto* this_object = interpreter.this_value(global_object).to_object(interpreter, global_object);
    if (!this_object)
        return {};
    if (!this_object->is_error())
        return interpreter.throw_exception<TypeError>(ErrorType::NotAn, "Error");
    return js_string(interpreter, static_cast<const Error*>(this_object)->message());
}

JS_DEFINE_NATIVE_FUNCTION(ErrorPrototype::to_string)
{
    if (!interpreter.this_value(global_object).is_object())
        return interpreter.throw_exception<TypeError>(ErrorType::NotAnObject, interpreter.this_value(global_object).to_string_without_side_effects().characters());
    auto& this_object = interpreter.this_value(global_object).as_object();

    String name = "Error";
    auto name_property = this_object.get("name");
    if (interpreter.exception())
        return {};
    if (!name_property.is_empty() && !name_property.is_undefined()) {
        name = name_property.to_string(interpreter);
        if (interpreter.exception())
            return {};
    }

    String message = "";
    auto message_property = this_object.get("message");
    if (interpreter.exception())
        return {};
    if (!message_property.is_empty() && !message_property.is_undefined()) {
        message = message_property.to_string(interpreter);
        if (interpreter.exception())
            return {};
    }

    if (name.length() == 0)
        return js_string(interpreter, message);
    if (message.length() == 0)
        return js_string(interpreter, name);
    return js_string(interpreter, String::format("%s: %s", name.characters(), message.characters()));
}

#define __JS_ENUMERATE(ClassName, snake_name, PrototypeName, ConstructorName) \
    PrototypeName::PrototypeName(GlobalObject& global_object)                 \
        : Object(global_object.error_prototype())                             \
    {                                                                         \
    }                                                                         \
    PrototypeName::~PrototypeName() { }                                       \
    const char* PrototypeName::class_name() const { return #PrototypeName; }

JS_ENUMERATE_ERROR_SUBCLASSES
#undef __JS_ENUMERATE

}
