//NOTE: longitude is positive for East and negative for West
//#define LATITUDE    51.0
//#define LONGITUDE 8.0
//#define TIMEZONE +1
#define DAY_NAME_LANGUAGE DAY_NAME_GERMAN 				// Valid values: DAY_NAME_ENGLISH, DAY_NAME_GERMAN, DAY_NAME_FRENCH
#define MOONPHASE_NAME_LANGUAGE MOONPHASE_TEXT_GERMAN 	// Valid values: MOONPHASE_TEXT_ENGLISH, MOONPHASE_TEXT_GERMAN, MOONPHASE_TEXT_FRENCH
#define day_month_x day_month_day_first 				// Valid values: day_month_month_first, day_month_day_first
#define TRANSLATION_CW "KW%V" 							// Translation for the calendar week (e.g. "CW%V")

// ----- Additional time zones to display on the top right


static char AdditionalTimezone_1_Description[] = "EDT"; // Timezone name to display
static char AdditionalTimezone_2_Description[] = "HME"; // Timezone name to display
// ----- Additional time zones to display on the top right


// ---- Constants for all available languages ----------------------------------------

const int day_month_day_first[] = {
	75,
	108
};

const int day_month_month_first[] = {
	108,
	75
};

const char *MOONPHASE_TEXT_GERMAN[] = {
	"NM",
	"NM+",
	"NM++",
	"VM-",
	"VM",
	"VM+",
	"VM++",
	"NM-"
};

const char *DAY_NAME_GERMAN[] = {
	"SON",
	"MON",
	"DIE",
	"MIT",
	"DON",
	"FRE",
	"SAM"
};

const char *DAY_NAME_ENGLISH[] = {
	"SUN",
	"MON",
	"TUE",
	"WED",
	"THU",
	"FRE",
	"SAT"
};

const char *MOONPHASE_TEXT_ENGLISH[] = {
	"NM",
	"NM+",
	"NM++",
	"FM-",
	"FM",
	"FM+",
	"FM++",
	"NM-"
};

const char *DAY_NAME_FRENCH[] = {
    "DIM",
    "LUN",
    "MAR",
    "MER",
    "JEU",
    "VEN",
    "SAM"
};
const char *MOONPHASE_TEXT_FRENCH[] = {
    "NL",
    "NL+",
    "NL++",
    "PL-",
    "PL",
    "PL+",
    "PL++",
    "NL-"
};
