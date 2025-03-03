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

#include <AK/FlyString.h>
#include <AK/Function.h>
#include <LibJS/Interpreter.h>
#include <LibJS/Runtime/GlobalObject.h>
#include <LibJS/Runtime/MathObject.h>
#include <math.h>

namespace JS {

MathObject::MathObject(GlobalObject& global_object)
    : Object(global_object.object_prototype())
{
}

void MathObject::initialize(Interpreter& interpreter, GlobalObject& global_object)
{
    Object::initialize(interpreter, global_object);
    u8 attr = Attribute::Writable | Attribute::Configurable;
    define_native_function("abs", abs, 1, attr);
    define_native_function("random", random, 0, attr);
    define_native_function("sqrt", sqrt, 1, attr);
    define_native_function("floor", floor, 1, attr);
    define_native_function("ceil", ceil, 1, attr);
    define_native_function("round", round, 1, attr);
    define_native_function("max", max, 2, attr);
    define_native_function("min", min, 2, attr);
    define_native_function("trunc", trunc, 1, attr);
    define_native_function("sin", sin, 1, attr);
    define_native_function("cos", cos, 1, attr);
    define_native_function("tan", tan, 1, attr);
    define_native_function("pow", pow, 2, attr);
    define_native_function("exp", exp, 1, attr);
    define_native_function("expm1", expm1, 1, attr);
    define_native_function("sign", sign, 1, attr);
    define_native_function("clz32", clz32, 1, attr);

    define_property("E", Value(M_E), 0);
    define_property("LN2", Value(M_LN2), 0);
    define_property("LN10", Value(M_LN10), 0);
    define_property("LOG2E", Value(log2(M_E)), 0);
    define_property("LOG10E", Value(log10(M_E)), 0);
    define_property("PI", Value(M_PI), 0);
    define_property("SQRT1_2", Value(M_SQRT1_2), 0);
    define_property("SQRT2", Value(M_SQRT2), 0);
}

MathObject::~MathObject()
{
}

JS_DEFINE_NATIVE_FUNCTION(MathObject::abs)
{
    auto number = interpreter.argument(0).to_number(interpreter);
    if (interpreter.exception())
        return {};
    if (number.is_nan())
        return js_nan();
    return Value(number.as_double() >= 0 ? number.as_double() : -number.as_double());
}

Value MathObject::random(Interpreter&, GlobalObject&)
{
#ifdef __serenity__
    double r = (double)arc4random() / (double)UINT32_MAX;
#else
    double r = (double)rand() / (double)RAND_MAX;
#endif
    return Value(r);
}

JS_DEFINE_NATIVE_FUNCTION(MathObject::sqrt)
{
    auto number = interpreter.argument(0).to_number(interpreter);
    if (interpreter.exception())
        return {};
    if (number.is_nan())
        return js_nan();
    return Value(::sqrt(number.as_double()));
}

JS_DEFINE_NATIVE_FUNCTION(MathObject::floor)
{
    auto number = interpreter.argument(0).to_number(interpreter);
    if (interpreter.exception())
        return {};
    if (number.is_nan())
        return js_nan();
    return Value(::floor(number.as_double()));
}

JS_DEFINE_NATIVE_FUNCTION(MathObject::ceil)
{
    auto number = interpreter.argument(0).to_number(interpreter);
    if (interpreter.exception())
        return {};
    if (number.is_nan())
        return js_nan();
    return Value(::ceil(number.as_double()));
}

JS_DEFINE_NATIVE_FUNCTION(MathObject::round)
{
    auto number = interpreter.argument(0).to_number(interpreter);
    if (interpreter.exception())
        return {};
    if (number.is_nan())
        return js_nan();
    return Value(::round(number.as_double()));
}

JS_DEFINE_NATIVE_FUNCTION(MathObject::max)
{
    if (!interpreter.argument_count())
        return js_negative_infinity();

    auto max = interpreter.argument(0).to_number(interpreter);
    if (interpreter.exception())
        return {};
    for (size_t i = 1; i < interpreter.argument_count(); ++i) {
        auto cur = interpreter.argument(i).to_number(interpreter);
        if (interpreter.exception())
            return {};
        max = Value(cur.as_double() > max.as_double() ? cur : max);
    }
    return max;
}

JS_DEFINE_NATIVE_FUNCTION(MathObject::min)
{
    if (!interpreter.argument_count())
        return js_infinity();

    auto min = interpreter.argument(0).to_number(interpreter);
    if (interpreter.exception())
        return {};
    for (size_t i = 1; i < interpreter.argument_count(); ++i) {
        auto cur = interpreter.argument(i).to_number(interpreter);
        if (interpreter.exception())
            return {};
        min = Value(cur.as_double() < min.as_double() ? cur : min);
    }
    return min;
}

JS_DEFINE_NATIVE_FUNCTION(MathObject::trunc)
{
    auto number = interpreter.argument(0).to_number(interpreter);
    if (interpreter.exception())
        return {};
    if (number.is_nan())
        return js_nan();
    if (number.as_double() < 0)
        return MathObject::ceil(interpreter, global_object);
    return MathObject::floor(interpreter, global_object);
}

JS_DEFINE_NATIVE_FUNCTION(MathObject::sin)
{
    auto number = interpreter.argument(0).to_number(interpreter);
    if (interpreter.exception())
        return {};
    if (number.is_nan())
        return js_nan();
    return Value(::sin(number.as_double()));
}

JS_DEFINE_NATIVE_FUNCTION(MathObject::cos)
{
    auto number = interpreter.argument(0).to_number(interpreter);
    if (interpreter.exception())
        return {};
    if (number.is_nan())
        return js_nan();
    return Value(::cos(number.as_double()));
}

JS_DEFINE_NATIVE_FUNCTION(MathObject::tan)
{
    auto number = interpreter.argument(0).to_number(interpreter);
    if (interpreter.exception())
        return {};
    if (number.is_nan())
        return js_nan();
    return Value(::tan(number.as_double()));
}

JS_DEFINE_NATIVE_FUNCTION(MathObject::pow)
{
    return JS::exp(interpreter, interpreter.argument(0), interpreter.argument(1));
}

JS_DEFINE_NATIVE_FUNCTION(MathObject::exp)
{
    auto number = interpreter.argument(0).to_number(interpreter);
    if (interpreter.exception())
        return {};
    if (number.is_nan())
        return js_nan();
    return Value(::pow(M_E, number.as_double()));
}

JS_DEFINE_NATIVE_FUNCTION(MathObject::expm1)
{
    auto number = interpreter.argument(0).to_number(interpreter);
    if (interpreter.exception())
        return {};
    if (number.is_nan())
        return js_nan();
    return Value(::pow(M_E, number.as_double()) - 1);
}

JS_DEFINE_NATIVE_FUNCTION(MathObject::sign)
{
    auto number = interpreter.argument(0).to_number(interpreter);
    if (interpreter.exception())
        return {};
    if (number.is_positive_zero())
        return Value(0);
    if (number.is_negative_zero())
        return Value(-0.0);
    if (number.as_double() > 0)
        return Value(1);
    if (number.as_double() < 0)
        return Value(-1);
    return js_nan();
}

JS_DEFINE_NATIVE_FUNCTION(MathObject::clz32)
{
    auto number = interpreter.argument(0).to_number(interpreter);
    if (interpreter.exception())
        return {};
    if (!number.is_finite_number() || (unsigned)number.as_double() == 0)
        return Value(32);
    return Value(__builtin_clz((unsigned)number.as_double()));
}

}
