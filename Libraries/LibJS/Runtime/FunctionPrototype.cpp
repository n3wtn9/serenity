/*
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

#include <AK/Function.h>
#include <AK/StringBuilder.h>
#include <LibJS/AST.h>
#include <LibJS/Interpreter.h>
#include <LibJS/Runtime/BoundFunction.h>
#include <LibJS/Runtime/Error.h>
#include <LibJS/Runtime/Function.h>
#include <LibJS/Runtime/FunctionPrototype.h>
#include <LibJS/Runtime/GlobalObject.h>
#include <LibJS/Runtime/MarkedValueList.h>
#include <LibJS/Runtime/ScriptFunction.h>

namespace JS {

FunctionPrototype::FunctionPrototype(GlobalObject& global_object)
    : Object(global_object.object_prototype())
{
}

void FunctionPrototype::initialize(Interpreter& interpreter, GlobalObject& global_object)
{
    Object::initialize(interpreter, global_object);
    u8 attr = Attribute::Writable | Attribute::Configurable;
    define_native_function("apply", apply, 2, attr);
    define_native_function("bind", bind, 1, attr);
    define_native_function("call", call, 1, attr);
    define_native_function("toString", to_string, 0, attr);
    define_property("length", Value(0), Attribute::Configurable);
    define_property("name", js_string(heap(), ""), Attribute::Configurable);
}

FunctionPrototype::~FunctionPrototype()
{
}

JS_DEFINE_NATIVE_FUNCTION(FunctionPrototype::apply)
{
    auto* this_object = interpreter.this_value(global_object).to_object(interpreter, global_object);
    if (!this_object)
        return {};
    if (!this_object->is_function())
        return interpreter.throw_exception<TypeError>(ErrorType::NotA, "Function");
    auto& function = static_cast<Function&>(*this_object);
    auto this_arg = interpreter.argument(0);
    auto arg_array = interpreter.argument(1);
    if (arg_array.is_null() || arg_array.is_undefined())
        return interpreter.call(function, this_arg);
    if (!arg_array.is_object())
        return interpreter.throw_exception<TypeError>(ErrorType::FunctionArgsNotObject);
    auto length_property = arg_array.as_object().get("length");
    if (interpreter.exception())
        return {};
    auto length = length_property.to_size_t(interpreter);
    if (interpreter.exception())
        return {};
    MarkedValueList arguments(interpreter.heap());
    for (size_t i = 0; i < length; ++i) {
        auto element = arg_array.as_object().get(i);
        if (interpreter.exception())
            return {};
        arguments.append(element.value_or(js_undefined()));
    }
    return interpreter.call(function, this_arg, move(arguments));
}

JS_DEFINE_NATIVE_FUNCTION(FunctionPrototype::bind)
{
    auto* this_object = interpreter.this_value(global_object).to_object(interpreter, global_object);
    if (!this_object)
        return {};
    if (!this_object->is_function())
        return interpreter.throw_exception<TypeError>(ErrorType::NotA, "Function");

    auto& this_function = static_cast<Function&>(*this_object);
    auto bound_this_arg = interpreter.argument(0);

    Vector<Value> arguments;
    if (interpreter.argument_count() > 1) {
        arguments = interpreter.call_frame().arguments;
        arguments.remove(0);
    }

    return this_function.bind(bound_this_arg, move(arguments));
}

JS_DEFINE_NATIVE_FUNCTION(FunctionPrototype::call)
{
    auto* this_object = interpreter.this_value(global_object).to_object(interpreter, global_object);
    if (!this_object)
        return {};
    if (!this_object->is_function())
        return interpreter.throw_exception<TypeError>(ErrorType::NotA, "Function");
    auto& function = static_cast<Function&>(*this_object);
    auto this_arg = interpreter.argument(0);
    MarkedValueList arguments(interpreter.heap());
    if (interpreter.argument_count() > 1) {
        for (size_t i = 1; i < interpreter.argument_count(); ++i)
            arguments.append(interpreter.argument(i));
    }
    return interpreter.call(function, this_arg, move(arguments));
}

JS_DEFINE_NATIVE_FUNCTION(FunctionPrototype::to_string)
{
    auto* this_object = interpreter.this_value(global_object).to_object(interpreter, global_object);
    if (!this_object)
        return {};
    if (!this_object->is_function())
        return interpreter.throw_exception<TypeError>(ErrorType::NotA, "Function");

    String function_name = static_cast<Function*>(this_object)->name();
    String function_parameters = "";
    String function_body;

    if (this_object->is_native_function() || this_object->is_bound_function()) {
        function_body = String::format("  [%s]", this_object->class_name());
    } else {
        StringBuilder parameters_builder;
        auto first = true;
        for (auto& parameter : static_cast<ScriptFunction*>(this_object)->parameters()) {
            if (!first)
                parameters_builder.append(", ");
            first = false;
            parameters_builder.append(parameter.name);
            if (parameter.default_value) {
                // FIXME: See note below
                parameters_builder.append(" = TODO");
            }
        }
        function_parameters = parameters_builder.build();
        // FIXME: ASTNodes should be able to dump themselves to source strings - something like this:
        // auto& body = static_cast<ScriptFunction*>(this_object)->body();
        // function_body = body.to_source();
        function_body = "  ???";
    }

    auto function_source = String::format("function %s(%s) {\n%s\n}",
        function_name.is_null() ? "" : function_name.characters(),
        function_parameters.characters(),
        function_body.characters());
    return js_string(interpreter, function_source);
}

}
