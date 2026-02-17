#include <iostream>
#include <print>
#include <string>
#include "DataTypes.hpp"

namespace carrolljson {
	class Value;

	using Object = Map<std::string, Value>;

	class Value {
	private:
		// Helper functions
		static std::string format_bool(bool x) {
			return x ? "true" : "false";
		}

		static std::string format_double(double x) {
			return std::format("{}", x);
		}

	private:
		std::string m_Data;

	public:
		Value() : m_Data("null") {}
		Value(nullptr_t) : m_Data("null") {}
		Value(bool v) : m_Data(format_bool(v)) {}
		Value(double v) : m_Data(format_double(v)) {}
		Value(int v) : m_Data(std::to_string(v)) {}
		Value(std::string v) : m_Data('"' + std::move(v) + '"') {}
		Value(const char* v) : m_Data('"' + static_cast<std::string>(v) + '"') {}

		Value(const Vector<Value>& vals) : m_Data(array_to_string(vals)) {}
		Value(const std::initializer_list<Value>& list) {
			Vector<Value> values;
			for (const auto& val : list) {
				values.push_back(val);
			}
			m_Data = array_to_string(values);
		}

		Value(const Object& obj) : m_Data(object_to_string(obj)) {}
		Value(const std::initializer_list<std::pair<const std::string, Value>>& list) {
			Object obj;
			for (const auto& pair : list) {
				obj[pair.first] = pair.second;
			}
			m_Data = object_to_string(obj);
		}

		Value(const Value& other) : m_Data(other.m_Data) {}
		Value(Value&& other) noexcept : m_Data(std::move(other.m_Data)) {}

		Value& operator=(const Value& other) {
			if (this != &other) {
				this->m_Data = other.m_Data;
			}
			return *this;
		}

		Value& operator=(Value&& other) noexcept {
			if (this != &other) {
				this->m_Data = std::move(other.m_Data);
			}
			return *this;
		}

		std::string get_data() const {
			return m_Data;
		}

		bool is_null() const {
			return m_Data == "null";
		}

		bool is_number() const {
			try {
				double value = std::stod(m_Data);
				return true;
			}
			catch (...) {
				return false;
			}
		}

		bool is_bool() const {
			return m_Data == "true" || m_Data == "false";
		}

		bool is_string() const {
			return m_Data.front() == '"' && m_Data.back() == '"';
		}

		bool is_array() const {
			return m_Data.front() == '[' && m_Data.back() == ']';
		}

		bool is_object() const {
			return m_Data.front() == '{' && m_Data.back() == '}';
		}

		double as_number() const {
			return std::stod(m_Data);
		}

		bool as_bool() const {
			return m_Data == "true";
		}

		std::string as_string() const {
			if (is_string()) {
				return m_Data;
			}
			return '"' + m_Data + '"';
		}

		Vector<Value> as_array() const {
			return parse_array();
		}

		Object as_object() const {
			return parse_object();
		}

	private:
		static Value parse_value(std::string data) {
			Value val;
			val.m_Data = data;

			if (val.is_null()) {
				return Value();
			}
			if (val.is_number()) {
				return val.as_number();
			}
			if (val.is_bool()) {
				return val.as_bool();
			}
			if (val.is_string()) {
				return val.get_data();
			}
			if (val.is_array()) {
				return val.as_array();
			}
			if (val.is_object()) {
				return val.as_object();
			}

			throw std::runtime_error("Unknown value passed to parse_value.");
		}

		static void strip_quotations(std::string& str) {
			for (size_t i = 0; i < str.size(); ++i) {
				if (str[i] == '"') {
					str.erase(i, 1);
				}
			}
		}

