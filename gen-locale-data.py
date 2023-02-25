import os
import re
import sys
import xml.etree.ElementTree as ET 

# Digits are not included inside the locale distributions, which means we have to make them ourselves.
altdigits_map = {
    "ar_": "٠١٢٣٤٥٦٧٨٩",
    "bn_": "০১২৩৪৫৬৭৮৯",
#    "zh_CN": "〇一二三四五六七八九", // Too complicated
#    "zh_TW": "零壹貳參肆伍陸柒捌玖", // Too complicated
    #"san": "०१२३४५६७८९", # not present in GNU locales
#    "am_": "0፩፪፫፬፭፮፯፰፱", // Too complicated
    "gu_": "૦૧૨૩૪૫૬૭૮૯",
    "pa_": "੦੧੨੩੪੫੬੭੮੯",
    "kn_": "೦೧೨೩೪೫೬೭೮೯",
    "km_": "០១២៣៤៥៦៧៨៩",
    "lo_": "໐໑໒໓໔໕໖໗໘໙",
    "ml_": "൦൧൨൩൪൫൬൭൮൯",
    "mn_": "᠐᠑᠒᠓᠔᠕᠖᠗᠘᠙",
    "or_": "୦୧୨୩୪୫୬୭୮୯",
    "ta_": "௦௧௨௩௪௫௬௭௮௯",
    "te_": "౦౧౨౩౪౫౬౭౮౯",
    "th_": "๐๑๒๓๔๕๖๗๘๙",
    "bo_": "༠༡༢༣༤༥༦༧༨༩",
    "ur_": "۰۱۲۳۴۵۶۷۸۹",
    "fa_": "۰۱۲۳۴۵۶۷۸۹",
}


def decode_utf8(encoded_string):
    decoded_string = ''
    thres = 0
    for i in range(len(encoded_string)):
        if thres > i:
            continue
        if encoded_string[i] == "<" and re.search(r'<U[0-9A-Fa-f]{4}>', encoded_string[i:i+7]):
            code = encoded_string[i+2:i+6]
            code_int = int(code, 16)  # convert to base 16
            decoded_string += chr(code_int)
            thres = i+7
        else:
            decoded_string += encoded_string[i]
            thres = i+1
    return decoded_string

def read_locale_file(file_name, language_map, code):
    # Set the comment_char and escape_char options
    comment_char = '%'
    escape_char = '/'

    # Read in the given file
    with open(file_name) as f:
        all_lines = []
        line = "1"
        while line:
            try:
                line = f.readline()
                all_lines.append(line)
            except UnicodeDecodeError as e:
                # Some comment has invalid bytes as the real data
                # is only supposed to be in ASCII - just ignore it
                continue

    # Variables to store the current section and key-values
    current_section = ""
    kv_pairs = {"" : {}}
    continue_line = False

    # Iterate through all the lines
    for line in all_lines:
       
        line = line.replace('\t', ' ')
        line = decode_utf8(line.strip())
        # If the line starts with the comment character, blank,
        # or an END directive (e.g. END LC_MESSAGES), ignore it
        if line == '' or line.startswith("END") or not continue_line and line.startswith(comment_char):
            continue
        elif continue_line:
            if line[-1] == escape_char:
                continue_line = True
                line = line[:-1]
            else:
                continue_line = False
            value = line.replace('//', '/').split(';')
            kv_pairs[current_section][key] += [v for v in value if v != '']
        elif line.count(' ') > 0:
            key = line.split(' ')[0]
            value = line[len(key)+1:]
            # check for escape char (except for the escape_char key)
            if len(value) > 0 and value[-1] == escape_char and key != "escape_char":
                continue_line = True
                value = value[:-1]
            else:
                continue_line = False
            # Strip any whitespace
            key = key.strip()
            value = value.strip().replace('//', '/').split(';')
            # Add it to the dict
            if key in kv_pairs[current_section].keys():
                kv_pairs[current_section][key] += [v for v in value if v != '']
            else:
                kv_pairs[current_section][key] = [v for v in value if v != '']
        else:
            # The line is a section header
            # Get the section name
            current_section = line.strip()
            kv_pairs[current_section] = {}
    # STOP! Don't just return here, we need to make sure that LC_TIME fields are filled.
    if "LC_TIME" not in kv_pairs.keys():
        return language_map
    language_map[code]["locale_info"] = kv_pairs
    if "copy" in kv_pairs["LC_TIME"].keys():
         ref_code = kv_pairs["LC_TIME"]["copy"][0].replace('"', '')
         language_map[ref_code] = {}
         language_map = read_locale_file("/usr/share/i18n/locales/{}".format(ref_code), language_map, ref_code)
         # Will it work?
         language_map[code]["locale_info"]["LC_TIME"] = language_map[ref_code]["locale_info"]["LC_TIME"]
    return language_map

