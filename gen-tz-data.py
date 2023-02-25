import csv
import subprocess
import datetime
import json


tznames = json.loads('''
{"ACDT": [10, 30],
 "ACST": [9, 30],
 "ACT": [8, 0],
 "ACWST": [8, 45],
 "ADT": [3, 0],
 "AEDT": [11, 0],
 "AEST": [10, 0],
 "AFT": [4, 30],
 "AKDT": [8, 0],
 "AKST": [9, 0],
 "ALMT": [6, 0],
 "AMST": [3, 0],
 "AMT": [4, 0],
 "ANAT": [12, 0],
 "AQTT": [5, 0],
 "ART": [3, 0],
 "ASEAN Common Time": [8, 0],
 "AST": [4, 0],
 "AWST": [8, 0],
 "AZOST": [0, 0],
 "AZOT": [1, 0],
 "AZT": [4, 0],
 "Acre Time": [5, 0],
 "Afghanistan Time": [4, 30],
 "Alaska Daylight Time": [8, 0],
 "Alaska Standard Time": [9, 0],
 "Alma-Ata Time": [6, 0],
 "Amazon Summer Time": [3, 0],
 "Amazon Time": [4, 0],
 "Anadyr Time": [12, 0],
 "Aqtobe Time": [5, 0],
 "Arabia Standard Time": [3, 0],
 "Argentina Time": [3, 0],
 "Armenia Time": [4, 0],
 "Atlantic Daylight Time": [3, 0],
 "Atlantic Standard Time": [4, 0],
 "Australian Central Daylight Saving Time": [10, 30],
 "Australian Central Standard Time": [9, 30],
 "Australian Central Western Standard Time": [8, 45],
 "Australian Eastern Daylight Saving Time": [11, 0],
 "Australian Eastern Standard Time": [10, 0],
 "Australian Western Standard Time": [8, 0],
 "Azerbaijan Time": [4, 0],
 "Azores Standard Time": [1, 0],
 "Azores Summer Time": [0, 0],
 "BIOT": [6, 0],
 "BIT": [12, 0],
 "BNT": [8, 0],
 "BOT": [4, 0],
 "BRST": [2, 0],
 "BRT": [3, 0],
 "BST": [1, 0],
 "BTT": [6, 0],
 "Baker Island Time": [12, 0],
 "Bangladesh Standard Time": [6, 0],
 "Bhutan Time": [6, 0],
 "Bolivia Time": [4, 0],
 "Bougainville Standard Time": [11, 0],
 "Brasília Summer Time": [2, 0],
 "Brasília Time": [3, 0],
 "British Indian Ocean Time": [6, 0],
 "British Summer Time": [1, 0],
 "Brunei Time": [8, 0],
 "CAT": [2, 0],
 "CCT": [6, 30],
 "CDT": [4, 0],
 "CEST": [2, 0],
 "CET": [1, 0],
 "CHADT": [13, 45],
 "CHAST": [12, 45],
 "CHOST": [9, 0],
 "CHOT": [8, 0],
 "CHST": [10, 0],
 "CHUT": [10, 0],
 "CIST": [8, 0],
 "CKT": [10, 0],
 "CLST": [3, 0],
 "CLT": [4, 0],
 "COST": [4, 0],
 "COT": [5, 0],
 "CST": [5, 0],
 "CVT": [1, 0],
 "CWST": [8, 45],
 "CXT": [7, 0],
 "Cape Verde Time": [1, 0],
 "Central Africa Time": [2, 0],
 "Central Daylight Time": [5, 0],
 "Central European Summer Time": [2, 0],
 "Central European Time": [1, 0],
 "Central Indonesia Time": [8, 0],
 "Central Standard Time": [6, 0],
 "Central Western Standard Time": [8, 45],
 "Chagos Archipelago|Indian Ocean Time": [3, 0],
 "Chamorro Standard Time": [10, 0],
 "Chatham Daylight Time": [13, 45],
 "Chatham Standard Time": [12, 45],
 "Chile Standard Time": [4, 0],
 "Chile Summer Time": [3, 0],
 "China Standard Time": [8, 0],
 "Choibalsan Standard Time": [8, 0],
 "Choibalsan Summer Time": [9, 0],
 "Christmas Island Time": [7, 0],
 "Chuuk Time": [10, 0],
 "Clipperton Island Standard Time": [8, 0],
 "Cocos Islands Time": [6, 30],
 "Colombia Summer Time": [4, 0],
 "Colombia Time": [5, 0],
 "Cook Island Time": [10, 0],
 "Coordinated Universal Time": [0, 0],
 "Cuba Daylight Time": [4, 0],
 "Cuba Standard Time": [5, 0],
 "DAVT": [7, 0],
 "DDUT": [10, 0],
 "DFT": [1, 0],
 "Davis Time": [7, 0],
 "Dumont d'Urville Time": [10, 0],
 "EASST": [5, 0],
 "EAST": [6, 0],
 "EAT": [3, 0],
 "ECT": [5, 0],
 "EDT": [4, 0],
 "EEST": [3, 0],
 "EET": [2, 0],
 "EGST": [0, 0],
 "EGT": [1, 0],
 "EST": [5, 0],
 "East Africa Time": [3, 0],
 "Easter Island Standard Time": [6, 0],
 "Easter Island Summer Time": [5, 0],
 "Eastern Caribbean Time": [4, 0],
 "Eastern Daylight Time": [4, 0],
 "Eastern European Summer Time": [3, 0],
 "Eastern European Time": [2, 0],
 "Eastern Greenland Summer Time": [0, 0],
 "Eastern Greenland Time": [1, 0],
 "Eastern Indonesian Time": [9, 0],
 "Eastern Standard Time": [5, 0],
 "Ecuador Time": [5, 0],
 "FET": [3, 0],
 "FJT": [12, 0],
 "FKST": [3, 0],
 "FKT": [4, 0],
 "FNT": [2, 0],
 "Falkland Islands Summer Time": [3, 0],
 "Falkland Islands Time": [4, 0],
 "Fernando de Noronha Time": [2, 0],
 "Fiji Time": [12, 0],
 "French Guiana Time": [3, 0],
 "French Southern and Antarctic Time": [5, 0],
 "Further-eastern European Time": [3, 0],
 "GALT": [6, 0],
 "GAMT": [9, 0],
 "GET": [4, 0],
 "GFT": [3, 0],
 "GILT": [12, 0],
 "GIT": [9, 0],
 "GMT": [0, 0],
 "GST": [4, 0],
 "GYT": [4, 0],
 "Galápagos Time": [6, 0],
 "Gambier Island Time": [9, 0],
 "Gambier Islands Time": [9, 0],
 "Georgia Standard Time": [4, 0],
 "Gilbert Island Time": [12, 0],
 "Greenwich Mean Time": [0, 0],
 "Gulf Standard Time": [4, 0],
 "Guyana Time": [4, 0],
 "HAEC": [2, 0],
 "HDT": [9, 0],
 "HKT": [8, 0],
 "HMT": [5, 0],
 "HOVST": [8, 0],
 "HOVT": [7, 0],
 "HST": [10, 0],
 "Hawaii–Aleutian Daylight Time": [9, 0],
 "Hawaii–Aleutian Standard Time": [10, 0],
 "Heard and McDonald Islands": [5, 0],
 "Heure Avancée d'Europe Centrale": [2, 0],
 "Hong Kong Time": [8, 0],
 "Hovd Summer Time": [8, 0],
 "Hovd Time": [7, 0],
 "ICT": [7, 0],
 "IDLW": [12, 0],
 "IDT": [3, 0],
 "IOT": [3, 0],
 "IRDT": [4, 30],
 "IRKT": [8, 0],
 "IRST": [3, 30],
 "IST": [2, 0],
 "Indian Standard Time": [5, 30],
 "Indochina Time": [7, 0],
 "Iran Daylight Time": [4, 30],
 "Iran Standard Time": [3, 30],
 "Irish Standard Time": [1, 0],
 "Irkutsk Time": [8, 0],
 "Israel Daylight Time": [3, 0],
 "Israel Standard Time": [2, 0],
 "JST": [9, 0],
 "Japan Standard Time": [9, 0],
 "KALT": [2, 0],
 "KGT": [6, 0],
 "KOST": [11, 0],
 "KRAT": [7, 0],
 "KST": [9, 0],
 "Kaliningrad Time": [2, 0],
 "Kamchatka Time": [12, 0],
 "Korea Standard Time": [9, 0],
 "Kosrae Time": [11, 0],
 "Krasnoyarsk Time": [7, 0],
 "Kyrgyzstan Time": [6, 0],
 "LHST": [11, 0],
 "LINT": [14, 0],
 "Line Islands": [14, 0],
 "Lord Howe Standard Time": [10, 30],
 "Lord Howe Summer Time": [11, 0],
 "MAGT": [12, 0],
 "MART": [9, 30],
 "MAWT": [5, 0],
 "MDT": [6, 0],
 "MEST": [2, 0],
 "MET": [1, 0],
 "MHT": [12, 0],
 "MIST": [11, 0],
 "MIT": [9, 30],
 "MMT": [6, 30],
 "MSK": [3, 0],
 "MST": [7, 0],
 "MUT": [4, 0],
 "MVT": [5, 0],
 "MYT": [8, 0],
 "Macquarie Island Station Time": [11, 0],
 "Magadan Time": [12, 0],
 "Malaysia Standard Time": [8, 0],
 "Malaysia Time": [8, 0],
 "Maldives Time": [5, 0],
 "Marquesas Islands Time": [9, 30],
 "Marshall Islands Time": [12, 0],
 "Mauritius Time": [4, 0],
 "Mawson Station Time": [5, 0],
 "Middle European Summer Time": [2, 0],
 "Middle European Time": [1, 0],
 "Moscow Time": [3, 0],
 "Mountain Daylight Time": [6, 0],
 "Mountain Standard Time": [7, 0],
 "Myanmar Standard Time": [6, 30],
 "NCT": [11, 0],
 "NDT": [2, 30],
 "NFT": [11, 0],
 "NOVT": [7, 0],
 "NPT": [5, 45],
 "NST": [3, 30],
 "NT": [3, 30],
 "NUT": [11, 0],
 "NZDT": [13, 0],
 "NZST": [12, 0],
 "Nepal Time": [5, 45],
 "New Caledonia Time": [11, 0],
 "New Zealand Daylight Time": [13, 0],
 "New Zealand Standard Time": [12, 0],
 "Newfoundland Daylight Time": [2, 30],
 "Newfoundland Standard Time": [3, 30],
 "Newfoundland Time": [3, 30],
 "Niue Time": [11, 0],
 "Norfolk Island Time": [11, 0],
 "Novosibirsk Time ": [7, 0],
 "OMST": [6, 0],
 "ORAT": [5, 0],
 "Omsk Time": [6, 0],
 "Oral Time": [5, 0],
 "PDT": [7, 0],
 "PET": [5, 0],
 "PETT": [12, 0],
 "PGT": [10, 0],
 "PHOT": [13, 0],
 "PHST": [8, 0],
 "PHT": [8, 0],
 "PKT": [5, 0],
 "PMDT": [2, 0],
 "PMST": [3, 0],
 "PONT": [11, 0],
 "PST": [8, 0],
 "PWT": [9, 0],
 "PYST": [3, 0],
 "PYT": [4, 0],
 "Pacific Daylight Time": [7, 0],
 "Pacific Standard Time": [8, 0],
 "Pakistan Standard Time": [5, 0],
 "Palau Time": [9, 0],
 "Papua New Guinea Time": [10, 0],
 "Paraguay Summer Time": [3, 0],
 "Paraguay Time": [4, 0],
 "Peru Time": [5, 0],
 "Philippine Standard Time": [8, 0],
 "Philippine Time": [8, 0],
 "Phoenix Island Time": [13, 0],
 "Pohnpei Standard Time": [11, 0],
 "RET": [4, 0],
 "ROTT": [3, 0],
 "Rothera Research Station Time": [3, 0],
 "Réunion Time": [4, 0],
 "SAKT": [11, 0],
 "SAMT": [4, 0],
 "SAST": [2, 0],
 "SBT": [11, 0],
 "SCT": [4, 0],
 "SDT": [10, 0],
 "SGT": [8, 0],
 "SLST": [5, 30],
 "SRET": [11, 0],
 "SRT": [3, 0],
 "SST": [8, 0],
 "SYOT": [3, 0],
 "Saint Pierre and Miquelon Daylight Time": [2, 0],
 "Saint Pierre and Miquelon Standard Time": [3, 0],
 "Sakhalin Island Time": [11, 0],
 "Samara Time": [4, 0],
 "Samoa Daylight Time": [10, 0],
 "Samoa Standard Time": [11, 0],
 "Seychelles Time": [4, 0],
 "Showa Station Time": [3, 0],
 "Singapore Standard Time": [8, 0],
 "Singapore Time": [8, 0],
 "Solomon Islands Time": [11, 0],
 "South African Standard Time": [2, 0],
 "South Georgia and the South Sandwich Islands Time": [2, 0],
 "Srednekolymsk Time": [11, 0],
 "Sri Lanka Standard Time": [5, 30],
 "Suriname Time": [3, 0],
 "TAHT": [10, 0],
 "TFT": [5, 0],
 "THA": [7, 0],
 "TJT": [5, 0],
 "TKT": [13, 0],
 "TLT": [9, 0],
 "TMT": [5, 0],
 "TOT": [13, 0],
 "TRT": [3, 0],
 "TVT": [12, 0],
 "Tahiti Time": [10, 0],
 "Tajikistan Time": [5, 0],
 "Thailand Standard Time": [7, 0],
 "Timor Leste Time": [9, 0],
 "Tokelau Time": [13, 0],
 "Tonga Time": [13, 0],
 "Turkey Time": [3, 0],
 "Turkmenistan Time": [5, 0],
 "Tuvalu Time": [12, 0],
 "ULAST": [9, 0],
 "ULAT": [8, 0],
 "UTC": [0, 0],
 "UYST": [2, 0],
 "UYT": [3, 0],
 "UZT": [5, 0],
 "Ulaanbaatar Standard Time": [8, 0],
 "Ulaanbaatar Summer Time": [9, 0],
 "Uruguay Standard Time": [3, 0],
 "Uruguay Summer Time": [2, 0],
 "Uzbekistan Time": [5, 0],
 "VET": [4, 0],
 "VLAT": [10, 0],
 "VOLT": [3, 0],
 "VOST": [6, 0],
 "VUT": [11, 0],
 "Vanuatu": [11, 0],
 "Venezuelan Standard Time": [4, 0],
 "Vladivostok Time": [10, 0],
 "Volgograd Time": [3, 0],
 "Vostok Station Time": [6, 0],
 "WAKT": [12, 0],
 "WAST": [2, 0],
 "WAT": [1, 0],
 "WEST": [1, 0],
 "WET": [0, 0],
 "WGST": [2, 0],
 "WGT": [3, 0],
 "WIB": [7, 0],
 "WIT": [9, 0],
 "WITA": [8, 0],
 "WST": [8, 0],
 "Wake Island Time": [12, 0],
 "West Africa Summer Time": [2, 0],
 "West Africa Time": [1, 0],
 "West Greenland Summer Time": [2, 0],
 "West Greenland Time": [3, 0],
 "Western European Summer Time": [1, 0],
 "Western European Time": [0, 0],
 "Western Indonesian Time": [7, 0],
 "Western Standard Time": [8, 0],
 "YAKT": [9, 0],
 "YEKT": [5, 0],
 "Yakutsk Time": [9, 0],
 "Yekaterinburg Time": [5, 0]}
''')

