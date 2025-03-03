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

#pragma once

#include <AK/Assertions.h>
#include <AK/Forward.h>
#include <AK/LogStream.h>
#include <AK/Types.h>
#include <LibJS/Forward.h>
#include <math.h>

// 2 ** 53 - 1
static constexpr double MAX_ARRAY_LIKE_INDEX = 9007199254740991.0;

namespace JS {

class Value {
public:
    enum class Type {
        Empty,
        Undefined,
        Null,
        Number,
        String,
        Object,
        Boolean,
        Symbol,
        Accessor,
        BigInt,
    };

    enum class PreferredType {
        Default,
        String,
        Number,
    };

    bool is_empty() const { return m_type == Type::Empty; }
    bool is_undefined() const { return m_type == Type::Undefined; }
    bool is_null() const { return m_type == Type::Null; }
    bool is_number() const { return m_type == Type::Number; }
    bool is_string() const { return m_type == Type::String; }
    bool is_object() const { return m_type == Type::Object; }
    bool is_boolean() const { return m_type == Type::Boolean; }
    bool is_symbol() const { return m_type == Type::Symbol; }
    bool is_accessor() const { return m_type == Type::Accessor; };
    bool is_bigint() const { return m_type == Type::BigInt; };
    bool is_cell() const { return is_string() || is_accessor() || is_object() || is_bigint() || is_symbol(); }
    bool is_array() const;
    bool is_function() const;

    bool is_nan() const { return is_number() && __builtin_isnan(as_double()); }
    bool is_infinity() const { return is_number() && __builtin_isinf(as_double()); }
    bool is_positive_infinity() const { return is_number() && __builtin_isinf_sign(as_double()) > 0; }
    bool is_negative_infinity() const { return is_number() && __builtin_isinf_sign(as_double()) < 0; }
    bool is_positive_zero() const { return is_number() && 1.0 / as_double() == INFINITY; }
    bool is_negative_zero() const { return is_number() && 1.0 / as_double() == -INFINITY; }
    bool is_integer() const { return is_finite_number() && (i32)as_double() == as_double(); }
    bool is_finite_number() const
    {
        if (!is_number())
            return false;
        auto number = as_double();
        return !__builtin_isnan(number) && !__builtin_isinf(number);
    }

    Value()
        : m_type(Type::Empty)
    {
    }

    explicit Value(bool value)
        : m_type(Type::Boolean)
    {
        m_value.as_bool = value;
    }

    explicit Value(double value)
        : m_type(Type::Number)
    {
        m_value.as_double = value;
    }

    explicit Value(unsigned value)
        : m_type(Type::Number)
    {
        m_value.as_double = static_cast<double>(value);
    }

    explicit Value(i32 value)
        : m_type(Type::Number)
    {
        m_value.as_double = value;
    }

    Value(Object* object)
        : m_type(object ? Type::Object : Type::Null)
    {
        m_value.as_object = object;
    }

    Value(PrimitiveString* string)
        : m_type(Type::String)
    {
        m_value.as_string = string;
    }

    Value(Symbol* symbol)
        : m_type(Type::Symbol)
    {
        m_value.as_symbol = symbol;
    }

    Value(Accessor* accessor)
        : m_type(Type::Accessor)
    {
        m_value.as_accessor = accessor;
    }

    Value(BigInt* bigint)
        : m_type(Type::BigInt)
    {
        m_value.as_bigint = bigint;
    }

    explicit Value(Type type)
        : m_type(type)
    {
    }

    Type type() const { return m_type; }

    double as_double() const
    {
        ASSERT(type() == Type::Number);
        return m_value.as_double;
    }

    bool as_bool() const
    {
        ASSERT(type() == Type::Boolean);
        return m_value.as_bool;
    }

    Object& as_object()
    {
        ASSERT(type() == Type::Object);
        return *m_value.as_object;
    }

    const Object& as_object() const
    {
        ASSERT(type() == Type::Object);
        return *m_value.as_object;
    }

    PrimitiveString& as_string()
    {
        ASSERT(is_string());
        return *m_value.as_string;
    }

