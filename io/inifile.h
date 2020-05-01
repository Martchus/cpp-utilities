#ifndef IOUTILITIES_INIFILE_H
#define IOUTILITIES_INIFILE_H

#include "../global.h"
#include "../misc/flagenumclass.h"

#include <algorithm>
#include <iosfwd>
#include <map>
#include <optional>
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

enum class IniFileParseOptions {
    None = 0,
};

enum class IniFileMakeOptions {
    None = 0,
};

enum class IniFileFieldFlags {
    None = 0,
    HasValue = (1 << 0),
};

enum class IniFileSectionFlags {
    None = 0,
    Implicit = (1 << 0),
    Truncated = (1 << 1),
};

struct CPP_UTILITIES_EXPORT AdvancedIniFile {
    struct Field {
        std::string key;
        std::string value;
        std::string precedingCommentBlock;
        std::string followingInlineComment;
        std::size_t paddedKeyLength = 0;
        IniFileFieldFlags flags = IniFileFieldFlags::HasValue;
    };
    using FieldList = std::vector<Field>;
    struct Section {
        FieldList::iterator findField(std::string_view key);
        FieldList::const_iterator findField(std::string_view key) const;
        FieldList::iterator findField(FieldList::iterator after, std::string_view key);
        FieldList::const_iterator findField(FieldList::iterator after, std::string_view key) const;
        FieldList::iterator fieldEnd();
        FieldList::const_iterator fieldEnd() const;

        std::string name;
        FieldList fields;
        std::string precedingCommentBlock;
        std::string followingInlineComment;
        IniFileSectionFlags flags = IniFileSectionFlags::None;
    };
    using SectionList = std::vector<Section>;

    SectionList::iterator findSection(std::string_view sectionName);
    SectionList::const_iterator findSection(std::string_view sectionName) const;
    SectionList::iterator findSection(SectionList::iterator after, std::string_view sectionName);
    SectionList::const_iterator findSection(SectionList::iterator after, std::string_view sectionName) const;
    SectionList::iterator sectionEnd();
    SectionList::const_iterator sectionEnd() const;
    std::optional<FieldList::iterator> findField(std::string_view sectionName, std::string_view key);
    std::optional<FieldList::const_iterator> findField(std::string_view sectionName, std::string_view key) const;
    void parse(std::istream &inputStream, IniFileParseOptions options = IniFileParseOptions::None);
    void make(std::ostream &outputStream, IniFileMakeOptions options = IniFileMakeOptions::None);

    SectionList sections;
};

/*!
 * \brief Returns an iterator to the first section with the name \a sectionName.
 */
inline AdvancedIniFile::SectionList::iterator AdvancedIniFile::findSection(std::string_view sectionName)
{
    return std::find_if(sections.begin(), sections.end(), [&sectionName](const auto &scope) { return scope.name == sectionName; });
}

/*!
 * \brief Returns an iterator to the first section with the name \a sectionName.
 */
inline AdvancedIniFile::SectionList::const_iterator AdvancedIniFile::findSection(std::string_view sectionName) const
{
    return const_cast<AdvancedIniFile *>(this)->findSection(sectionName);
}

/*!
 * \brief Returns an iterator to the first section with the name \a sectionName which comes after \a after.
 */
inline AdvancedIniFile::SectionList::iterator AdvancedIniFile::findSection(SectionList::iterator after, std::string_view sectionName)
{
    return std::find_if(after + 1, sections.end(), [&sectionName](const auto &scope) { return scope.name == sectionName; });
}

/*!
 * \brief Returns an iterator to the first section with the name \a sectionName which comes after \a after.
 */
inline AdvancedIniFile::SectionList::const_iterator AdvancedIniFile::findSection(SectionList::iterator after, std::string_view sectionName) const
{
    return const_cast<AdvancedIniFile *>(this)->findSection(after, sectionName);
}

/*!
 * \brief Returns an iterator that points one past the last section.
 */
inline AdvancedIniFile::SectionList::iterator AdvancedIniFile::sectionEnd()
{
    return sections.end();
}

/*!
 * \brief Returns an iterator that points one past the last section.
 */
inline AdvancedIniFile::SectionList::const_iterator AdvancedIniFile::sectionEnd() const
{
    return sections.end();
}

/*!
 * \brief Returns an iterator to the first field within the first section with matching \a sectionName and \a key.
 */
inline std::optional<AdvancedIniFile::FieldList::iterator> AdvancedIniFile::findField(std::string_view sectionName, std::string_view key)
{
    const SectionList::iterator scope = findSection(sectionName);
    if (scope == sectionEnd()) {
        return std::nullopt;
    }
    const FieldList::iterator field = scope->findField(key);
    if (field == scope->fieldEnd()) {
        return std::nullopt;
    }
    return field;
}

/*!
 * \brief Returns an iterator to the first field within the first section with matching \a sectionName and \a key.
 */
inline std::optional<AdvancedIniFile::FieldList::const_iterator> AdvancedIniFile::findField(std::string_view sectionName, std::string_view key) const
{
    return const_cast<AdvancedIniFile *>(this)->findField(sectionName, key);
}

/*!
 * \brief Returns an iterator to the first field with the key \a key.
 */
inline AdvancedIniFile::FieldList::iterator AdvancedIniFile::Section::findField(std::string_view key)
{
    return std::find_if(fields.begin(), fields.end(), [&key](const auto &field) { return field.key == key; });
}

/*!
 * \brief Returns an iterator to the first field with the key \a key.
 */
inline AdvancedIniFile::FieldList::const_iterator AdvancedIniFile::Section::findField(std::string_view key) const
{
    return const_cast<Section *>(this)->findField(key);
}

/*!
 * \brief Returns an iterator to the first field with the key \a key which comes after \a after.
 */
inline AdvancedIniFile::FieldList::iterator AdvancedIniFile::Section::findField(FieldList::iterator after, std::string_view key)
{
    return std::find_if(after + 1, fields.end(), [&key](const auto &field) { return field.key == key; });
}

/*!
 * \brief Returns an iterator to the first field with the key \a key which comes after \a after.
 */
inline AdvancedIniFile::FieldList::const_iterator AdvancedIniFile::Section::findField(FieldList::iterator after, std::string_view key) const
{
    return const_cast<Section *>(this)->findField(after, key);
}

/*!
 * \brief Returns an iterator that points one past the last field.
 */
inline AdvancedIniFile::FieldList::iterator AdvancedIniFile::Section::fieldEnd()
{
    return fields.end();
}

/*!
 * \brief Returns an iterator that points one past the last field.
 */
inline AdvancedIniFile::FieldList::const_iterator AdvancedIniFile::Section::fieldEnd() const
{
    return fields.end();
}

} // namespace CppUtilities

CPP_UTILITIES_MARK_FLAG_ENUM_CLASS(CppUtilities, IniFileParseOptions);
CPP_UTILITIES_MARK_FLAG_ENUM_CLASS(CppUtilities, IniFileMakeOptions);
CPP_UTILITIES_MARK_FLAG_ENUM_CLASS(CppUtilities, IniFileFieldFlags);
CPP_UTILITIES_MARK_FLAG_ENUM_CLASS(CppUtilities, IniFileSectionFlags);

#endif // IOUTILITIES_INIFILE_H