def parse_ldml_language_map(file_name):
    tree = ET.parse(file_name)
    root = tree.getroot()

    language_map = {}

    for node in root.iter('localeDisplayNames'):
        for language in node.iter('languages'):
            for name in language.iter('language'):
                short_name = name.attrib['type']
                if "_" not in short_name:
                    long_name = name.text
                    language_map[short_name] = {}
                    language_map[short_name]["name"] = long_name.upper()

    return language_map                                 


def parse_ldml_locales(language_map, file_name, iso_code):
    tree = ET.parse(file_name)
    root = tree.getroot()
   
    months_long = {}
    months_short = {}
    weeks_long = {}
    weeks_short = {}

    for node in root.iter('dates'):
        for node in node.find('calendars').iter('calendar'):
            calendar_name = node.attrib['type']
            if calendar_name == 'gregorian': # we only support gregorian calendars for now
                for month_node in node.iter('months'):
                    for month in month_node.iter('monthContext'):
                        month_name = month.attrib['type']
                        if month_name == 'format':
                            for names in month.iter('monthWidth'):
                                name_type = names.attrib['type']
                                if name_type == 'wide':                                                                 
                                    for name in names.iter('month'):                                                    
                                        month_string = name.attrib['type']                                         
                                        months_long[month_string] = name.text                                           
                                elif name_type == 'abbreviated':                                                        
                                    for name in names.iter('month'):                                                    
                                        month_string = name.attrib['type']                                         
                                        months_short[month_string] = name.text                                          
                for week_node in node.iter('days'):                                                                     
                    for week in week_node.iter('dayContext'):                                                           
                        week_name = week.attrib['type']                                                                 
                        if week_name == 'format':               
                            for names in week.iter('dayWidth'):
                                name_type = names.attrib['type']
                                if name_type == 'wide':
                                    for name in names.iter('month'):
                                        month_string = name.attrib['type']
                                        months_long[month_string] = name.text
                                elif name_type == 'abbreviated':
                                    for name in names.iter('month'):
                                        month_string = name.attrib['type']
                                        months_short[month_string] = name.text
                for week_node in node.iter('days'):
                    for week in week_node.iter('dayContext'):
                        week_name = week.attrib['type']
                        if week_name == 'format':
                            for names in week.iter('dayWidth'):
                                name_type = names.attrib['type']
                                if name_type == 'wide':
                                    for name in names.iter('day'):
                                        week_string = name.attrib['type']
                                        weeks_long[week_string] = name.text
                                elif name_type == 'abbreviated':
                                    for name in names.iter('day'):
                                        week_string = name.attrib['type']
                                        weeks_short[week_string] = name.text

    language_map[iso_code]["months_long"] = months_long
    language_map[iso_code]["months_short"] = months_short
    language_map[iso_code]["weeks_long"] = weeks_long
    language_map[iso_code]["weeks_short"] = weeks_short
    return language_map

def print_xdatetime_macros(language_map):
    print("// You must define an X_DATETIME_ONLY_LOCALE_* macro, which is only read if you don't want locales.")
    print("// English is set by default in the event that locales are disabled - which are also enabled by default.")
    print("// This means if you want to disable the English locale, you must undefine this macro before including this file.")
    print("#define X_DATETIME_ONLY_LOCALE_ENGLISH\n")
    print("#ifndef X_DATETIME_NO_LOCALES")
    for code, language in language_map.items():
        if "months_long" not in language.keys() or ("months_long" in language.keys() and len(language["months_long"]) == 0) or \
                "months_short" not in language.keys() or ("months_short" in language.keys() and len(language["months_short"]) == 0) or \
                "weeks_long" not in language.keys() or ("weeks_long" in language.keys() and len(language["weeks_long"]) == 0) or \
                "weeks_short" not in language.keys() or ("weeks_short" in language.keys() and len(language["weeks_short"]) == 0):
            continue

        name = language["name"]
        name = name.replace(',', '_')
        name = name.replace(' ', '_')
        name = name.replace('-', '_')
        for identifier, data in language["months_long"].items():
            print("    data.long_months[\"{}\"][\"{}\"] = u8\"{}\";".format(code, identifier, data))
        print("") # empty line

        for identifier, data in language["months_short"].items():
            print("    data.short_months[\"{}\"][\"{}\"] = u8\"{}\";".format(code, identifier, data))
        print("")

        for identifier, data in language["weeks_long"].items():
            print("    data.long_weeks[\"{}\"][\"{}\"] = u8\"{}\";".format(code, identifier, data))
        print("")

        for identifier, data in language["weeks_short"].items():
            print("    data.short_months[\"{}\"][\"{}\"] = u8\"{}\";".format(code, identifier, data))
        print("\n") # two empty lines
    print("#else")
    for code, language in language_map.items():
        if "months_long" not in language.keys() or ("months_long" in language.keys() and len(language["months_long"]) == 0) or \
                "months_short" not in language.keys() or ("months_short" in language.keys() and len(language["months_short"]) == 0) or \
                "weeks_long" not in language.keys() or ("weeks_long" in language.keys() and len(language["weeks_long"]) == 0) or \
                "weeks_short" not in language.keys() or ("weeks_short" in language.keys() and len(language["weeks_short"]) == 0):
            continue

        name = language["name"]
        name = name.replace(',', '_')
        name = name.replace(' ', '_')
        name = name.replace('-', '_')
        print("#ifdef X_DATETIME_ONLY_LOCALE_{}".format(name))
        for identifier, data in language["months_long"].items():
            print("    data.long_months[\"{}\"][\"{}\"] = u8\"{}\";".format(code, identifier, data))
        print("") # empty line

        for identifier, data in language["months_short"].items():
            print("    data.short_months[\"{}\"][\"{}\"] = u8\"{}\";".format(code, identifier, data))
        print("")

        for identifier, data in language["weeks_long"].items():
            print("    data.long_weeks[\"{}\"][\"{}\"] = u8\"{}\";".format(code, identifier, data))
        print("")

        for identifier, data in language["weeks_short"].items():
            print("    data.short_weeks[\"{}\"][\"{}\"] = u8\"{}\";".format(code, identifier, data))
        print("#endif\n")
        print("\n") # two empty lines
    print("#endif\n")


