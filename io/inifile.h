#ifndef IOUTILITIES_INIFILE_H
#define IOUTILITIES_INIFILE_H

#include "../global.h"

#include <iosfwd>
#include <map>
#include <string>
#include <vector>

namespace CppUtilities {

class CPP_UTILITIES_EXPORT IniFile {
public:
    using ScopeName = std::string;
    using ScopeData = std::multimap<std::string, std::string>;
    using Scope = std::pair<ScopeName, ScopeData>;
    using ScopeList = std::vector<Scope>;

    IniFile();
    ScopeList &data();
    const ScopeList &data() const;
    void parse(std::istream &inputStream);
    void make(std::ostream &outputStream);

private:
    ScopeList m_data;
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
inline IniFile::ScopeList &IniFile::data()
{
    return m_data;
}

/*!
 * \brief Returns the data of the file.
 * \remarks The returned pairs represent the [scope names] and the contained "key = value"-pairs.
 */
inline const IniFile::ScopeList &IniFile::data() const
{
    return m_data;
}

} // namespace CppUtilities

#endif // IOUTILITIES_INIFILE_H
