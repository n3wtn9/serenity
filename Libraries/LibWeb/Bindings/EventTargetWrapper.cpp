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

#include <AK/FlyString.h>
#include <AK/Function.h>
#include <LibJS/Interpreter.h>
#include <LibJS/Runtime/Error.h>
#include <LibJS/Runtime/Function.h>
#include <LibJS/Runtime/GlobalObject.h>
#include <LibWeb/Bindings/EventListenerWrapper.h>
#include <LibWeb/Bindings/EventTargetWrapper.h>
#include <LibWeb/DOM/EventListener.h>
#include <LibWeb/DOM/EventTarget.h>

namespace Web {
namespace Bindings {

EventTargetWrapper::EventTargetWrapper(JS::GlobalObject& global_object, EventTarget& impl)
    : Wrapper(*global_object.object_prototype())
    , m_impl(impl)
{
}

void EventTargetWrapper::initialize(JS::Interpreter& interpreter, JS::GlobalObject& global_object)
{
    Wrapper::initialize(interpreter, global_object);
    define_native_function("addEventListener", add_event_listener, 2);
}

EventTargetWrapper::~EventTargetWrapper()
{
}

JS_DEFINE_NATIVE_FUNCTION(EventTargetWrapper::add_event_listener)
{
    auto* this_object = interpreter.this_value(global_object).to_object(interpreter, global_object);
    if (!this_object)
        return {};
    if (interpreter.argument_count() < 2)
        return interpreter.throw_exception<JS::TypeError>(JS::ErrorType::BadArgCountMany, "addEventListener", "two");
    auto event_name = interpreter.argument(0).to_string(interpreter);
    if (interpreter.exception())
        return {};
    auto callback = interpreter.argument(1);
    ASSERT(callback.is_object());
    ASSERT(callback.as_object().is_function());
    auto& function = callback.as_function();
    auto listener = adopt(*new EventListener(JS::make_handle(&function)));
    static_cast<EventTargetWrapper*>(this_object)->impl().add_event_listener(event_name, move(listener));
    return JS::js_undefined();
}

}
}
