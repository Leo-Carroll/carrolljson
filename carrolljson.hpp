#ifndef CARROLLJSON_H
#define CARROLLJSON_H

#include <iostream>
#include <string>
#include <variant>
#include <format>
#include "DataTypes.hpp"

class carrolljson {
private:
	class Value;
	using value_t = std::variant<
		std::monostate,
		nullptr_t,
		bool,
		double,
		std::string,
		Vector<Value>,
		Map<std::string, Value>
	>;

	class Value {
	private:
		value_t m_Data;

	public:
		Value() = default;
		Value(value_t value) : m_Data(std::move(value)) {}
		Value(std::initializer_list<value_t> list) {
			Vector<Value> values;
			for (const auto& value : list) {
				values.push_back(value);
			}
			m_Data = values;
		}
		Value(std::initializer_list<std::pair<std::string, value_t>> list) {
			Map<std::string, Value> values;
			for (const auto& [key, value] : list) {
				values[key] = value;
			}
			m_Data = values;
		}

		Value& operator=(value_t val) {
			m_Data = val;
			return *this;
		}

		Value& operator=(std::initializer_list<value_t> list) {
			Vector<Value> values;
			for (const auto& value : list) {
				values.push_back(value);
			}
			m_Data = values;
			return *this;
		}

		value_t& get_data() {
			return m_Data;
		}

		const value_t& get_data() const {
			return m_Data;
		}
	};

	static std::string get_indent(int indent) {
		return std::string(indent * 4, ' ');
	}

	static std::string get_value_string(const Value& value, int& depth) {
		std::string str = "";
		std::visit([&str, &depth](const auto& val) {
			using T = std::decay_t<decltype(val)>;

			if constexpr (std::is_same_v<T, double> || std::is_same_v<T, bool>) {
				str = std::format("{}", val);
			}
			else if constexpr (std::is_same_v<T, std::string>) {
				str = std::format("\"{}\"", val);
			}
			else if constexpr (std::is_same_v<T, nullptr_t> || std::is_same_v<T, std::monostate>) {
				str = "null";
			}
			else if constexpr (std::is_same_v<T, Vector<Value>>) {
				str = "[\n";
				depth += 2;
				int zero = 0;
				for (size_t i = 0; i < val.size(); ++i) {
					str += get_indent(depth) + get_value_string(val[i], zero) + (i + 1 < val.size() ? "," : "") + "\n";
				}
				--depth;
				str += get_indent(depth) + "]";
			}
			else if constexpr (std::is_same_v<T, Map<std::string, Value>>) {
				str = "{\n";
				++depth;
				int zero = 0;
				for (size_t i = 0; i < val.size(); ++i) {
					str += get_indent(depth) + "\"" + val.get_key(i) + "\": " + get_value_string(val.get_value(i), zero) + (i + 1 < val.size() ? ", " : "") + "\n";
				}
				--depth;
				str += get_indent(depth) + "}";
			}
		}, value.get_data());
		return str;
	}

	class JsonParser {
	private:
		const std::string& jsonSource;
		size_t i;

	public:
		JsonParser(const std::string& source) : jsonSource(source), i(0) {}

		Map<std::string, Value> parse_json() {
			i = 0;
			skip_whitespace();

			if (jsonSource[i] != '{') {
				throw std::runtime_error("JSON must start with an object.");
			}

			return parse_object();
		}

	private:
		void skip_whitespace() {
			while (i < jsonSource.size() && isspace(jsonSource[i])) {
				++i;
			}
		}

		std::string parse_string() {
			std::string result = "";
			++i;
			while (jsonSource[i] != '"') {
				if (jsonSource[i] == '\\') {
					++i;
					if (i >= jsonSource.size()) {
						throw std::runtime_error("Invalid escape sequence in string.");
					}

					switch (jsonSource[i]) {
						case '"': result += '"'; break;
						case '\\': result += '\\'; break;
						case 'b': result += '\b'; break;
						case 'f': result += '\f'; break;
						case 'n': result += '\n'; break;
						case 'r': result += '\r'; break;
						case 't': result += '\t'; break;
						default: throw std::runtime_error("Invalid escape sequence in string.");
					}
				}
				else {
					result += jsonSource[i];
				}

				++i;
			}

			if (i >= jsonSource.size() || jsonSource[i] != '"') {
				throw std::runtime_error("Unterminated string in input.");
			}
			++i;

			return result;
		}

		double parse_number() {
			size_t start = i;
			bool seenDot = false;
			bool seenDash = false;

			while (i < jsonSource.size() && (isdigit(jsonSource[i]) || jsonSource[i] == '.' || jsonSource[i] == '-')) {
				if (jsonSource[i] == '.') {
					if (seenDot) {
						throw std::runtime_error("Improperly formatted number value in JSON");
					}
					seenDot = true;
				}
				else if (jsonSource[i] == '-') {
					if (seenDash) {
						throw std::runtime_error("Improperly formatted number value in JSON");
					}
					seenDash = true;
				}
				++i;
			}

			return std::stod(jsonSource.substr(start, i - start));
		}

		Vector<Value> parse_array() {
			Vector<Value> array;
			++i;
			skip_whitespace();

			while (jsonSource[i] != ']') {
				array.push_back(parse_value());
				skip_whitespace();

				if (jsonSource[i] == ',') {
					++i;
					skip_whitespace();
				}
				else if (jsonSource[i] != ']') {
					throw std::runtime_error("Expected ',' or ']' in array");
				}
			}

			++i;
			return array;
		}

		Map<std::string, Value> parse_object() {
			Map<std::string, Value> obj;
			++i;
			skip_whitespace();

			while (jsonSource[i] != '}') {
				std::string key = parse_string();
				skip_whitespace();

				if (jsonSource[i] != ':') {
					throw std::runtime_error("Expected ':' after object key");
				}
				++i;

				Value value = parse_value();
				obj[key] = value;

				skip_whitespace();
				if (jsonSource[i] == ',') {
					++i;
					skip_whitespace();
				}
				else if (jsonSource[i] != '}') {
					throw std::runtime_error("Expected ',' or '}' in object.");
				}
			}

			++i;
			return obj;
		}

		Value parse_value() {
			skip_whitespace();
			char c = jsonSource[i];

			if (c == '"') {
				return Value(parse_string());
			}
			else if (isdigit(c) || c == '-') {
				return Value(parse_number());
			}
			else if (c == '{') {
				return Value(parse_object());
			}
			else if (c == '[') {
				return Value(parse_array());
			}
			else if (jsonSource.substr(i, 4) == "true") {
				i += 4;
				return Value(true);
			}
			else if (jsonSource.substr(i, 5) == "false") {
				i += 5;
				return Value(false);
			}
			else if (jsonSource.substr(i, 4) == "null") {
				i += 4;
				return Value(nullptr);
			}

			throw std::runtime_error("Invalid JSON value.");
		}
	};

public:
	class JsonObject {
	private:
		Map<std::string, Value> m_Nodes;

	public:
		Value& operator[](std::string key) {
			return m_Nodes[key];
		}

		const Value& operator[](std::string key) const {
			return m_Nodes[key];
		}

		std::string get_json() {
			int depth = 0;
			return get_value_string(Value(m_Nodes), depth);
		}

		void parse_json(const std::string& jsonInput) {
			JsonParser parser(jsonInput);
			m_Nodes = parser.parse_json();
		}
	};
};

#endif
