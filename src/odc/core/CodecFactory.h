/*
 * (C) Copyright 1996-2012 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Simon Smart
/// @date January 2019

#ifndef odc_core_CodecFactory_H
#define odc_core_CodecFactory_H

#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>

#include "eckit/memory/NonCopyable.h"

#include "odc/core/Exceptions.h"

namespace odc {
namespace core {

//----------------------------------------------------------------------------------------------------------------------

class CodecBuilderBase;
class Codec;
template <typename ByteOrder> class DataStream;
class SameByteOrder;
class OtherByteOrder;


class CodecFactory : private eckit::NonCopyable {

public: // methods

    CodecFactory();
    ~CodecFactory();

    static CodecFactory& instance();

    void enregister(const std::string& name, CodecBuilderBase& builder);
    void deregister(const std::string& name, CodecBuilderBase& builder);

    template <typename ByteOrder>
    std::unique_ptr<Codec> build(const std::string& name) const;

private: // members

    mutable std::mutex m_;
    std::map<std::string, std::reference_wrapper<CodecBuilderBase>> builders_;
};


//----------------------------------------------------------------------------------------------------------------------


class CodecBuilderBase {

protected: // methods

    CodecBuilderBase(const std::string& name);
    ~CodecBuilderBase();

public: // methods

    virtual std::unique_ptr<Codec> make(const SameByteOrder&) const = 0;
    virtual std::unique_ptr<Codec> make(const OtherByteOrder&) const = 0;

private: // members

    std::string name_;
};

//----------------------------------------------------------------------------------------------------------------------

template <template <typename> class CODEC>
class CodecBuilder : public CodecBuilderBase {

public: // methods

    CodecBuilder() : CodecBuilderBase(CODEC<SameByteOrder>::codec_name) {}
    ~CodecBuilder() {}

private: // methods

    std::unique_ptr<Codec> make(const SameByteOrder&) const override {
        return std::unique_ptr<Codec>(new CODEC<SameByteOrder>());
    }
    std::unique_ptr<Codec> make(const OtherByteOrder&) const override {
        return std::unique_ptr<Codec>(new CODEC<OtherByteOrder>());
    }
};

//----------------------------------------------------------------------------------------------------------------------

template <typename ByteOrder>
std::unique_ptr<Codec> CodecFactory::build(const std::string& name) const {
    std::lock_guard<std::mutex> lock(m_);

    auto it = builders_.find(name);
    if (it == builders_.end()) throw ODBDecodeError(std::string("Codec '") + name +"' not found", Here());
    return it->second.get().make(ByteOrder());
}

//----------------------------------------------------------------------------------------------------------------------

} // namespace core
} // namespace odc

#endif