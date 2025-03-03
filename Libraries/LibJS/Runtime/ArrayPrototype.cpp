/*
 * Copyright (c) 2020, Andreas Kling <kling@serenityos.org>
 * Copyright (c) 2020, Linus Groh <mail@linusgroh.de>
 * Copyright (c) 2020, Marcin Gasperowicz <xnooga@gmail.com>
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
#include <LibJS/Heap/Heap.h>
#include <LibJS/Interpreter.h>
#include <LibJS/Runtime/Array.h>
#include <LibJS/Runtime/ArrayPrototype.h>
#include <LibJS/Runtime/Error.h>
#include <LibJS/Runtime/Function.h>
#include <LibJS/Runtime/GlobalObject.h>
#include <LibJS/Runtime/MarkedValueList.h>
#include <LibJS/Runtime/ObjectPrototype.h>
#include <LibJS/Runtime/Value.h>

namespace JS {

ArrayPrototype::ArrayPrototype(GlobalObject& global_object)
    : Object(global_object.object_prototype())
{
}

void ArrayPrototype::initialize(Interpreter& interpreter, GlobalObject& global_object)
{
    Object::initialize(interpreter, global_object);
    u8 attr = Attribute::Writable | Attribute::Configurable;

    define_native_function("filter", filter, 1, attr);
    define_native_function("forEach", for_each, 1, attr);
    define_native_function("map", map, 1, attr);
    define_native_function("pop", pop, 0, attr);
    define_native_function("push", push, 1, attr);
    define_native_function("shift", shift, 0, attr);
    define_native_function("toString", to_string, 0, attr);
    define_native_function("toLocaleString", to_locale_string, 0, attr);
    define_native_function("unshift", unshift, 1, attr);
    define_native_function("join", join, 1, attr);
    define_native_function("concat", concat, 1, attr);
    define_native_function("slice", slice, 2, attr);
    define_native_function("indexOf", index_of, 1, attr);
    define_native_function("reduce", reduce, 1, attr);
    define_native_function("reduceRight", reduce_right, 1, attr);
    define_native_function("reverse", reverse, 0, attr);
    define_native_function("lastIndexOf", last_index_of, 1, attr);
    define_native_function("includes", includes, 1, attr);
    define_native_function("find", find, 1, attr);
    define_native_function("findIndex", find_index, 1, attr);
    define_native_function("some", some, 1, attr);
    define_native_function("every", every, 1, attr);
    define_native_function("splice", splice, 2, attr);
    define_native_function("fill", fill, 1, attr);
    define_property("length", Value(0), Attribute::Configurable);
}

ArrayPrototype::~ArrayPrototype()
{
}

static Function* callback_from_args(Interpreter& interpreter, const String& name)
{
    if (interpreter.argument_count() < 1) {
        interpreter.throw_exception<TypeError>(ErrorType::ArrayPrototypeOneArg, name.characters());
        return nullptr;
    }
    auto callback = interpreter.argument(0);
    if (!callback.is_function()) {
        interpreter.throw_exception<TypeError>(ErrorType::NotAFunction, callback.to_string_without_side_effects().characters());
        return nullptr;
    }
    return &callback.as_function();
}

static size_t get_length(Interpreter& interpreter, Object& object)
{
    auto length_property = object.get("length");
    if (interpreter.exception())
        return 0;
    return length_property.to_size_t(interpreter);
}

static void for_each_item(Interpreter& interpreter, GlobalObject& global_object, const String& name, AK::Function<IterationDecision(size_t index, Value value, Value callback_result)> callback, bool skip_empty = true)
{
    auto* this_object = interpreter.this_value(global_object).to_object(interpreter, global_object);
    if (!this_object)
        return;

    auto initial_length = get_length(interpreter, *this_object);
    if (interpreter.exception())
        return;

    auto* callback_function = callback_from_args(interpreter, name);
    if (!callback_function)
        return;

    auto this_value = interpreter.argument(1);

    for (size_t i = 0; i < initial_length; ++i) {
        auto value = this_object->get(i);
        if (interpreter.exception())
            return;
        if (value.is_empty()) {
            if (skip_empty)
                continue;
            value = js_undefined();
        }

        MarkedValueList arguments(interpreter.heap());
        arguments.append(value);
        arguments.append(Value((i32)i));
        arguments.append(this_object);

        auto callback_result = interpreter.call(*callback_function, this_value, move(arguments));
        if (interpreter.exception())
            return;

        if (callback(i, value, callback_result) == IterationDecision::Break)
            break;
    }
}

JS_DEFINE_NATIVE_FUNCTION(ArrayPrototype::filter)
{
    auto* new_array = Array::create(global_object);
    for_each_item(interpreter, global_object, "filter", [&](auto, auto value, auto callback_result) {
        if (callback_result.to_boolean())
            new_array->indexed_properties().append(value);
        return IterationDecision::Continue;
    });
    return Value(new_array);
}

JS_DEFINE_NATIVE_FUNCTION(ArrayPrototype::for_each)
{
    for_each_item(interpreter, global_object, "forEach", [](auto, auto, auto) {
        return IterationDecision::Continue;
    });
    return js_undefined();
}

JS_DEFINE_NATIVE_FUNCTION(ArrayPrototype::map)
{
    auto* this_object = interpreter.this_value(global_object).to_object(interpreter, global_object);
    if (!this_object)
        return {};
    auto initial_length = get_length(interpreter, *this_object);
    if (interpreter.exception())
        return {};
    auto* new_array = Array::create(global_object);
    new_array->indexed_properties().set_array_like_size(initial_length);
    for_each_item(interpreter, global_object, "map", [&](auto index, auto, auto callback_result) {
        new_array->put(index, callback_result);
        if (interpreter.exception())
            return IterationDecision::Break;
        return IterationDecision::Continue;
    });
    return Value(new_array);
}

JS_DEFINE_NATIVE_FUNCTION(ArrayPrototype::push)
{
    auto* this_object = interpreter.this_value(global_object).to_object(interpreter, global_object);
    if (!this_object)
        return {};
    if (this_object->is_array()) {
        auto* array = static_cast<Array*>(this_object);
        for (size_t i = 0; i < interpreter.argument_count(); ++i)
            array->indexed_properties().append(interpreter.argument(i));
        return Value(static_cast<i32>(array->indexed_properties().array_like_size()));
    }
    auto length = get_length(interpreter, *this_object);
    if (interpreter.exception())
        return {};
    auto argument_count = interpreter.argument_count();
    auto new_length = length + argument_count;
    if (new_length > MAX_ARRAY_LIKE_INDEX)
        return interpreter.throw_exception<TypeError>(ErrorType::ArrayMaxSize);
    for (size_t i = 0; i < argument_count; ++i) {
        this_object->put(length + i, interpreter.argument(i));
        if (interpreter.exception())
            return {};
    }
    auto new_length_value = Value((i32)new_length);
    this_object->put("length", new_length_value);
    if (interpreter.exception())
        return {};
    return new_length_value;
}

JS_DEFINE_NATIVE_FUNCTION(ArrayPrototype::unshift)
{
    auto* array = Array::typed_this(interpreter, global_object);
    if (!array)
        return {};
    for (size_t i = 0; i < interpreter.argument_count(); ++i)
        array->indexed_properties().insert(i, interpreter.argument(i));
    return Value(static_cast<i32>(array->indexed_properties().array_like_size()));
}

JS_DEFINE_NATIVE_FUNCTION(ArrayPrototype::pop)
{
    auto* this_object = interpreter.this_value(global_object).to_object(interpreter, global_object);
    if (!this_object)
        return {};
    if (this_object->is_array()) {
        auto* array = static_cast<Array*>(this_object);
        if (array->indexed_properties().is_empty())
            return js_undefined();
        return array->indexed_properties().take_last(array).value.value_or(js_undefined());
    }
    auto length = get_length(interpreter, *this_object);
    if (length == 0) {
        this_object->put("length", Value(0));
        return js_undefined();
    }
    auto index = length - 1;
    auto element = this_object->get(index).value_or(js_undefined());
    if (interpreter.exception())
        return {};
    this_object->delete_property(index);
    if (interpreter.exception())
        return {};
    this_object->put("length", Value((i32)index));
    if (interpreter.exception())
        return {};
    return element;
}

JS_DEFINE_NATIVE_FUNCTION(ArrayPrototype::shift)
{
    auto* array = Array::typed_this(interpreter, global_object);
    if (!array)
        return {};
    if (array->indexed_properties().is_empty())
        return js_undefined();
    auto result = array->indexed_properties().take_first(array);
    if (interpreter.exception())
        return {};
    return result.value.value_or(js_undefined());
}

JS_DEFINE_NATIVE_FUNCTION(ArrayPrototype::to_string)
{
    auto* this_object = interpreter.this_value(global_object).to_object(interpreter, global_object);
    if (!this_object)
        return {};
    auto join_function = this_object->get("join");
    if (interpreter.exception())
        return {};
    if (!join_function.is_function())
        return ObjectPrototype::to_string(interpreter, global_object);
    return interpreter.call(join_function.as_function(), this_object);
}

JS_DEFINE_NATIVE_FUNCTION(ArrayPrototype::to_locale_string)
{
    auto* this_object = interpreter.this_value(global_object).to_object(interpreter, global_object);
    if (!this_object)
        return {};
    String separator = ","; // NOTE: This is implementation-specific.
    auto length = get_length(interpreter, *this_object);
    if (interpreter.exception())
        return {};
    StringBuilder builder;
    for (size_t i = 0; i < length; ++i) {
        if (i > 0)
            builder.append(separator);
        auto value = this_object->get(i).value_or(js_undefined());
        if (interpreter.exception())
            return {};
        if (value.is_undefined() || value.is_null())
            continue;
        auto* value_object = value.to_object(interpreter, global_object);
        ASSERT(value_object);
        auto locale_string_result = value_object->invoke("toLocaleString");
        if (interpreter.exception())
            return {};
        auto string = locale_string_result.to_string(interpreter);
        if (interpreter.exception())
            return {};
        builder.append(string);
    }
    return js_string(interpreter, builder.to_string());
}

JS_DEFINE_NATIVE_FUNCTION(ArrayPrototype::join)
{
    auto* this_object = interpreter.this_value(global_object).to_object(interpreter, global_object);
    if (!this_object)
        return {};
    String separator = ",";
    if (interpreter.argument_count()) {
        separator = interpreter.argument(0).to_string(interpreter);
        if (interpreter.exception())
            return {};
    }
    auto length = get_length(interpreter, *this_object);
    if (interpreter.exception())
        return {};
    StringBuilder builder;
    for (size_t i = 0; i < length; ++i) {
        if (i > 0)
            builder.append(separator);
        auto value = this_object->get(i).value_or(js_undefined());
        if (interpreter.exception())
            return {};
        if (value.is_undefined() || value.is_null())
            continue;
        auto string = value.to_string(interpreter);
        if (interpreter.exception())
            return {};
        builder.append(string);
    }
    return js_string(interpreter, builder.to_string());
}

JS_DEFINE_NATIVE_FUNCTION(ArrayPrototype::concat)
{
    auto* array = Array::typed_this(interpreter, global_object);
    if (!array)
        return {};

    auto* new_array = Array::create(global_object);
    new_array->indexed_properties().append_all(array, array->indexed_properties());
    if (interpreter.exception())
        return {};

    for (size_t i = 0; i < interpreter.argument_count(); ++i) {
        auto argument = interpreter.argument(i);
        if (argument.is_array()) {
            auto& argument_object = argument.as_object();
            new_array->indexed_properties().append_all(&argument_object, argument_object.indexed_properties());
            if (interpreter.exception())
                return {};
        } else {
            new_array->indexed_properties().append(argument);
        }
    }

    return Value(new_array);
}

JS_DEFINE_NATIVE_FUNCTION(ArrayPrototype::slice)
{
    auto* array = Array::typed_this(interpreter, global_object);
    if (!array)
        return {};

    auto* new_array = Array::create(global_object);
    if (interpreter.argument_count() == 0) {
        new_array->indexed_properties().append_all(array, array->indexed_properties());
        if (interpreter.exception())
            return {};
        return new_array;
    }

    ssize_t array_size = static_cast<ssize_t>(array->indexed_properties().array_like_size());
    auto start_slice = interpreter.argument(0).to_i32(interpreter);
    if (interpreter.exception())
        return {};
    auto end_slice = array_size;

    if (start_slice > array_size)
        return new_array;

    if (start_slice < 0)
        start_slice = end_slice + start_slice;

    if (interpreter.argument_count() >= 2) {
        end_slice = interpreter.argument(1).to_i32(interpreter);
        if (interpreter.exception())
            return {};
        if (end_slice < 0)
            end_slice = array_size + end_slice;
        else if (end_slice > array_size)
            end_slice = array_size;
    }

    for (ssize_t i = start_slice; i < end_slice; ++i) {
        new_array->indexed_properties().append(array->get(i));
        if (interpreter.exception())
            return {};
    }

    return new_array;
}

JS_DEFINE_NATIVE_FUNCTION(ArrayPrototype::index_of)
{
    auto* this_object = interpreter.this_value(global_object).to_object(interpreter, global_object);
    if (!this_object)
        return {};
    i32 length = get_length(interpreter, *this_object);
    if (interpreter.exception())
        return {};
    if (length == 0)
        return Value(-1);
    i32 from_index = 0;
    if (interpreter.argument_count() >= 2) {
        from_index = interpreter.argument(1).to_i32(interpreter);
        if (interpreter.exception())
            return {};
        if (from_index >= length)
            return Value(-1);
        if (from_index < 0)
            from_index = max(length + from_index, 0);
    }
    auto search_element = interpreter.argument(0);
    for (i32 i = from_index; i < length; ++i) {
        auto element = this_object->get(i);
        if (interpreter.exception())
            return {};
        if (strict_eq(interpreter, element, search_element))
            return Value(i);
    }
    return Value(-1);
}

JS_DEFINE_NATIVE_FUNCTION(ArrayPrototype::reduce)
{
    auto* this_object = interpreter.this_value(global_object).to_object(interpreter, global_object);
    if (!this_object)
        return {};

    auto initial_length = get_length(interpreter, *this_object);
    if (interpreter.exception())
        return {};

    auto* callback_function = callback_from_args(interpreter, "reduce");
    if (!callback_function)
        return {};

    size_t start = 0;

    auto accumulator = js_undefined();
    if (interpreter.argument_count() > 1) {
        accumulator = interpreter.argument(1);
    } else {
        bool start_found = false;
        while (!start_found && start < initial_length) {
            auto value = this_object->get(start);
            if (interpreter.exception())
                return {};
            start_found = !value.is_empty();
            if (start_found)
                accumulator = value;
            start += 1;
        }
        if (!start_found) {
            interpreter.throw_exception<TypeError>(ErrorType::ReduceNoInitial);
            return {};
        }
    }

    auto this_value = js_undefined();

    for (size_t i = start; i < initial_length; ++i) {
        auto value = this_object->get(i);
        if (interpreter.exception())
            return {};
        if (value.is_empty())
            continue;

        MarkedValueList arguments(interpreter.heap());
        arguments.append(accumulator);
        arguments.append(value);
        arguments.append(Value((i32)i));
        arguments.append(this_object);

        accumulator = interpreter.call(*callback_function, this_value, move(arguments));
        if (interpreter.exception())
            return {};
    }

    return accumulator;
}

JS_DEFINE_NATIVE_FUNCTION(ArrayPrototype::reduce_right)
{
    auto* this_object = interpreter.this_value(global_object).to_object(interpreter, global_object);
    if (!this_object)
        return {};

    auto initial_length = get_length(interpreter, *this_object);
    if (interpreter.exception())
        return {};

    auto* callback_function = callback_from_args(interpreter, "reduceRight");
    if (!callback_function)
        return {};

    int start = initial_length - 1;

    auto accumulator = js_undefined();
    if (interpreter.argument_count() > 1) {
        accumulator = interpreter.argument(1);
    } else {
        bool start_found = false;
        while (!start_found && start >= 0) {
            auto value = this_object->get(start);
            if (interpreter.exception())
                return {};
            start_found = !value.is_empty();
            if (start_found)
                accumulator = value;
            start -= 1;
        }
        if (!start_found) {
            interpreter.throw_exception<TypeError>(ErrorType::ReduceNoInitial);
            return {};
        }
    }

    auto this_value = js_undefined();

    for (int i = start; i >= 0; --i) {
        auto value = this_object->get(i);
        if (interpreter.exception())
            return {};
        if (value.is_empty())
            continue;

        MarkedValueList arguments(interpreter.heap());
        arguments.append(accumulator);
        arguments.append(value);
        arguments.append(Value(i));
        arguments.append(this_object);

        accumulator = interpreter.call(*callback_function, this_value, move(arguments));
        if (interpreter.exception())
            return {};
    }

    return accumulator;
}

JS_DEFINE_NATIVE_FUNCTION(ArrayPrototype::reverse)
{
    auto* array = Array::typed_this(interpreter, global_object);
    if (!array)
        return {};

    if (array->indexed_properties().is_empty())
        return array;

    Vector<Value> array_reverse;
    auto size = array->indexed_properties().array_like_size();
    array_reverse.ensure_capacity(size);

    for (ssize_t i = size - 1; i >= 0; --i) {
        array_reverse.append(array->get(i));
        if (interpreter.exception())
            return {};
    }

    array->set_indexed_property_elements(move(array_reverse));

    return array;
}

JS_DEFINE_NATIVE_FUNCTION(ArrayPrototype::last_index_of)
{
    auto* this_object = interpreter.this_value(global_object).to_object(interpreter, global_object);
    if (!this_object)
        return {};
    i32 length = get_length(interpreter, *this_object);
    if (interpreter.exception())
        return {};
    if (length == 0)
        return Value(-1);
    i32 from_index = length - 1;
    if (interpreter.argument_count() >= 2) {
        from_index = interpreter.argument(1).to_i32(interpreter);
        if (interpreter.exception())
            return {};
        if (from_index >= 0)
            from_index = min(from_index, length - 1);
        else
            from_index = length + from_index;
    }
    auto search_element = interpreter.argument(0);
    for (i32 i = from_index; i >= 0; --i) {
        auto element = this_object->get(i);
        if (interpreter.exception())
            return {};
        if (strict_eq(interpreter, element, search_element))
            return Value(i);
    }
    return Value(-1);
}

JS_DEFINE_NATIVE_FUNCTION(ArrayPrototype::includes)
{
    auto* this_object = interpreter.this_value(global_object).to_object(interpreter, global_object);
    if (!this_object)
        return {};
    i32 length = get_length(interpreter, *this_object);
    if (interpreter.exception())
        return {};
    if (length == 0)
        return Value(false);
    i32 from_index = 0;
    if (interpreter.argument_count() >= 2) {
        from_index = interpreter.argument(1).to_i32(interpreter);
        if (interpreter.exception())
            return {};
        if (from_index >= length)
            return Value(false);
        if (from_index < 0)
            from_index = max(length + from_index, 0);
    }
    auto value_to_find = interpreter.argument(0);
    for (i32 i = from_index; i < length; ++i) {
        auto element = this_object->get(i).value_or(js_undefined());
        if (interpreter.exception())
            return {};
        if (same_value_zero(interpreter, element, value_to_find))
            return Value(true);
    }
    return Value(false);
}

JS_DEFINE_NATIVE_FUNCTION(ArrayPrototype::find)
{
    auto result = js_undefined();
    for_each_item(
        interpreter, global_object, "find", [&](auto, auto value, auto callback_result) {
            if (callback_result.to_boolean()) {
                result = value;
                return IterationDecision::Break;
            }
            return IterationDecision::Continue;
        },
        false);
    return result;
}

JS_DEFINE_NATIVE_FUNCTION(ArrayPrototype::find_index)
{
    auto result_index = -1;
    for_each_item(
        interpreter, global_object, "findIndex", [&](auto index, auto, auto callback_result) {
            if (callback_result.to_boolean()) {
                result_index = index;
                return IterationDecision::Break;
            }
            return IterationDecision::Continue;
        },
        false);
    return Value(result_index);
}

JS_DEFINE_NATIVE_FUNCTION(ArrayPrototype::some)
{
    auto result = false;
    for_each_item(interpreter, global_object, "some", [&](auto, auto, auto callback_result) {
        if (callback_result.to_boolean()) {
            result = true;
            return IterationDecision::Break;
        }
        return IterationDecision::Continue;
    });
    return Value(result);
}

JS_DEFINE_NATIVE_FUNCTION(ArrayPrototype::every)
{
    auto result = true;
    for_each_item(interpreter, global_object, "every", [&](auto, auto, auto callback_result) {
        if (!callback_result.to_boolean()) {
            result = false;
            return IterationDecision::Break;
        }
        return IterationDecision::Continue;
    });
    return Value(result);
}

JS_DEFINE_NATIVE_FUNCTION(ArrayPrototype::splice)
{
    auto* this_object = interpreter.this_value(global_object).to_object(interpreter, global_object);
    if (!this_object)
        return {};

    auto initial_length = get_length(interpreter, *this_object);
    if (interpreter.exception())
        return {};

    auto relative_start = interpreter.argument(0).to_i32(interpreter);
    if (interpreter.exception())
        return {};

    size_t actual_start;

    if (relative_start < 0)
        actual_start = max((ssize_t)initial_length + relative_start, (ssize_t)0);
    else
        actual_start = min((size_t)relative_start, initial_length);

    size_t insert_count = 0;
    size_t actual_delete_count = 0;

    if (interpreter.argument_count() == 1) {
        actual_delete_count = initial_length - actual_start;
    } else if (interpreter.argument_count() >= 2) {
        insert_count = interpreter.argument_count() - 2;
        i32 delete_count = interpreter.argument(1).to_i32(interpreter);
        if (interpreter.exception())
            return {};

        actual_delete_count = min((size_t)max(delete_count, 0), initial_length - actual_start);
    }

    size_t new_length = initial_length + insert_count - actual_delete_count;

    if (new_length > MAX_ARRAY_LIKE_INDEX)
        return interpreter.throw_exception<TypeError>(ErrorType::ArrayMaxSize);

    auto removed_elements = Array::create(global_object);

    for (size_t i = 0; i < actual_delete_count; ++i) {
        auto value = this_object->get(actual_start + i);
        if (interpreter.exception())
            return {};

        removed_elements->indexed_properties().append(value);
    }

    if (insert_count < actual_delete_count) {
        for (size_t i = actual_start; i < initial_length - actual_delete_count; ++i) {
            auto from = this_object->get(i + actual_delete_count);
            if (interpreter.exception())
                return {};

            auto to = i + insert_count;

            if (!from.is_empty()) {
                this_object->put(to, from);
            } else {
                this_object->delete_property(to);
            }
            if (interpreter.exception())
                return {};
        }

        for (size_t i = initial_length; i > new_length; --i) {
            this_object->delete_property(i - 1);
            if (interpreter.exception())
                return {};
        }
    } else if (insert_count > actual_delete_count) {
        for (size_t i = initial_length - actual_delete_count; i > actual_start; --i) {
            auto from = this_object->get(i + actual_delete_count - 1);
            if (interpreter.exception())
                return {};

            auto to = i + insert_count - 1;

            if (!from.is_empty()) {
                this_object->put(to, from);
            } else {
                this_object->delete_property(to);
            }
            if (interpreter.exception())
                return {};
        }
    }

    for (size_t i = 0; i < insert_count; ++i) {
        this_object->put(actual_start + i, interpreter.argument(i + 2));
        if (interpreter.exception())
            return {};
    }

    this_object->put("length", Value((i32)new_length));
    if (interpreter.exception())
        return {};

    return removed_elements;
}

JS_DEFINE_NATIVE_FUNCTION(ArrayPrototype::fill)
{
    auto* this_object = interpreter.this_value(global_object).to_object(interpreter, global_object);
    if (!this_object)
        return {};

    ssize_t length = get_length(interpreter, *this_object);
    if (interpreter.exception())
        return {};

    ssize_t relative_start = 0;
    ssize_t relative_end = length;

    if (interpreter.argument_count() >= 2) {
        relative_start = interpreter.argument(1).to_i32(interpreter);
        if (interpreter.exception())
            return {};
    }

    if (interpreter.argument_count() >= 3) {
        relative_end = interpreter.argument(2).to_i32(interpreter);
        if (interpreter.exception())
            return {};
    }

    size_t from, to;

    if (relative_start < 0)
        from = max(length + relative_start, 0L);
    else
        from = min(relative_start, length);

    if (relative_end < 0)
        to = max(length + relative_end, 0L);
    else
        to = min(relative_end, length);

    for (size_t i = from; i < to; i++) {
        this_object->put(i, interpreter.argument(0));
        if (interpreter.exception())
            return {};
    }

    return this_object;
}

}