    const PrimitiveString& as_string() const
    {
        ASSERT(is_string());
        return *m_value.as_string;
    }

    Symbol& as_symbol()
    {
        ASSERT(is_symbol());
        return *m_value.as_symbol;
    }

    const Symbol& as_symbol() const
    {
        ASSERT(is_symbol());
        return *m_value.as_symbol;
    }

    Cell* as_cell()
    {
        ASSERT(is_cell());
        return m_value.as_cell;
    }

    Accessor& as_accessor()
    {
        ASSERT(is_accessor());
        return *m_value.as_accessor;
    }

    BigInt& as_bigint()
    {
        ASSERT(is_bigint());
        return *m_value.as_bigint;
    }

    Array& as_array();
    Function& as_function();

    i32 as_i32() const;
    size_t as_size_t() const;

    String to_string(Interpreter&) const;
    PrimitiveString* to_primitive_string(Interpreter&);
    Value to_primitive(Interpreter&, PreferredType preferred_type = PreferredType::Default) const;
    Object* to_object(Interpreter&, GlobalObject&) const;
    Value to_numeric(Interpreter&) const;
    Value to_number(Interpreter&) const;
    BigInt* to_bigint(Interpreter&) const;
    double to_double(Interpreter&) const;
    i32 to_i32(Interpreter&) const;
    size_t to_size_t(Interpreter&) const;
    bool to_boolean() const;

    String to_string_without_side_effects() const;

    Value value_or(Value fallback) const
    {
        if (is_empty())
            return fallback;
        return *this;
    }

private:
    Type m_type { Type::Empty };

    union {
        bool as_bool;
        double as_double;
        PrimitiveString* as_string;
        Symbol* as_symbol;
        Object* as_object;
        Cell* as_cell;
        Accessor* as_accessor;
        BigInt* as_bigint;
    } m_value;
};

inline Value js_undefined()
{
    return Value(Value::Type::Undefined);
}

inline Value js_null()
{
    return Value(Value::Type::Null);
}

inline Value js_nan()
{
    return Value(NAN);
}

inline Value js_infinity()
{
    return Value(INFINITY);
}

inline Value js_negative_infinity()
{
    return Value(-INFINITY);
}

Value greater_than(Interpreter&, Value lhs, Value rhs);
Value greater_than_equals(Interpreter&, Value lhs, Value rhs);
Value less_than(Interpreter&, Value lhs, Value rhs);
Value less_than_equals(Interpreter&, Value lhs, Value rhs);
Value bitwise_and(Interpreter&, Value lhs, Value rhs);
Value bitwise_or(Interpreter&, Value lhs, Value rhs);
Value bitwise_xor(Interpreter&, Value lhs, Value rhs);
Value bitwise_not(Interpreter&, Value);
Value unary_plus(Interpreter&, Value);
Value unary_minus(Interpreter&, Value);
Value left_shift(Interpreter&, Value lhs, Value rhs);
Value right_shift(Interpreter&, Value lhs, Value rhs);
Value unsigned_right_shift(Interpreter&, Value lhs, Value rhs);
Value add(Interpreter&, Value lhs, Value rhs);
Value sub(Interpreter&, Value lhs, Value rhs);
Value mul(Interpreter&, Value lhs, Value rhs);
Value div(Interpreter&, Value lhs, Value rhs);
Value mod(Interpreter&, Value lhs, Value rhs);
Value exp(Interpreter&, Value lhs, Value rhs);
Value in(Interpreter&, Value lhs, Value rhs);
Value instance_of(Interpreter&, Value lhs, Value rhs);

bool abstract_eq(Interpreter&, Value lhs, Value rhs);
bool strict_eq(Interpreter&, Value lhs, Value rhs);
bool same_value(Interpreter&, Value lhs, Value rhs);
bool same_value_zero(Interpreter&, Value lhs, Value rhs);
bool same_value_non_numeric(Interpreter&, Value lhs, Value rhs);
TriState abstract_relation(Interpreter&, bool left_first, Value lhs, Value rhs);
size_t length_of_array_like(Interpreter&, Value);

const LogStream& operator<<(const LogStream&, const Value&);

}
