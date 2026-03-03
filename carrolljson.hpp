#ifndef CARROLLJSON_H
#define CARROLLJSON_H

#include "DataTypes.hpp"
#include <variant>
#include <format>
#include <string>

class carrolljson {
private:
	class JsonValue;

	using value_t = std::variant<
		std::monostate,
		nullptr_t,
		bool,
		double,
		std::string,
		Vector<JsonValue>,
		HashMap<std::string, JsonValue>
	>;

	class JsonValue {
	private:
		value_t m_Data;

	public:
		JsonValue() = default;
		JsonValue(value_t value) : m_Data(std::move(value)) {}

		JsonValue(const std::initializer_list<value_t>& list) {
			Vector<JsonValue> values;
			for (const auto& value : list) {
				values.push_back(value);
			}
			m_Data = values;
		}

		JsonValue(const std::initializer_list<std::pair<std::string, value_t>>& list) {
			HashMap<std::string, JsonValue> map;
			for (const auto& [key, value] : list) {
				map[key] = value;
			}
			m_Data = map;
		}

		JsonValue& operator=(const value_t& val) {
			m_Data = val;
			return *this;
		}

		JsonValue& operator=(value_t&& val) {
			m_Data = std::move(val);
			return *this;
		}

		value_t& get_data() {
			return m_Data;
		}

		const value_t& get_data() const {
			return m_Data;
		}
	};

	static std::string get_indent(int depth) {
		return std::string(depth * 4, ' ');
	}

	static std::string get_value_string(const JsonValue& value, int depth) {
		std::string valueStr = "";
		std::visit([&valueStr, &depth](const auto& data) {
			using T = std::decay_t<decltype(data)>;

			if constexpr (std::is_same_v<T, double> || std::is_same_v<T, bool>) {
				valueStr = std::format("{}", data);
			}
			else if constexpr (std::is_same_v<T, std::string>) {
				valueStr = std::format("\"{}\"", data);
			}
			else if constexpr (std::is_same_v<T, nullptr_t> || std::is_same_v<T, std::monostate>) {
				valueStr = "null";
			}
			else if constexpr (std::is_same_v<T, Vector<JsonValue>>) {
				valueStr = "[\n";
				depth += 2;
				for (size_t i = 0; i < data.size(); ++i) {
					valueStr += get_indent(depth) + get_value_string(data[i], 0) + (i + 1 < data.size() ? "," : "") + "\n";
				}
				--depth;
				valueStr += get_indent(depth) + "]";
			}
			else if constexpr (std::is_same_v<T, HashMap<std::string, JsonValue>>) {
				valueStr = "{\n";
				depth += 1;
				size_t i = 0;
				for (const auto& [key, val] : data) {
					valueStr += get_indent(depth) + "\"" + key + "\": " + get_value_string(val, 0) + (i + 1 < data.size() ? ", \n" : "\n");
					i++;
				}
				--depth;
				valueStr += get_indent(depth) + "}";
			}
		}, value.get_data());
		return valueStr;
	}

	class JsonParser {
	private:
		const std::string& m_JsonSource;
		size_t m_Idx;

	public:
		JsonParser(const std::string& source) : m_JsonSource(source) {
			m_Idx = 0;
		}

		HashMap<std::string, JsonValue> parse_json() {
			m_Idx = 0;
			
		}

	private:
		void skip_whitespace() {
			while (m_Idx < m_JsonSource.size() && isspace(m_JsonSource[m_Idx])) {
				++m_Idx;
			}
		}