		Vector<Value> parse_array() const {
			if (!is_array()) {
				throw std::runtime_error("parse_array called on non-array or improperly formatted array.");
			}
			Vector<Value> values;
			for (size_t i = 0; i < m_Data.size(); ++i) {
				if (isspace(m_Data[i]) || m_Data[i] == ',' || m_Data[i] == '[' || m_Data[i] == ']') {
					continue;
				}
				std::string valueData = "";
				while (!isspace(m_Data[i]) && m_Data[i] != ',') {
					valueData += m_Data[i++];
				}
				Value currentValue = parse_value(valueData);
				if (currentValue.is_string()) {
					strip_quotations(currentValue.m_Data);
				}
				values.push_back(currentValue);
			}
			return values;
		}

		Object parse_object() const {
			Object obj;
			std::string currKey;
			Value currVal;

			for (size_t i = 0; i < m_Data.size(); ++i) {
				if (isspace(m_Data[i]) || m_Data[i] == ',' || m_Data[i] == '{' || m_Data[i] == '}') {
					continue;
				}
				std::string data = "";
				while (!isspace(m_Data[i]) && m_Data[i] != ',') {
					data += m_Data[i++];
				}
				if (data.back() == ':') {
					currKey = data;
					strip_quotations(currKey);
					currKey.pop_back();
				}
				else {
					Value dataVal;
					dataVal.m_Data = data;
					currVal = dataVal;
				}

				if (currKey != "" && !currVal.is_null()) {
					obj[currKey] = currVal;
					currKey = "";
					currVal = nullptr;
				}
				//std::cout << dataVal.get_data() << '\n';
			}

			return obj;
		}

		static std::string array_to_string(const Vector<Value>& vals) {
			std::string result = "[ ";
			for (size_t i = 0; i < vals.size(); ++i) {
				result += vals[i].get_data();
				if (i + 1 < vals.size()) {
					result += ", ";
				}
			}
			return result + " ]";
		}

		static std::string object_to_string(const Object& obj) {
			std::string result = "{ ";
			for (size_t i = 0; i < obj.size(); ++i) {
				result += std::format("{{ \"{}\": {} }}", obj.get_key(i), obj.get_value(i).get_data());
				if (i + 1 < obj.size()) {
					result += ", ";
				}
			}
			return result + " }";
		}
	};

	class Variable {
	private:
		Value m_Value;
		std::string m_Name;

	public:
		Variable() : m_Name(""), m_Value() {}
		Variable(std::string name) : m_Name(std::move(name)), m_Value() {}
		Variable(std::string name, nullptr_t) : m_Name(std::move(name)), m_Value() {}
		Variable(std::string name, bool v) : m_Name(std::move(name)), m_Value(v) {}
		Variable(std::string name, double v) : m_Name(std::move(name)), m_Value(v) {}
		Variable(std::string name, int v) : m_Name(std::move(name)), m_Value(v) {}
		Variable(std::string name, std::string v) : m_Name(std::move(name)), m_Value(std::move(v)) {}
		Variable(std::string name, const char* v) : m_Name(std::move(name)), m_Value(v) {}
		Variable(std::string name, const Value& v) : m_Name(std::move(name)), m_Value(v) {}
		Variable(std::string name, const Vector<Value>& vals) : m_Name(std::move(name)), m_Value(vals) {}
		Variable(std::string name, const std::initializer_list<Value>& list) : m_Name(std::move(name)), m_Value(list) {}
		Variable(std::string name, const Object& obj) : m_Name(std::move(name)), m_Value(obj) {}
		Variable(std::string name, const std::initializer_list<std::pair<const std::string, Value>>& list) : m_Name(std::move(name)), m_Value(list) {}

		std::string get_name() const {
			return m_Name;
		}

		void set_name(std::string name) {
			m_Name = std::move(name);
		}

		Value get_value() const {
			return m_Value;
		}

		void set_value(Value val) {
			m_Value = std::move(val);
		}
	};

	constexpr int INDENT_SPACES = 4;
	static std::string get_indent(int depth) {
		return std::format("{:>{}}", "", depth * INDENT_SPACES);
	}

