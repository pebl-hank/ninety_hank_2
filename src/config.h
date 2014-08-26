//NOTE: longitude is positive for East and negative for West
//#define LATITUDE    51.0
//#define LONGITUDE 8.0
//#define TIMEZONE +1
//#define DAY_NAME_LANGUAGE DAY_NAME_GERMAN 				// Valid values: DAY_NAME_ENGLISH, DAY_NAME_GERMAN, DAY_NAME_FRENCH
//#define MOONPHASE_NAME_LANGUAGE MOONPHASE_TEXT_GERMAN 	// Valid values: MOONPHASE_TEXT_ENGLISH, MOONPHASE_TEXT_GERMAN, MOONPHASE_TEXT_FRENCH
//#define day_month_x day_month_day_first 				// Valid values: day_month_month_first, day_month_day_first
#define TRANSLATION_CW "KW%V" 							// Translation for the calendar week (e.g. "CW%V")

// ----- Additional time zones to display on the top right


static char AdditionalTimezone_1_Description[] = "EDT"; // Timezone name to display
static char AdditionalTimezone_2_Description[] = "HME"; // Timezone name to display
// ----- Additional time zones to display on the top right


// ---- Constants for all available languages ----------------------------------------

 int day_month_day_first[] = {
	75,
	108
};

 int day_month_month_first[] = {
	108,
	75
};

const char *CW_NAME[3] = {
	"KW%V",
	"CW%V",
	"S%V"
};

const char *DAY_NAME[7][3] = {
	{"SON","SUN","DIM"},
	{"MON","MON","LUN"},
	{"DIE","TUE","MAR"},
	{"MIT","WED","MER"},
	{"DON","THU","JEU"},
	{"FRE","FRE","VEN"},
	{"SAM","SAT","SAM"}
};

const char *MOONPHASE_TEXT[8][3] = {
	{"NM","NM","NL"},
	{"NM+","NM+","NL+"},
	{"NM++","NM++","NL++"},
	{"VM-","FM-","PL-"},
	{"VM","FM","PL"},
	{"VM+","FM+","PL+"},
	{"VM++","FM++","PL++"},
	{"NM-","NM-","NL-"}
};
