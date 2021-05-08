#pragma once

#include "config.hpp"
#include "FileWrapper.hpp"

namespace __cpplib {

using namespace __config;

namespace HardDisk {

    struct Record {
        using Self          = Record;
        using offset_type   = FileWrapper::offset_type;

        offset_type offset;


        Record(): offset(static_cast<offset_type>(-1)) {}
        Record(offset_type __offset): offset(__offset) {}
        Record(Self &&other): offset(other.offset) { other.offset = static_cast<offset_type>(-1); }
        Record(const Self &other): offset(other.offset) {}

        auto operator = (Self &&rhs) -> Self& {
            offset = rhs.offset;
            rhs.offset = static_cast<offset_type>(-1);
            return *this;
        }
        auto operator = (const Self &rhs) -> Self& { return offset = rhs.offset, *this; }

        auto empty() const -> bool { return offset == static_cast<offset_type>(-1); }

        template <typename T>
        auto get(FileWrapper &io) const -> T {
            if (empty()) throw "try to load from an empty record";
            T value;
            load(io, value);
            return value;
        }

        template <typename T>
        auto save(FileWrapper &io, const T &value) -> Self {
            if (empty())
                return Record(offset = io.append(value));
            io.seek(offset);
            io.write(value);
            return *this;
        }
        template <typename T>
        auto load(FileWrapper &io, T &value) const -> Self {
            if (empty()) throw "try to load from an empty record";
            io.seek(offset);
            io.read(value);
            return *this;
        }
    };

    template <typename T>
    struct RecordPool {
        Vec<Record> recs;

        RecordPool(): recs() {}

        auto alloc() -> Record {
            if (recs.empty()) return Record();
            Record tmp = recs.back();
            recs.pop_back();
            return tmp;
        }

        auto dealloc(const Record &rec) -> void {
            recs.push_back(rec);
        }
    };

    // template <typename T>
    // struct Record {
    // 	using Self			= Record;
    // 	using value_type	= T;
    // 	using offset_type	= FileWrapper::offset_type;

    // 	offset_type offset;


    // 	Record(): offset(static_cast<offset_type>(-1)) {}
    // 	Record(offset_type __offset): offset(__offset) {}
    // 	Record(const Self &other): offset(other.offset) {}

    // 	auto operator = (const Self &rhs) -> Self& { return offset = rhs.offset, *this; }

    //	auto empty() const -> bool { return offset == static_cast<offset_type>(-1); }

    // 	auto save(FileWrapper &io, const value_type &value) const -> void {
    // 		io.seek(offset);
    // 		io.write(value);
    // 	}
    // 	auto load(FileWrapper &io, value_type &value) const -> void {
    // 		io.seek(offset);
    // 		io.read(value);
    // 	}
    // };

}

}