def get_tz(city):
    p = subprocess.run(["date", "+%z"], env={"TZ": city}, capture_output=True)
    out = p.stdout
    hr = int(out[1:3])
    mn = int(out[3:5])
    if out[0] == "-":
        hr = -hr
        mn = -mn
    return hr, mn

def get_cities():
    with open('/usr/share/zoneinfo/zone.tab') as f:
        reader = csv.reader(f, delimiter='\t')
        cities = [row[2] for row in reader if not row[0].startswith("#")]
        return cities

def get_time_jumps(city):
    print(city)
    p = subprocess.run(["zdump", "-v", "/usr/share/zoneinfo/{}".format(city)], capture_output=True)
    out = p.stdout
    times = out.strip().split(b"\n")
    metadata = []
    for t in times:
        if b"NULL" in t:
            continue
        elements = [e for e in t.split(b' ') if e]
        utc = b' '.join(elements[2:6]) # Does not include timezone
        local = b' '.join(elements[9:13]) # Does not include timezone
        tzname = elements[13]
        dst = int(elements[14].split(b'=')[1])
        gmtoff = int(elements[15].split(b'=')[1])
        metadata.append({city: (utc.decode(), local.decode(), tzname, dst, gmtoff)});
    return metadata

def print_output(city_to_tz):
    print("// Autogenerated by gen-tz-data.py. Do not modify.")
    print("#ifndef X_DATETIME_TIMEZONE_H")
    print("#define X_DATETIME_TIMEZONE_H")
    print("#include <string>")
    print("#include <map>")
    print("#include <vector>")
    print("#include <time.h>")
    print("namespace xDateTime {")
    print("struct Timezone {")
    print("    std::string name;")
    #print("    bool is_city;")
    print("    std::vector<time_t> utc;")
    print("    std::vector<time_t> local;")
    print("    std::vector<std::string> tzname;")
    print("    std::vector<int> dst;")
    print("    std::vector<int> gmtoff; // seconds")
    print("    static std::string CalcOffset(int hour, int minute) {")
    print("        std::string n = std::to_string(hour) + ((minute > 0) ? \":\" + std::to_string(minute) : \"\");")
    print("        n.insert(0, hour < 0 ? \"UTC-\" : \"UTC+\");")
    print("        return n;")
    print("    }")
    print("    Timezone() {}")
    print("    Timezone(int hour, int minute) {")
    print("        name = Timezone::CalcOffset(hour, minute);")
    #print("        is_city = false;")
    print("        gmtoff.push_back(minute * 60 + hour * 60 * 60);")
    print("    }")
    print("    Timezone(const std::string& name_, int hour, int minute) {")
    print("        name = name_;")
    #print("        is_city = false;")
    print("        gmtoff.push_back(minute * 60 + hour * 60 * 60);")
    print("    }")
    print("};\n")
    print("static inline std::map<std::string, Timezone> InitializeTimezones();")
    print("static inline Timezone TZ(const std::string& name) { return InitializeTimezones()[name]; }")
    print("struct BaseTimezone {")
    print("    Timezone tz;")
    print("    BaseTimezone() { tz = TZ(\"UTC\"); }")
    print("};\n")
    print("static inline std::map<std::string, Timezone> InitializeTimezones() {")
    print("    static std::map<std::string, Timezone> timezones; ")
    print("    static bool initialized = false; ")
    print("    struct tm t;")
    print("    if (initialized) return timezones;\n")
    print("    // Cities markes as time zone reference points")
    print("    // Note: None of these conversions allow auto DST because we apply it ourselves")
    print("    // based on the info we have handy.")
    for city, metadata in city_to_tz.items():
        print("    timezones[\"{}\"] = Timezone();".format(city))
        print("    timezones[\"{}\"].name = \"{}\";".format(city, city))
        #print("    timezones[\"{}\"].is_city = true;\n".format(city))
        for entry in metadata:
             print("    strptime(\"{}\", \"%b %d %H:%M:%S %Y\", &t);".format(entry.utc))
             print("    t.tm_isdst = 0;")
             print("    timezones[\"{}\".utc.push_back(mktime(&t));".format(city))
             print("    strptime(\"{}\", \"%b %d %H:%M:%S %Y\", &t);".format(entry.local))
             print("    t.tm_isdst = 0;")
             print("    timezones[\"{}\".local.push_back(mktime(&t));".format(city))
             print("    timezones[\"{}\".tzname.push_back(\"{}\");".format(city, entry.tzname))
             print("    timezones[\"{}\".dst.push_back({});".format(city, entry.dst))
             print("    timezones[\"{}\".gmtoff.push_back({});\n".format(city, entry.gmtoff))
        print("") # another empty line

    print("    // Time zone offsets from UTC")
    for h in range(-12, 14+1):
        for m in [0, 15, 30, 45]:
            print("    timezones[Timezone::CalcOffset({}, {})] = Timezone({}, {});".format(h, m, h, m))
    print("") # another empty line

    print("    // Time zone names")
    for name, hm in tznames.items():
        print("    timezones[\"{}\"] = Timezone(\"{}\", {}, {});".format(name, name, hm[0], hm[1]))   
    print("") # another empty line
    print("    initialized = true;");
    print("    return timezones;");
    print("}\n")
    for name, hm in tznames.items():
        if name.upper() == name: #To identify the time zone appreviations
            replaced_name = name.replace("+", "p").replace("-", "m")
            print("struct {}_Timezone: public BaseTimezone {{".format(replaced_name))
            print("    {}_Timezone(): BaseTimezone() {{".format(replaced_name))
            print("        tz = TZ(\"{}\");".format(name))
            print("    }")
            print("};")
    for h in range(-12, 14+1):
        for m in [0, 15, 30, 45]:
            if h == 14 and m > 0:
                break
            name = "UTC{}".format(h)
            if (m > 0):
                name += ":{}".format(m)
            replaced_name = name.replace("+", "p").replace("-", "m").replace(":", '')
            print("struct {}_Timezone: public BaseTimezone {{".format(replaced_name))
            print("    Timezone tz;")
            print("    {}_Timezone(): BaseTimezone() {{".format(replaced_name))
            print("        tz = TZ(\"{}\");".format(name))
            print("    }")
            print("};")
    print("}")
    print("#endif /* X_DATETIME_TIMEZONE_H */")

def main():
    city_to_tz = {}
    cities = get_cities()
    for city in cities:
        pass
        #city_to_tz[city] = get_time_jumps(city) # Thia takes too long.
    print_output(city_to_tz)


if __name__ == "__main__":
    main()
