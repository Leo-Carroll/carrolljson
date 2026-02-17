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
		Value(const char* v) : m_Data('"' + v + '"') {}

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
			return (m_Data == "true") ? true : false;
		}

		std::string as_string() const {
			if (is_string()) {
				return m_Data;
			}
			return '"' + m_Data + '"';
		}

		Vector<Value> as_array() const {
			Vector<Value> values;

		}

		Object as_object() const {

		}

	private:
		Value parse_value() const {
			if (is_null()) {
				return Value();
			}
			if (is_number()) {
				return as_number();
			}
			if (is_bool()) {
				return as_bool();
			}
			if (is_string()) {
				return as_string();
			}
			if (is_array()) {
				return as_array();
			}
			if (is_object()) {
				return as_object();
			}
			throw std::runtime_error("Unknown value passed to parse_value.");
		}

		Vector<Value> parse_array() const {
			if (!is_array()) {
				throw std::runtime_error("parse_array called on non-array or improperly formatted array.");
			}

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
		Variable(std::string name) : m_Name(std::move(name)), m_Value() {}
		Variable(std::string name, nullptr_t) : m_Name(std::move(name)), m_Value() {}
		Variable(std::string name, bool v) : m_Name(std::move(name)), m_Value(v) {}
		Variable(std::string name, double v) : m_Name(std::move(name)), m_Value(v) {}
		Variable(std::string name, int v) : m_Name(std::move(name)), m_Value(v) {}
		Variable(std::string name, std::string v) : m_Name(std::move(name)), m_Value(std::move(v)) {}
		Variable(std::string name, const char* v) : m_Name(std::move(name)), m_Value(v) {}
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

	static void add_value_to_string(const Value& value, int depth, std::string& outputStr) {

	}
}

int main() {
	carrolljson::Value array = { 1.0, true, "hello" };
	carrolljson::Value object = carrolljson::Object{ { "key1", true }, { "key2", false } };
	std::cout << array.get_data() << '\n' << object.get_data();
}