# The collective GNU C library community wisdom regarding abday, day, week, first_weekday, and first_workday states at https://sourceware.org/glibc/wiki/Locales the following:
#
# *  The value of the second week list item specifies the base of the abday and day lists.
#
# *  first_weekday specifies the offset of the first day-of-week in the abday and day lists.
#
# *  For compatibility reasons, all glibc locales should set the value of the second week list item to 19971130 (Sunday) and base the abday and day lists appropriately, and set first_weekday and first_workday to
#    1 or 2, depending on whether the week and work week actually starts on Sunday or Monday for the locale.
def print_xdatetime_macros2(language_map):
    for code, language in language_map.items():
        if "locale_info" not in language.keys() or ("locale_info" in language.keys() and len(language["locale_info"]) == 0):
            continue
        if code == "i18n":
            continue # "i18n" is a garbage locale. It is not used by anything.
        language = language["locale_info"]

        name = code.upper().replace("@", "_");
        print("#ifdef X_DATETIME_WITH_LOCALE_{}".format(name))
        print("    data.am[\"{}\"] = reinterpret_cast<const char*>(u8\"{}\");".format(code, language["LC_TIME"]["am_pm"][0].replace('"', '')))
        print("    data.pm[\"{}\"] = reinterpret_cast<const char*>(u8\"{}\");".format(code, language["LC_TIME"]["am_pm"][1].replace('"', '')))
        if "date_fmt" in language["LC_TIME"].keys():
            print("    data.date1_format[\"{}\"] = reinterpret_cast<const char*>(u8\"{}\");".format(code, language["LC_TIME"]["date_fmt"][0].replace('"', '')))
        else:
            print("    data.date1_format[\"{}\"] = reinterpret_cast<const char*>(u8\"%a %b %e %H:%M:%S %Z %Y\");".format(code))
        print("    data.date_time_format[\"{}\"] = reinterpret_cast<const char*>(u8\"{}\");".format(code, language["LC_TIME"]["d_t_fmt"][0].replace('"', '')))
        print("    data.date_format[\"{}\"] = reinterpret_cast<const char*>(u8\"{}\");".format(code, language["LC_TIME"]["d_fmt"][0].replace('"', '')))
        print("    data.time24_format[\"{}\"] = reinterpret_cast<const char*>(u8\"{}\");".format(code, language["LC_TIME"]["t_fmt"][0].replace('"', '')))
        if "t_fmt_ampm" in language["LC_TIME"].keys(): 
            print("    data.time12_format[\"{}\"] = reinterpret_cast<const char*>(u8\"{}\");".format(code, language["LC_TIME"]["t_fmt_ampm"][0].replace('"', '')))
        else:
            print("    data.time12_format[\"{}\"] = reinterpret_cast<const char*>(u8\"{}\");".format(code, language["LC_TIME"]["t_fmt"][0].replace('"', '')))

        if "week" in language["LC_TIME"].keys():
            print("    data.days_in_week[\"{}\"] = {};".format(code, int(language["LC_TIME"]["week"][0])))
            print("    data.first_weekday_ref[\"{}\"] = {};".format(code, int(language["LC_TIME"]["week"][1])))
            print("    data.first_week_year_min_days[\"{}\"] = {};".format(code, int(language["LC_TIME"]["week"][2])))
        else:
            print("    data.days_in_week[\"{}\"] = 7;".format(code))
            print("    data.first_weekday_ref[\"{}\"] = 11971130;".format(code))
            print("    data.first_week_year_min_days[\"{}\"] = 4;".format(code))

        if "first_weekday" in language["LC_TIME"].keys():
            print("    data.first_weekday[\"{}\"] = {};".format(code, int(language["LC_TIME"]["first_weekday"][0])))
        else:
            print("    data.first_weekday[\"{}\"] = 1;".format(code))

        i = 0
        for data in language["LC_TIME"]["mon"]:
            print("    data.long_months[\"{}\"][{}] = reinterpret_cast<const char*>(u8\"{}\");".format(code, i, data.replace('"', '')))
            i += 1
        print("") # empty line

        i = 0
        for data in language["LC_TIME"]["abmon"]:
            print("    data.short_months[\"{}\"][{}] = reinterpret_cast<const char*>(u8\"{}\");".format(code, i, data.replace('"', '')))
            i += 1
        print("")

        i = 0
        for data in language["LC_TIME"]["day"]:
            print("    data.long_weekdays[\"{}\"][{}] = reinterpret_cast<const char*>(u8\"{}\");".format(code, i, data.replace('"', '')))
            i += 1
        print("")

        i = 0
        for data in language["LC_TIME"]["abday"]:
            print("    data.short_weekdays[\"{}\"][{}] = reinterpret_cast<const char*>(u8\"{}\");".format(code, i, data.replace('"', '')))
            i += 1
        
        foundkey = ""
        for altkey in altdigits_map.keys():
            if code.startswith(altkey):
                foundkey = altkey
                break
        if foundkey:
            for i in range(0, 10):
                print("    data.alt_digits[\"{}\"][{}] = reinterpret_cast<const char*>(u8\"{}\");".format(code, i, altdigits_map[foundkey][i]))
        else:
            for i in range(0, 10):
                print("    data.alt_digits[\"{}\"][{}] = \"{}\";".format(code, i, str(i)))
        print("#endif /* X_DATETIME_WITH_LOCALE_{} */".format(name))
       
        print("")
    
