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

#include <LibWeb/Bindings/NodeWrapper.h>

namespace Web {
namespace Bindings {

class ElementWrapper : public NodeWrapper {
public:
    ElementWrapper(JS::GlobalObject&, Element&);
    virtual void initialize(JS::Interpreter&, JS::GlobalObject&) override;
    virtual ~ElementWrapper() override;

    Element& node();
    const Element& node() const;

private:
    virtual const char* class_name() const override { return "ElementWrapper"; }

    JS_DECLARE_NATIVE_GETTER(inner_html_getter);
    JS_DECLARE_NATIVE_SETTER(inner_html_setter);

    JS_DECLARE_NATIVE_GETTER(id_getter);
    JS_DECLARE_NATIVE_SETTER(id_setter);

    JS_DECLARE_NATIVE_FUNCTION(get_attribute);
    JS_DECLARE_NATIVE_FUNCTION(set_attribute);
};

}
}
