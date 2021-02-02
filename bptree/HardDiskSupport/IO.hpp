#pragma once

#include "../../config.hpp"

#define C_STYLE_HardDiskIO

namespace HardDisk {

#ifdef C_STYLE_HardDiskIO

	struct IO {
		using Self			= IO;
		using offset_type	= u32;

		std::FILE *file;

		IO(): file(nullptr) { }
		explicit IO(const char *filename) { open(filename); }
		explicit IO(const str &filename) { open(filename.c_str()); }

		IO(Self &&other): file(other.file) { other.file = nullptr; }
		IO(const Self &) = delete;

		~IO() { if (file != nullptr) std::fclose(file); }

		auto open(const char *filename) -> bool {
			if (file = std::fopen(filename, "r+b"); file == nullptr)
				return file = std::fopen(filename, "w+b"), false;
			return true;
		}
		auto open(const str &filename) -> bool { return open(filename.c_str()); }

		auto is_open() const -> bool { return file != nullptr; }
		auto close() -> void { std::fclose(file); }
		auto flush() -> void { std::fflush(file); }

		auto tell() const -> offset_type { return std::ftell(file); }
		auto seek(offset_type offset) -> void { std::fseek(file, offset, SEEK_SET); }
		auto seek_end() -> void { std::fseek(file, 0, SEEK_END); }

		template <typename T>
		auto read() -> T {
			T value;
			std::fread(std::addressof(value), sizeof(T), 1, file);
			return value;
		}

		template <typename T>
		auto read(T &obj) -> void {
			std::fread(std::addressof(obj), sizeof(T), 1, file);
		}

		template <typename T>
		auto write(const T &obj) -> void {
			std::fwrite(std::addressof(obj), sizeof(T), 1, file);
		}

		template <typename T>
		auto append(const T &obj) -> offset_type {
			seek_end();
			offset_type offset = tell();
			write(obj);
			return offset;
		}
	};

#else

	struct IO {
		using Self			= IO;
		using offset_type	= u32;

		std::fstream file;

		IO(): file() { }
		explicit IO(const char *filename): file(filename) { }
		explicit IO(const str &filename): file(filename) { }

		IO(Self &&other): file(std::move(other.file)) { }
		IO(const Self &) = delete;


		auto open(const char *filename) -> void { file.open(filename); }
		auto open(const str &filename) -> void {file.open(filename); }

		auto is_open() const -> bool { return file.is_open(); }
		auto close() -> void { file.close(); }
		auto flush() -> void { file.flush(); }

		auto tell() const -> offset_type { return file.tellp(); }
		auto seek(offset_type offset) -> void { file.seekp(offset); }
		auto seek_end() -> void { file.seekp(0, std::fstream::end); }

		template <typename T>
		auto read() -> T {
			T value;
			file.read(reinterpret_cast<char*>(std::addressof(value)), sizeof(T));
			return value;
		}

		template <typename T>
		auto read(T &obj) -> void {
			file.read(reinterpret_cast<char*>(std::addressof(obj)), sizeof(T));
		}

		template <typename T>
		auto write(const T &obj) -> void {
			file.write(reinterpret_cast<char*>(std::addressof(obj)), sizeof(T));
		}

		template <typename T>
			auto append(const T &obj) -> offset_type {
				seek_end();
				offset_type offset = tell();
				write(obj);
				return offset;
			}
	};

#endif

}