		std::string parse_string() {
			std::string result = "";
			++m_Idx;
			while (m_JsonSource[m_Idx] != '"') {
				if (m_JsonSource[m_Idx] == '\\') {
					++m_Idx;
					if (m_Idx >= m_JsonSource.size()) {
						throw std::runtime_error("Invalid escape sequence in string.");
					}

					switch (m_JsonSource[m_Idx]) {
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
					result += m_JsonSource[m_Idx];
				}

				++m_Idx;
			}

			if (m_Idx >= m_JsonSource.size() || m_JsonSource[m_Idx] != '"') {
				throw std::runtime_error("Unterminated string in input.");
			}
			++m_Idx;

			return result;
		}

		double parse_number() {
			size_t start = m_Idx;
			bool seenDot = false;
			bool seenDash = false;

			while (m_Idx < m_JsonSource.size() && (isdigit(m_JsonSource[m_Idx]) || m_JsonSource[m_Idx] == '.' || m_JsonSource[m_Idx] == '-')) {
				if (m_JsonSource[m_Idx] == '.') {
					if (seenDot) {
						throw std::runtime_error("Improperly formatted number value in JSON");
					}
					seenDot = true;
				}
				else if (m_JsonSource[m_Idx] == '-') {
					if (seenDash) {
						throw std::runtime_error("Improperly formatted number value in JSON");
					}
					seenDash = true;
				}
				++m_Idx;
			}

			return std::stod(m_JsonSource.substr(start, m_Idx - start));
		}

		Vector<JsonValue> parse_array() {
			Vector<JsonValue> array;
			++m_Idx;
			skip_whitespace();

			while (m_JsonSource[m_Idx] != ']') {
				array.push_back(parse_value());
				skip_whitespace();

				if (m_JsonSource[m_Idx] == ',') {
					++m_Idx;
					skip_whitespace();
				}
				else if (m_JsonSource[m_Idx] != ']') {
					throw std::runtime_error("Expected ',' or ']' in array");
				}
			}

			++m_Idx;
			return array;
		}

		HashMap<std::string, JsonValue> parse_object() {
			HashMap<std::string, JsonValue> obj;
			++m_Idx;
			skip_whitespace();

			while (m_JsonSource[m_Idx] != '}') {
				std::string key = parse_string();
				skip_whitespace();

				if (m_JsonSource[m_Idx] != ':') {
					throw std::runtime_error("Expected ':' after object key");
				}
				++m_Idx;

				JsonValue value = parse_value();
				obj[key] = value;

				skip_whitespace();
				if (m_JsonSource[m_Idx] == ',') {
					++m_Idx;
					skip_whitespace();
				}
				else if (m_JsonSource[m_Idx] != '}') {
					throw std::runtime_error("Expected ',' or '}' in object.");
				}
			}

			++m_Idx;
			return obj;
		}

		JsonValue parse_value() {
			skip_whitespace();
			char c = m_JsonSource[m_Idx];

			if (c == '"') {
				return JsonValue(parse_string());
			}
			else if (isdigit(c) || c == '-') {
				return JsonValue(parse_number());
			}
			else if (c == '{') {
				return JsonValue(parse_object());
			}
			else if (c == '[') {
				return JsonValue(parse_array());
			}
			else if (m_JsonSource.substr(m_Idx, 4) == "true") {
				m_Idx += 4;
				return JsonValue(true);
			}
			else if (m_JsonSource.substr(m_Idx, 5) == "false") {
				m_Idx += 5;
				return JsonValue(false);
			}
			else if (m_JsonSource.substr(m_Idx, 4) == "null") {
				m_Idx += 4;
				return JsonValue(nullptr);
			}

			throw std::runtime_error("Invalid JSON value.");
		}
	};

public:
	class JsonObject {
	private:
		HashMap<std::string, JsonValue> m_Nodes;

	public:
		JsonValue& operator[](const std::string& key) {
			return m_Nodes[key];
		}

		const JsonValue& operator[](const std::string& key) const {
			return m_Nodes[key];
		}

		std::string get_json() {
			return get_value_string(JsonValue(m_Nodes), 0);
		}

		void parse_json(const std::string& jsonSource) {
			JsonParser parser(jsonSource);
			m_Nodes = parser.parse_json();
		}
	};
};

#endif