	static std::string get_value_string(const Value& value) {
		if (value.is_null() || value.is_number() || value.is_bool() || value.is_string()) {
			return value.get_data();
		}
		if (value.is_array()) {
			Vector<Value> values = value.as_array();
			std::string result = "[ ";
			for (size_t i = 0; i < values.size(); ++i) {
				result += get_value_string(values[i]);
				if (i + 1 < values.size()) {
					result += ", ";
				}
			}
			return result + " ]";
		}
		if (value.is_object()) {
			Object object = value.as_object();
			std::string result = "{ ";
			for (size_t i = 0; i < object.size(); ++i) {
				result += std::format("{{ \"{}\": {} }}", object.get_key(i), object.get_value(i).get_data());
				if (i + 1 < object.size()) {
					result += ", ";
				}
			}
			return result + " }";
		}
		return value.get_data();
	}
	static void add_value_to_string(const Value& value, int depth, std::string& outputStr) {
		if (value.is_null() || value.is_number() || value.is_bool() || value.is_string()) {
			outputStr += value.get_data();
			return;
		}
		if (value.is_array()) {
			Vector<Value> values = value.as_array();
			outputStr += get_indent(depth);
			outputStr += "[\n";
			++depth;
			for (size_t i = 0; i < values.size(); ++i) {
				outputStr += get_indent(depth);
				add_value_to_string(values[i], depth, outputStr);
				if (i + 1 < values.size()) {
					outputStr += ", ";
				}
				outputStr += '\n';
			}
			--depth;
			outputStr += get_indent(depth);
			outputStr += "]";
			return;
		}
		if (value.is_object()) {
			Object object = value.as_object();
			outputStr += get_indent(depth);
			outputStr += "{\n";
			++depth;
			for (size_t i = 0; i < object.size(); ++i) {
				outputStr += get_indent(depth);
				add_value_to_string(object[object.get_key(i)], depth, outputStr);
				if (i + 1 < object.size()) {
					outputStr += ", ";
				}
				outputStr += '\n';
			}
			--depth;
			outputStr += get_indent(depth);
			outputStr += "}";
			return;
		}
		outputStr += value.get_data();
	}

	static void add_variable_to_string(const Variable& variable, int depth, std::string& outputStr) {
		outputStr += get_indent(depth);
		outputStr += std::format("\"{}\": ", variable.get_name());
		Value value = variable.get_value();

		if (value.is_null() || value.is_number() || value.is_bool() || value.is_string()) {
			outputStr += value.get_data();
			return;
		}
		if (value.is_array()) {
			Vector<Value> values = value.as_array();
			outputStr += "[\n";
			++depth;
			for (size_t i = 0; i < values.size(); ++i) {
				outputStr += get_indent(depth);
				add_value_to_string(values[i], depth, outputStr);
				if (i + 1 < values.size()) {
					outputStr += ", ";
				}
				outputStr += '\n';
			}
			--depth;
			outputStr += get_indent(depth);
			outputStr += "]";
			return;
		}
		if (value.is_object()) {
			Object object = value.as_object();
			outputStr += "{\n";
			++depth;
			for (size_t i = 0; i < object.size(); ++i) {
				outputStr += get_indent(depth);
				add_value_to_string(object[object.get_key(i)], depth, outputStr);
				if (i + 1 < object.size()) {
					outputStr += ", ";
				}
				outputStr += '\n';
			}
			--depth;
			outputStr += get_indent(depth);
			outputStr += "}";
			return;
		}
		outputStr += value.get_data();
	}

	static std::string get_json(const Vector<Variable>& variables) {
		std::string jsonData = "{\n";
		int depth = 1;

		for (size_t i = 0; i < variables.size(); ++i) {
			add_variable_to_string(variables[i], depth, jsonData);
			if (i + 1 < variables.size()) {
				jsonData += ",";
			}
			jsonData += '\n';
		}

		return jsonData + "}";
	}

	class JSONParser {
	private:
		const std::string& source;
		size_t idx;

	public:
		JSONParser(const std::string& source) : source(source), idx(0) {}

