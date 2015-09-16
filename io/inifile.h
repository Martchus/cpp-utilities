#ifndef IOUTILITIES_INIFILE_H
#define IOUTILITIES_INIFILE_H

#include "../application/global.h"

#include <vector>
#include <map>
#include <string>

namespace IoUtilities {

class LIB_EXPORT IniFile
{
public:
    IniFile();

    std::vector<std::pair<std::string, std::multimap<std::string, std::string> > > &data();
    const std::vector<std::pair<std::string, std::multimap<std::string, std::string> > > &data() const;
    void parse(std::istream &inputStream);
    void make(std::ostream &outputStream);

private:
    std::vector<std::pair<std::string, std::multimap<std::string, std::string> > > m_data;
};

/*!
 * \brief Constructs an empty ini file.
 */
inline IniFile::IniFile()
{}

/*!
 * \brief Returns the data of the file.
 *
 *  - The keys in the returned map represent the [scope name]s.
 *  - The values in the returned map are maps representing "key = value"-pairs within the scope.
 *  - The data might be modified an saved using the make() method.
 */
inline std::vector<std::pair<std::string, std::multimap<std::string, std::string> > > &IniFile::data()
{
    return m_data;
}

/*!
 * \brief Returns the data of the file.
 *
 *  - The keys in the returned map represent the [scope name]s.
 *  - The values in the returned map are maps representing "key = value"-pairs within the scope.
 */
inline const std::vector<std::pair<std::string, std::multimap<std::string, std::string> > > &IniFile::data() const
{
    return m_data;
}

} // namespace IoUtilities

#endif // IOUTILITIES_INIFILE_H
