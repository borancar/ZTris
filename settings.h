#ifndef SETTINGS_H
#define SETTINGS_H

#define MAXSETTINGSSTRLEN	1024
#define MAXLJUDIURANGU		10

enum {
	is_level = 0,
	is_lines,
	is_shownext,
	is_showdata,
	is_midfall,
	is_rotcw,
};

enum {
	ss_name=0,
	ss_skin,
};

typedef struct {
	const wchar_t *ime;
	int value;
} int_settings_t;

typedef struct {
	const wchar_t *ime;
	wchar_t value[MAXSETTINGSSTRLEN];
} str_settings_t;

typedef struct {
	int n;
	struct {
		wchar_t ime[MAXSETTINGSSTRLEN];
		int bodova;
	} imena[MAXLJUDIURANGU];
} scores_t;

extern int_settings_t int_settings[];
extern str_settings_t str_settings[];
extern scores_t scores;

int LoadSettings();
int SaveSettings();
int ShowSettings();
int AddToRang(const wchar_t *ime, int bodova);

#endif