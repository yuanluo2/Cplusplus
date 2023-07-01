// compile with C++14.
#include <iostream>
#include <string>
#include <stdexcept>
#include <algorithm>
#include "mysql.h"

using std::string;
using std::cout;
using std::cerr;
using namespace std::literals::string_literals;

std::string create_error_msg(const string& msg, const string& detail) {
	return __FILE__ + "    :"s + __FUNCTION__ + "()    :"s + std::to_string(__LINE__) + "    " + msg + ".\nreason: " + detail;
}

MYSQL* connectDatabase(const string& host, unsigned int port, const string& user, const string& passwd, const string& db, const string& charset) {
	MYSQL* mysql = nullptr;

	if (nullptr == (mysql = mysql_init(mysql))) {
		throw std::runtime_error{ create_error_msg("mysql_init() failed", mysql_error(mysql)) };
	}

	if (nullptr == mysql_real_connect(mysql, host.c_str(), user.c_str(), passwd.c_str(), db.c_str(), port, nullptr, 0)) {
		mysql_close(mysql);
		throw std::runtime_error{ create_error_msg("mysql_real_connect() failed", mysql_error(mysql)) };
	}

	if (0 != mysql_set_character_set(mysql, charset.c_str())) {
		mysql_close(mysql);
		throw std::runtime_error{ create_error_msg("mysql_set_character_set() failed", mysql_error(mysql)) };
	}

	return mysql;
}

class ResultSet {
	MYSQL_RES* res = nullptr;
	uint64_t rowNum = 0;
	unsigned int colNum = 0;
public:
	ResultSet(MYSQL_RES* _res)
		: res{ _res }, rowNum{ mysql_num_rows(_res) }, colNum{ mysql_num_fields(_res) }
	{}

	ResultSet(const ResultSet&) = delete;
	ResultSet& operator=(const ResultSet&) = delete;

	// used for copy elision.
	ResultSet(ResultSet&& other) noexcept {
		if (this != &other) {
			std::swap(this->res, other. res);
		}
	}

	ResultSet& operator=(ResultSet&& other) noexcept {
		if (this != &other) {
			std::swap(this->res, other.res);
			std::swap(this->rowNum, other.rowNum);
			std::swap(this->colNum, other.colNum);
		}

		return *this;
	}

	~ResultSet() {
		if (res != nullptr) {
			mysql_free_result(res);
		}
	}

	uint64_t getRowNum() const {
		return rowNum;
	}

	unsigned int getColNum() const {
		return colNum;
	}

	MYSQL_ROW fetchRow() const {
		return mysql_fetch_row(res);
	}

	MYSQL_FIELD* fetchFieldInfo(unsigned int colIndex) const {
		return mysql_fetch_field_direct(res, colIndex);
	}
};

class MysqlWrapper {
	MYSQL* mysql = nullptr;
public:
	MysqlWrapper(MYSQL* _mysql) : mysql{ _mysql } {}
	MysqlWrapper(const MysqlWrapper&) = delete;
	MysqlWrapper& operator=(const MysqlWrapper&) = delete;

	// used for copy elision.
	MysqlWrapper(MysqlWrapper&& other) noexcept {
		if (this != &other) {
			std::swap(this->mysql, other.mysql);
		}
	}

	MysqlWrapper& operator=(MysqlWrapper&& other) noexcept {
		if (this != &other) {
			std::swap(this->mysql, other.mysql);
		}

		return *this;
	}

	~MysqlWrapper() {
		if (mysql != nullptr) {
			mysql_close(mysql);
		}
	}

	uint64_t execute(const string& sql) {
		if (0 != mysql_real_query(mysql, sql.c_str(), static_cast<unsigned long>(sql.size()))) {
			throw std::runtime_error{ create_error_msg("mysql_real_query() failed", mysql_error(mysql)) };
		}

		return mysql_affected_rows(mysql);
	}

	ResultSet query(const string& sql) {
		execute(sql);

		MYSQL_RES* res = mysql_store_result(mysql);
		if (res == nullptr) {
			throw std::runtime_error{ create_error_msg("mysql_store_result() failed", mysql_error(mysql)) };
		}

		return ResultSet{ res };    // copy elision.
	}
};

/*
-- blog.m_blog definition

CREATE TABLE `m_blog` (
  `id` bigint NOT NULL AUTO_INCREMENT,
  `user_id` bigint NOT NULL,
  `title` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NOT NULL,
  `description` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NOT NULL,
  `content` text CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci,
  `created` datetime NOT NULL ON UPDATE CURRENT_TIMESTAMP,
  `status` tinyint DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=52 DEFAULT CHARSET=utf8;
*/
int main() {
	try {
		MYSQL* mysql = connectDatabase("host", 3306, "user", "passwd", "db", "charset");
		MysqlWrapper mw{ mysql };

		// insert one record.
		mw.execute("INSERT INTO m_blog VALUES (51, 39, 'title', '描述信息', '正文文本', '2023-07-01 12:21:09', 1)");

		// query.
		auto res = mw.query("SELECT id, content FROM m_blog");
		auto colNum = res.getColNum();
		MYSQL_ROW row = nullptr;

		while (row = res.fetchRow()) {
			for (unsigned int i = 0; i < colNum;++i) {
				if (row[i] == nullptr) {
					std::cout << "null value    ";
				}
				else {
					std::cout << row[i] << "    ";
				}
			}

			std::cout << "\n";
		}
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << "\n";
	}

	return 0;
}