def print_autogenerated_code(language_map):
    print("// Automatically generated by cldr-gen-locale-data.py. DO NOT MODIFY.\n")
    print("#ifndef X_DATETIME_LOCALE_DATA_H")
    print("#define X_DATETIME_LOCALE_DATA_H")
    print("#include <map>")
    print("#include <string>\n")
    print("namespace xDateTime {")
    print("struct _LocaleData {")
    print("    std::map<std::string, std::string> am;")
    print("    std::map<std::string, std::string> pm;")
    print("    std::map<std::string, std::string> date1_format;")
    print("    std::map<std::string, std::string> date_time_format;")
    print("    std::map<std::string, std::string> date_format;")
    print("    std::map<std::string, std::string> time24_format;")
    print("    std::map<std::string, std::string> time12_format;")
    print("    std::map<std::string, int> days_in_week;")
    print("    std::map<std::string, int> first_weekday_ref;")
    print("    std::map<std::string, int> first_weekday;")
    print("    std::map<std::string, int> first_week_year_min_days;")
    print("    std::map<std::string, std::map<int, std::string>> long_months;")
    print("    std::map<std::string, std::map<int, std::string>> short_months;")
    print("    std::map<std::string, std::map<int, std::string>> long_weekdays;")
    print("    std::map<std::string, std::map<int, std::string>> short_weekdays;")
    print("    std::map<std::string, std::map<int, std::string>> alt_digits;")
    print("};\n")
    print("static inline void InitializeLocaleData(_LocaleData& data) {")
    print_xdatetime_macros2(language_map)
    print("}\n")
    print("}") # namespace
    print("#endif /* X_DATETIME_LOCALE_DATA_H */")


import pprint

def print_locale_info(language_map):
    for key, value in language_map.items():
         if "locale_info" in value.keys():
             for vkey, vvalue in value["locale_info"].items():
                 if vkey == "LC_TIME":
                     print(key, "$$$$$$$")
                     pprint.pprint(vvalue)

def main():
    # search the OS-specific locales
    locales_folder = "/usr/share/i18n/locales"
    language_map = {}
    for file_name2 in os.listdir(locales_folder):
        # The OS locales do not have a file extension.
        language_map[file_name2] = {}
        language_map = read_locale_file(locales_folder + '/' + file_name2, language_map, file_name2)
    
    print_autogenerated_code(language_map)

if __name__ == '__main__':
    main()
