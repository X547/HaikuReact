#pragma once

#include <map>
#include <optional>
#include <string>
#include <vector>

#include <InterfaceDefs.h>

#include "IconSource.h"


namespace HaikuReact {


enum class ColumnType {
	Text,     // string, optionally with an icon
	Integer,
	Size,     // bytes
	Date,
	Icon,
};


// A column of the columns prop of <ColumnListView>.
struct ColumnSpec {
	std::string id;
	std::string title;
	ColumnType type = ColumnType::Text;
	float width = 100;
	float minWidth = 20;
	float maxWidth = 10000;
	alignment align = B_ALIGN_LEFT;

	bool operator==(const ColumnSpec &other) const = default;
};


// A value of the cells prop of <Row>: a string, a number, a Date (number in
// milliseconds), {text, icon} or an icon source.
struct Cell {
	std::string text;
	std::optional<double> number;
	IconSource icon;

	bool operator==(const Cell &other) const = default;
};

using CellMap = std::map<std::string, Cell>;


struct SortKey {
	std::string column;
	bool ascending = true;

	bool operator==(const SortKey &other) const = default;
};


} // HaikuReact
