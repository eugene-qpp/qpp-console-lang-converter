#ifndef CONVERT_HPP
#define CONVERT_HPP

namespace rapidcsv
{
    class Document;
}

rapidcsv::Document readCvs(const std::string &filename, int columnNameIndex = 1, int rowNameIndex = 1);
std::set<std::string> writeJson(const rapidcsv::Document &doc, const std::string &columnName, const std::string &filename, bool shouldReplaceBreakLines = true);

#endif // CONVERT_HPP
