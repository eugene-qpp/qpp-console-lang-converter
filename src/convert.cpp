#include <fstream>
#include <filesystem>
#include "rapidcsv.h"

rapidcsv::Document readCvs(const std::string &filename, int columnNameIndex = 1, int rowNameIndex = 1)
{
    std::filesystem::path path = filename;
    std::ifstream file(path);
    rapidcsv::Document doc(file, rapidcsv::LabelParams(columnNameIndex, rowNameIndex), rapidcsv::SeparatorParams(',', false, false, true, true));
    file.close();
    return doc;
}

void writeJson(const rapidcsv::Document &doc, const std::string &columnName, const std::string &filename, bool shouldReplaceBreakLines = true)
{
    std::ofstream output(filename);
    output << "{\n";
    auto rowNames = doc.GetRowNames();
    for (size_t i = 0; i < rowNames.size(); i++)
    {
        const std::string rowName = rowNames[i];
        std::string text = doc.GetCell<std::string>(columnName, rowName);
        if (shouldReplaceBreakLines)
        {
            // Replace '\n'
            int pos;
            while ((pos = text.find("\\n")) != std::string::npos)
            {
                text.replace(pos, 2, "");
            }
        }

        // Replace break line to "\\n"
        {
            int pos;
            while ((pos = text.find("\n")) != std::string::npos)
            {
                std::cout << "replacing: " << text << "\n";
                text.replace(pos, 1, "\\n");
            }
        }

        // Indent
        output << "  \"";
        output << rowName << "\": \"" << text << "\"";
        if (i != rowNames.size() - 1)
        {
            output << ",";
        }
        output << "\n";
    }

    output << "}";
}