		Vector<Variable> parse_json() {
			idx = 0;
			skip_whitespace();

			if (source[idx] != '{') {
				throw std::runtime_error("JSON input must start with an object.");
			}

			Object root = parse_object();
			Vector<Variable> variables;
			for (size_t i = 0; i < root.size(); ++i) {
				variables.push_back(Variable(root.get_key(i), root.get_value(i)));
			}
			return variables;
		}

	private:
		void skip_whitespace() {
			while (idx < source.size() && isspace(source[idx])) ++idx;
		}
		
		std::string parse_string() {
			std::string result = "";
			++idx;
			while (source[idx] != '"') {
				if (source[idx] == '\\') {
					++idx;
					if (idx >= source.size()) {
						throw std::runtime_error("Invalid escape sequence in string.");
					}

					switch (source[idx]) {
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
					result += source[idx];
				}
				++idx;
			}

			if (idx >= source.size() || source[idx] != '"') {
				throw std::runtime_error("Unterminated string literal in input.");
			}
			++idx;

			return result;
		}

		double parse_number() {
			size_t start = idx;
			bool seenDot = false;
			bool seenDash = false;

			while (idx < source.size() && (isdigit(source[idx]) || source[idx] == '.' || source[idx] == '-')) {
				if (source[idx] == '.') {
					if (seenDot) {
						throw std::runtime_error("Improperly formatted numeric literal in JSON input.");
					}
					seenDot = true;
				}
				else if (source[idx] == '-') {
					if (seenDash) {
						throw std::runtime_error("Improperly formatted numeric literal in JSON input.");
					}
					seenDash = true;
				}
				++idx;
			}

			return std::stod(source.substr(start, idx - start));
		}

		Vector<Value> parse_array() {
			Vector<Value> array;
			++idx;		// Skip opening '['
			skip_whitespace();

			while (source[idx] != ']') {
				array.push_back(parse_value());
				skip_whitespace();

				if (source[idx] == ',') {
					++idx;
					skip_whitespace();
				}
				else if (source[idx] != ']') {
					throw std::runtime_error("Expected ',' or ']' in array.");
				}
			}

			++idx;
			return array;
		}

		Object parse_object() {
			Object object;
			++idx;
			skip_whitespace();

			while (source[idx] != '}') {
				std::string key = parse_string();
				skip_whitespace();

				if (source[idx] != ':') {
					throw std::runtime_error("Expected ':' after key in JSON input.");
				}
				++idx;

				Value value = parse_value();
				object[key] = value;

				skip_whitespace();
				if (source[idx] == ',') {
					++idx;
					skip_whitespace();
				}
				else if (source[idx] != '}') {
					throw std::runtime_error("Expected ',' or '}' in object in JSON input.");
				}
			}
			++idx;
			return object;
		}

		Value parse_value() {
			skip_whitespace();
			char c = source[idx];

			if (c == '"') {
				return parse_string();
			}
			if (isdigit(c) || c == '-') {
				return parse_number();
			}
			if (c == '{') {
				return parse_object();
			}
			if (c == '[') {
				return parse_array();
			}
			if (source.substr(idx, 4) == "true") {
				idx += 4;
				return true;
			}
			if (source.substr(idx, 5) == "false") {
				idx += 4;
				return false;
			}
			if (source.substr(idx, 4) == "null") {
				idx += 4;
				return nullptr;
			}

			throw std::runtime_error("Invalid JSON value in input.");
		}
	};

	static Vector<Variable> parse_json(const std::string& input) {
		JSONParser parser(input);
		return parser.parse_json();
	}
}

int main() {
	Vector<carrolljson::Variable> variables = {
		carrolljson::Variable("var1", true),
		carrolljson::Variable("var2", "Hello"),
		carrolljson::Variable("var3", Vector<carrolljson::Value>{
			carrolljson::Value(1.0),
			carrolljson::Value(2.0),
			carrolljson::Value(3.0)
		})
	};

	std::string jsonData = carrolljson::get_json(variables);
	std::cout << jsonData << '\n';
	Vector<carrolljson::Variable> parsed = carrolljson::parse_json(jsonData);

	std::string parsedData = carrolljson::get_json(parsed);
	std::cout << parsedData;
}
