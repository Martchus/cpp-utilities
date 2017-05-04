#ifndef IOUTILITIES_INIFILE_H
#define IOUTILITIES_INIFILE_H

#include "../global.h"

#include <map>
#include <string>
#include <vector>

namespace IoUtilities {

class CPP_UTILITIES_EXPORT IniFile {
public:
    IniFile();

    std::vector<std::pair<std::string, std::multimap<std::string, std::string>>> &data();
    const std::vector<std::pair<std::string, std::multimap<std::string, std::string>>> &data() const;
    void parse(std::istream &inputStream);
    void make(std::ostream &outputStream);

private:
    std::vector<std::pair<std::string, std::multimap<std::string, std::string>>> m_data;
};

/*!
 * \brief Constructs an empty ini file.
 */
inline IniFile::IniFile()
{
}

/*!
 * \brief Returns the data of the file.
 * \remarks
 *  - The returned pairs represent the [scope names] and the contained "key = value"-pairs.
 *  - The data might be modified and then saved using the make() method.
 */
inline std::vector<std::pair<std::string, std::multimap<std::string, std::string>>> &IniFile::data()
{
    return m_data;
}

/*!
 * \brief Returns the data of the file.
 * \remarks The returned pairs represent the [scope names] and the contained "key = value"-pairs.
 */
inline const std::vector<std::pair<std::string, std::multimap<std::string, std::string>>> &IniFile::data() const
{
    return m_data;
}

} // namespace IoUtilities

#endif // IOUTILITIES_INIFILE_H
