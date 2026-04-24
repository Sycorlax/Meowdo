/*
 * meowdo  ---  a cute bongo cat todo list ---
 *
 * compile:  gcc -O2 -o meowdo meowdo.c -lncursesw       (all distros, wide-char)
 * arch:     sudo pacman -S ncurses
 * debian:   sudo apt install libncursesw5-dev
 *
 * ------ keys ------------------------------------------------------------------------------------------------------------------------------------------
 *  j/k  ------            navigate
 *  g / G              jump top / bottom
 *  PgUp / PgDn        page up / down
 *  n                  new task
 *  e                  edit selected task
 *  Space              toggle done
 *  p                  toggle pin  ---
 *  t                  set / clear tag
 *  d                  delete selected task
 *  D                  delete ALL tasks
 *  /                  search
 *  Esc                clear search + filter
 *  1-6                filter by tag
 *  0                  show all
 *  q                  quit
 * ------------------------------------------------------------------------------------------------------------------------------------------------------------------
 *
 * data: ~/.meowdo/todos.txt
 * line format: <P|->|<x| >|<tag>|<text>|<created_ts>|<done_ts>
 */

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 700
#endif
#include <ncurses.h>
#include <locale.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>
#include <ctype.h>
#include <stdbool.h>
#include <wchar.h>

/* ------ constants ------ */
#define MAX_TODOS  1024
#define MAX_LINE    256
#define MAX_TAG      32
#define MAX_UTAGS    64
#define LIST_Y0       4
#define BC_H         10

/* layout breakpoints */
#define MIN_COLS_FULL    80   /* below this: hide right pane */
#define MIN_COLS_COMPACT 20   /* below this: too small overlay */
#define MIN_ROWS         8    /* below this: too small overlay */

typedef enum { LAYOUT_FULL, LAYOUT_COMPACT, LAYOUT_TOOSMALL } Layout;

/* ------ colour pairs ------ */
enum {
    C_BORDER=1, C_TOPBAR=2, C_SEL=3,    C_DONE=4,  C_PEND=5,
    C_BONGO=6,  C_SBAR=7,   C_HDR=8,    C_GREEN=9,  C_PIN=10,
    C_TAG1=11,  C_TAG2=12,  C_TAG3=13,  C_TAG4=14,
    C_SEARCH=15,C_BEAT=16,  C_TITLE=17,
    C_TAG5=20,  C_TAG6=21,  C_TAG7=22,  C_TAG8=23,
};

/* ── UTF-8 capability flag (set at startup) ── */
static bool g_utf8 = 0;

/* ── bongo art: braille (UTF-8) ── */
static const char *BONGO_UTF8[BC_H] = {
    "\xe2\xa0\x80\xe2\xa0\x80\xe2\xa0\x80\xe2\xa0\x80\xe2\xa0\x80\xe2\xa0\x80\xe2\xa2\x80\xe2\xa3\xbf\xe2\xa1\x80\xe2\xa0\x80\xe2\xa0\x80\xe2\xa0\x80\xe2\xa0\x80\xe2\xa0\x80\xe2\xa0\x80\xe2\xa0\x80\xe2\xa0\x80",
    "\xe2\xa0\x80\xe2\xa0\x80\xe2\xa0\x80\xe2\xa0\x80\xe2\xa0\x80\xe2\xa2\x80\xe2\xa3\xbe\xe2\xa3\xbf\xe2\xa1\x87\xe2\xa0\x80\xe2\xa0\x80\xe2\xa0\x80\xe2\xa0\x80\xe2\xa0\x80\xe2\xa2\x80\xe2\xa3\xbc\xe2\xa1\x87",
    "\xe2\xa0\x80\xe2\xa0\x80\xe2\xa0\x80\xe2\xa0\x80\xe2\xa0\x80\xe2\xa3\xb8\xe2\xa3\xbf\xe2\xa3\xbf\xe2\xa1\x87\xe2\xa0\x80\xe2\xa0\x80\xe2\xa0\x80\xe2\xa0\x80\xe2\xa3\xb4\xe2\xa3\xbf\xe2\xa3\xbf\xe2\xa0\x83",
    "\xe2\xa0\x80\xe2\xa0\x80\xe2\xa0\x80\xe2\xa0\x80\xe2\xa2\xa0\xe2\xa3\xbf\xe2\xa3\xbf\xe2\xa3\xbf\xe2\xa3\x87\xe2\xa0\x80\xe2\xa0\x80\xe2\xa2\x80\xe2\xa3\xbe\xe2\xa3\xbf\xe2\xa3\xbf\xe2\xa3\xbf\xe2\xa0\x80",
    "\xe2\xa0\x80\xe2\xa0\x80\xe2\xa0\x80\xe2\xa3\xb4\xe2\xa3\xbf\xe2\xa3\xbf\xe2\xa3\xbf\xe2\xa3\xbf\xe2\xa3\xbf\xe2\xa3\xbf\xe2\xa3\xb7\xe2\xa3\xbf\xe2\xa3\xbf\xe2\xa3\xbf\xe2\xa3\xbf\xe2\xa1\x9f\xe2\xa0\x80",
    "\xe2\xa0\x80\xe2\xa0\x80\xe2\xa2\xb0\xe2\xa1\xbf\xe2\xa0\x89\xe2\xa0\x80\xe2\xa1\x9c\xe2\xa3\xbf\xe2\xa3\xbf\xe2\xa3\xbf\xe2\xa1\xbf\xe2\xa0\xbf\xe2\xa2\xbf\xe2\xa3\xbf\xe2\xa3\xbf\xe2\xa0\x83\xe2\xa0\x80",
    "\xe2\xa0\x92\xe2\xa0\x92\xe2\xa0\xb8\xe2\xa3\xbf\xe2\xa3\x84\xe2\xa1\x98\xe2\xa3\x83\xe2\xa3\xbf\xe2\xa3\xbf\xe2\xa1\x9f\xe2\xa2\xb0\xe2\xa0\x83\xe2\xa0\x80\xe2\xa2\xb9\xe2\xa3\xbf\xe2\xa1\x87\xe2\xa0\x80",
    "\xe2\xa0\x9a\xe2\xa0\x89\xe2\xa0\x80\xe2\xa0\x88\xe2\xa0\xbb\xe2\xa3\xbf\xe2\xa3\xbf\xe2\xa3\xbf\xe2\xa3\xbf\xe2\xa3\xbf\xe2\xa3\xae\xe2\xa3\xa4\xe2\xa3\xa4\xe2\xa3\xbf\xe2\xa1\x9f\xe2\xa0\x81\xe2\xa0\x80",
    "\xe2\xa0\x80\xe2\xa0\x80\xe2\xa0\x80\xe2\xa0\x80\xe2\xa0\x80\xe2\xa0\x80\xe2\xa0\x88\xe2\xa0\x99\xe2\xa0\x9b\xe2\xa0\x9b\xe2\xa0\x9b\xe2\xa0\x9b\xe2\xa0\x9b\xe2\xa0\x81\xe2\xa0\x80\xe2\xa0\x92\xe2\xa0\xa4",
    "\xe2\xa0\x80\xe2\xa0\x80\xe2\xa0\x80\xe2\xa0\x80\xe2\xa0\x80\xe2\xa0\x80\xe2\xa0\x80\xe2\xa0\x80\xe2\xa0\x80\xe2\xa0\x80\xe2\xa0\x80\xe2\xa0\x80\xe2\xa0\x80\xe2\xa0\x80\xe2\xa0\x91\xe2\xa0\x80\xe2\xa0\x80",
};

/* ── bongo art: ASCII fallback ── */
static const char *BONGO_ASCII[BC_H] = {
    "        .       .        ",
    "        |\\_---_/|        ",
    "       /   o_o   \\       ",
    "       |    U    |       ",
    "      \\  ._I_.  /       ",
    "        `-_____-'        ",
    "                         ",
    "     					   ",
    "                         ",
};

/* active bongo pointer — set at startup */
static const char **BONGO = NULL;


/* ------ data ------ */
typedef struct {
    bool   done, pinned;
    char   tag[MAX_TAG];
    char   text[MAX_LINE];
    time_t created_at;
    time_t done_at;
} Todo;

static Todo todos[MAX_TODOS];
static int  todo_count = 0;
static int  sel        = 0;
static char search[MAX_LINE]    = "";
static char filter_tag[MAX_TAG] = "";
static int  vis[MAX_TODOS];
static int  vis_count  = 0;
static char smsg[256]  = "";
static int  smsg_ttl   = 0;
static int  celebrate  = 0;

/* ------ tag cache ------ */
static char utags[MAX_UTAGS][MAX_TAG];
static int  utag_counts[MAX_UTAGS];
static int  tag_cnt_for[MAX_TODOS];
static int  utag_n     = 0;
static int  tags_dirty = 1;

/* frame-cached counts --- computed once per frame, not per draw call */
static int  g_pend=0, g_done_c=0, g_pin_c=0;

/* ------ helpers ------ */
static int tag_color(const char *tag) {
    if (!tag || !tag[0] || strcmp(tag,"none")==0) return C_PEND;
    unsigned h = 0;
    for (const char *p=tag; *p; p++) h = h*31+(unsigned char)*p;
    static const int cols[]={C_TAG1,C_TAG2,C_TAG3,C_TAG4,C_TAG5,C_TAG6,C_TAG7,C_TAG8};
    return cols[h%8];
}

static const char *bongo_mood(int p) {
    if (p==0 && todo_count>0) return "  purrfect!! all done~ ";
    if (p==0)                  return "  add a task nya~";
    if (p==1)                  return "  one more! you got it!";
    if (p<=3)                  return "  almost there, meow~";
    if (p<=7)                  return "  *meow meow! you productive nya~*";
    return                            "  !!so many tasks hiss! ";
}

/* ── UTF-8 detection: check LC_ALL / LC_CTYPE / LANG for UTF-8 ── */
static bool detect_utf8(void) {
    const char *vars[] = { "LC_ALL", "LC_CTYPE", "LANG", NULL };
    for (int i = 0; vars[i]; i++) {
        const char *v = getenv(vars[i]);
        if (!v || !*v) continue;
        char buf[strlen(v) + 1]; int j;
        for (j=0; v[j] && j<63; j++) buf[j]=(char)tolower((unsigned char)v[j]);
        buf[j]='\0';
        if (strstr(buf,"utf")) return true;
    }
    return false;
}


static void set_smsg(const char *m) {
    strncpy(smsg,m,sizeof smsg-1); smsg[sizeof smsg-1]='\0'; smsg_ttl=5;
}

/* ------ paths ------ */
static char* todo_path;
size_t todo_path_length;
static void build_paths(void) {
    #define LOCAL_BUFFER_SIZE 64

    const char *home=getenv("HOME");
    const char *xdg=getenv("XDG_DATA_HOME");

    size_t base_length = 2;
    if(home)
        base_length = strlen(home);
    if(xdg && strlen(xdg)>base_length)
        base_length = strlen(xdg);
    char base[base_length + 1 + LOCAL_BUFFER_SIZE];

    if(xdg && xdg[0])
        snprintf(base,sizeof base,"%s/meowdo",xdg);
    else if(home && home[0])
        snprintf(base,sizeof base,"%s/.local/share/meowdo",home);
    else
        strcpy(base, ".");

    if (mkdir(base,0755)!=0 && errno!=EEXIST){
        todo_path_length = LOCAL_BUFFER_SIZE;
        todo_path = malloc(todo_path_length);
        strcpy(todo_path, "todos.txt");
        return;
    }

    todo_path_length = strlen(base) + 1 + LOCAL_BUFFER_SIZE;
    todo_path = malloc(todo_path_length);
    snprintf(todo_path,todo_path_length,"%s/todos.txt",base);
}

/* ------ tag cache ------ */
static void rebuild_tag_cache(void) {
    utag_n=0;
    for (int i=0;i<todo_count;i++) {
        if (!todos[i].tag[0]||strcmp(todos[i].tag,"none")==0) continue;
        int found=-1;
        for (int j=0;j<utag_n;j++)
            if (strcmp(utags[j],todos[i].tag)==0){found=j;break;}
        if (found<0 && utag_n<MAX_UTAGS) {
            strncpy(utags[utag_n],todos[i].tag,MAX_TAG-1);
            utags[utag_n][MAX_TAG-1]='\0';
            utag_counts[utag_n]=1; utag_n++;
        } else if (found>=0) utag_counts[found]++;
    }
    for (int i=0;i<todo_count;i++) {
        tag_cnt_for[i]=0;
        for (int j=0;j<utag_n;j++)
            if (strcmp(utags[j],todos[i].tag)==0){tag_cnt_for[i]=utag_counts[j];break;}
    }
    tags_dirty=0;
}
static void mark_dirty(void){ tags_dirty=1; }

/* one pass to count pending/done/pinned */
static void recount(void) {
    g_pend=0; g_done_c=0; g_pin_c=0;
    for (int i=0;i<todo_count;i++){
        if(todos[i].done) g_done_c++; else g_pend++;
        if(todos[i].pinned) g_pin_c++;
    }
}

/* ------ I/O ------ */
static void todos_load(void) {
    todo_count=0;
    FILE *f=fopen(todo_path,"r"); if(!f) return;
    char line[MAX_LINE+MAX_TAG+64];
    while (todo_count<MAX_TODOS && fgets(line,(int)sizeof line,f)) {
        int len=(int)strlen(line);
        if (len>0&&line[len-1]=='\n') line[--len]='\0';
        if (len<5) continue;
        Todo *t=&todos[todo_count];
        t->done_at=0;
        t->tag[0]='\0';t->text[0]='\0';
        t->pinned=(line[0]=='P');
        t->done  =(line[2]=='x');
        char *p1=strchr(line+4,'|'); if(!p1) continue;
        int tlen=(int)(p1-(line+4));
        if(tlen<0) continue;
        if(tlen>=MAX_TAG) tlen=MAX_TAG-1;
        snprintf(t->tag, MAX_TAG, "%.*s", tlen, line+4);
        char *p2=p1+1, *p3=strchr(p2,'|');
        if(p3){
            int txtlen=(int)(p3-p2); if(txtlen<0)txtlen=0;
            if(txtlen>=MAX_LINE)txtlen=MAX_LINE-1;
            strncpy(t->text,p2,(size_t)txtlen); t->text[txtlen]='\0';
            char *p4=strchr(p3+1,'|');
            t->created_at=(time_t)atol(p3+1);
            if(p4) t->done_at=(time_t)atol(p4+1);
        } else {
            strncpy(t->text,p2,MAX_LINE-1); t->text[MAX_LINE-1]='\0';
            t->created_at=time(NULL);
        }
        todo_count++;
    }
    fclose(f); mark_dirty();
}

static void todos_save(void) {
    FILE *f=fopen(todo_path,"w"); if(!f) return;
    for (int i=0;i<todo_count;i++){
        Todo *t=&todos[i];
        fprintf(f,"%c|%c|%s|%s|%ld|%ld\n",
            t->pinned?'P':'-', t->done?'x':' ',
            t->tag[0]?t->tag:"none", t->text,
            (long)t->created_at,(long)t->done_at);
    }
    fclose(f);
}

/* ------ sort / filter ------ */
static int todo_cmp(const void *a,const void *b){
    const Todo *A=a,*B=b;
    if(A->pinned!=B->pinned) return B->pinned?1:-1;
    if(A->done  !=B->done)   return A->done  ?1:-1;
    return 0;
}

static void rebuild_vis(void) {
    qsort(todos,(size_t)todo_count,sizeof(Todo),todo_cmp);
    vis_count=0;
    char ndl[MAX_LINE]="";
    if (search[0]) {
        snprintf(ndl,sizeof ndl,"%s",search);
        for(int j=0;ndl[j];j++) ndl[j]=(char)tolower((unsigned char)ndl[j]);
    }
    for(int i=0;i<todo_count;i++){
        Todo *t=&todos[i];
        if(filter_tag[0]&&strcmp(t->tag,filter_tag)!=0) continue;
        if(search[0]){
            char hay[MAX_LINE];
            snprintf(hay,sizeof hay,"%s",t->text);
            for(int j=0;hay[j];j++) hay[j]=(char)tolower((unsigned char)hay[j]);
            if(!strstr(hay,ndl)) continue;
        }
        vis[vis_count++]=i;
    }
    if(sel>=vis_count) sel=vis_count>0?vis_count-1:0;
    mark_dirty();
}

/* ------ timestamp ------ */
static void fmt_ts(time_t ts,char *buf,int buflen){
    if(!ts){strncpy(buf,"-",(size_t)buflen);buf[buflen-1]='\0';return;}
    struct tm tmbuf; struct tm *tm=localtime_r(&ts,&tmbuf);
    if(tm) strftime(buf,(size_t)buflen,"%d %b %H:%M",tm);
    else   strncpy(buf,"?",(size_t)buflen);
}

/* ------ layout detection ------ */
static Layout get_layout(int rows, int cols) {
    if (rows < MIN_ROWS || cols < MIN_COLS_COMPACT) return LAYOUT_TOOSMALL;
    if (cols < MIN_COLS_FULL)                        return LAYOUT_COMPACT;
    return LAYOUT_FULL;
}

/* ------ popup helpers ------ */

/*
 * popup_draw_field: render the input field across one or more lines,
 * correctly handling multi-byte UTF-8 (e.g. Cyrillic) by measuring
 * display columns rather than byte lengths.
 *
 * Returns the (row, col) position where the cursor should be placed.
 *
 * Layout inside the window (pw wide):
 *   col 3: "> "   (2 chars prefix)
 *   col 5..pw-3:  text, wrapped to field_w = pw-7 display columns per line
 *
 * We walk the UTF-8 string character-by-character using mbtowc(),
 * tracking display columns consumed on the current line.  When a
 * character would overflow the line we move to the next row.
 */
static void popup_draw_field(WINDOW *win, int start_row, int field_w,
                             const char *s,
                             int *cur_row_out, int *cur_col_out)
{
    /* clear all field rows first — up to 8 extra lines is plenty */
    for (int r = start_row; r < start_row + 8; r++)
        mvwprintw(win, r, 3, "%-*s", field_w + 2, "");

    wattron(win, COLOR_PAIR(C_PEND)|A_BOLD);
    mvwprintw(win, start_row, 3, "> ");
    wattroff(win, COLOR_PAIR(C_PEND)|A_BOLD);

    int row = start_row;
    int col = 5;          /* column after "> " */
    int col_used = 0;     /* display columns used on this line */
    const char *p = s;

    while (*p) {
        wchar_t wc;
        int mb = mbtowc(&wc, p, MB_CUR_MAX);
        if (mb <= 0) { p++; continue; }   /* skip invalid bytes */

        int wcw = wcwidth(wc);
        if (wcw < 0) wcw = 0;             /* non-printable → 0 width */

        /* wrap to next line if this char doesn't fit */
        if (col_used + wcw > field_w) {
            /* pad remainder of current line */
            wattron(win, COLOR_PAIR(C_PEND)|A_BOLD);
            mvwprintw(win, row, col, "%-*s", field_w - col_used, "");
            wattroff(win, COLOR_PAIR(C_PEND)|A_BOLD);
            row++;
            col = 5; col_used = 0;
            /* continuation lines: no "> " prefix, just indent */
            wattron(win, COLOR_PAIR(C_PEND)|A_BOLD);
            mvwprintw(win, row, 3, "  ");
            wattroff(win, COLOR_PAIR(C_PEND)|A_BOLD);
        }

        /* print the character */
        char mb_buf[MB_CUR_MAX + 1];
        memcpy(mb_buf, p, (size_t)mb);
        mb_buf[mb] = '\0';
        wattron(win, COLOR_PAIR(C_PEND)|A_BOLD);
        mvwprintw(win, row, col, "%s", mb_buf);
        wattroff(win, COLOR_PAIR(C_PEND)|A_BOLD);

        col      += wcw;
        col_used += wcw;
        p        += mb;
    }

    *cur_row_out = row;
    *cur_col_out = col;
}

/*
 * popup_needed_rows: how many input-field rows does the current text
 * need given field_w display columns per row?
 */
static int popup_needed_rows(const char *s, int field_w) {
    if (!s || !s[0] || field_w < 1) return 1;
    int rows = 1, col_used = 0;
    const char *p = s;
    while (*p) {
        wchar_t wc;
        int mb = mbtowc(&wc, p, MB_CUR_MAX);
        if (mb <= 0) { p++; continue; }
        int wcw = wcwidth(wc); if (wcw < 0) wcw = 0;
        if (col_used + wcw > field_w) { rows++; col_used = 0; }
        col_used += wcw;
        p += mb;
    }
    return rows;
}

/* ------ popup ------ */
/*
 * Window row layout (ph rows total):
 *   row 0        top border          (drawn by box())
 *   row 1        title bar           (drawn by box() + title)
 *   row 2        hint text           (A_DIM)
 *   row 3        blank               (inside box)
 *   row 4..4+FR-1  input field lines  (FR = field_rows)
 *   row 4+FR     blank separator      (inside box)
 *   row 4+FR+1   "Enter:confirm…"    (A_DIM, second-to-last inside row)
 *   row ph-1     bottom border       (drawn by box())
 *
 *   ph = 4 + FR + 2 + 1(bottom border) = FR + 7
 *
 * popup_draw_field is told the first field row (4) AND the last
 * allowed field row (4+FR-1) so it never writes into the separator
 * or the hint bar.
 */
static int popup(const char *title, const char *hint, char *out, int maxlen){
    int rows, cols; getmaxyx(stdscr, rows, cols);
    int pw = (cols < 74) ? cols - 4 : 74;
    if (pw < 20 || rows < 9) return 0;

    int field_w = pw - 7;   /* usable display columns per input line */

    /* ph_for: given field_rows, compute total window height */
    #define PH_FOR(fr) ((fr) + 7)
    /* clamp ph so it fits the terminal and has a sane minimum */
    #define CLAMP_PH(ph) do { \
        if ((ph) > rows - 2) (ph) = rows - 2; \
        if ((ph) < PH_FOR(1)) (ph) = PH_FOR(1); \
    } while(0)

    int field_rows = popup_needed_rows(out, field_w);
    if (field_rows < 1) field_rows = 1;
    int ph = PH_FOR(field_rows);
    CLAMP_PH(ph);

    int px = (cols - pw) / 2;
    int py = rows/2 - ph/2;
    WINDOW *p = newwin(ph, pw, py, px); if (!p) return 0;

    /* full redraw: chrome + field, bottom bar always at ph-2 (inside border) */
    #define POPUP_REDRAW() do { \
        werase(p); \
        wattron(p,COLOR_PAIR(C_BORDER)|A_BOLD); box(p,0,0); wattroff(p,COLOR_PAIR(C_BORDER)|A_BOLD); \
        wattron(p,COLOR_PAIR(C_TOPBAR)|A_BOLD); mvwprintw(p,0,2," %s ",title); wattroff(p,COLOR_PAIR(C_TOPBAR)|A_BOLD); \
        if(hint&&hint[0]){wattron(p,A_DIM); mvwprintw(p,2,3,"%.*s",pw-6,hint); wattroff(p,A_DIM);} \
        wattron(p,A_DIM); mvwprintw(p,ph-2,3," Enter:confirm   Esc:cancel "); wattroff(p,A_DIM); \
        popup_draw_field(p, 4, field_w, out, &cur_row, &cur_col); \
    } while(0)

    curs_set(1);
    int len = (int)strnlen(out, (size_t)(maxlen-1));
    int cancelled = 0;
    int cur_row = 4, cur_col = 5;

    POPUP_REDRAW();
    wmove(p, cur_row, cur_col); wrefresh(p);

    while (1) {
        int ch = wgetch(p);
        if (ch == 27 || ch == KEY_F(1)) { out[0]='\0'; cancelled=1; break; }
        if (ch == '\n' || ch == '\r') break;

        if ((ch == KEY_BACKSPACE || ch == 127 || ch == '\b') && len > 0) {
            /* walk back past UTF-8 continuation bytes (10xxxxxx) */
            do { len--; } while (len > 0 && ((unsigned char)out[len] & 0xC0) == 0x80);
            out[len] = '\0';
        }
        /* accept bytes ≥ 0x80 so UTF-8 continuation bytes pass through */
        else if (((ch >= 32 && ch < 256) || (ch & 0x80)) && len < maxlen-1) {
            out[len++] = (char)ch; out[len] = '\0';
        }

        /* resize popup if text now wraps to a different number of lines */
        int new_fr = popup_needed_rows(out, field_w);
        if (new_fr < 1) new_fr = 1;
        int new_ph = PH_FOR(new_fr);
        CLAMP_PH(new_ph);

        if (new_ph != ph) {
            ph = new_ph;
            delwin(p);
            py = rows/2 - ph/2;
            p = newwin(ph, pw, py, px);
            if (!p) break;
        }

        POPUP_REDRAW();
        wmove(p, cur_row, cur_col); wrefresh(p);
    }

    #undef POPUP_REDRAW
    #undef PH_FOR
    #undef CLAMP_PH
    curs_set(0); delwin(p); touchwin(stdscr); refresh();
    return (!cancelled && len > 0);
}

/* ------ draw: too small overlay ------ */
static void draw_toosmall(int rows, int cols) {
    erase();
    attron(COLOR_PAIR(C_TOPBAR)|A_BOLD);
    for(int r=0;r<rows;r++) mvhline(r,0,' ',cols);

    static const char *lines[] = {
        "=^..^=",
        "terminal too small~",
        "please resize nya!",
    };
    int n = 3;
    int sy = rows/2 - n/2; if(sy<0) sy=0;
    for(int i=0;i<n&&sy+i<rows;i++){
        int slen=(int)strlen(lines[i]);
        int sx=(cols-slen)/2; if(sx<0)sx=0;
        mvprintw(sy+i, sx, "%s", lines[i]);
    }
    attroff(COLOR_PAIR(C_TOPBAR)|A_BOLD);
    refresh();
}

/* ------ draw: top bar ------ */
static void draw_top(WINDOW *w,int cols){
    if(!w) return;
    werase(w);
    wattron(w,COLOR_PAIR(C_TOPBAR)|A_BOLD);
    mvwprintw(w,0,0,"%*s",cols,"");
    mvwprintw(w,0,2," =^..^=  meowdo ");
    int cx=20;
    if(filter_tag[0]){
        wattron(w,COLOR_PAIR(C_TAG1)|A_BOLD);
        mvwprintw(w,0,cx," #%s ",filter_tag); cx+=(int)strlen(filter_tag)+4;
        wattron(w,COLOR_PAIR(C_TOPBAR)|A_BOLD);
    }
    if(search[0]){
        wattron(w,COLOR_PAIR(C_SEARCH)|A_BOLD);
        mvwprintw(w,0,cx," /%s/ ",search);
        wattron(w,COLOR_PAIR(C_TOPBAR)|A_BOLD);
    }
    time_t now=time(NULL); struct tm tb; struct tm *tm=localtime_r(&now,&tb);
    if(tm){
        char ts[32]; strftime(ts,sizeof ts,"%H:%M  %a %d %b",tm);
        mvwprintw(w,0,cols-(int)strlen(ts)-2,"%s",ts);
    }
    wattroff(w,COLOR_PAIR(C_TOPBAR)|A_BOLD);
    wnoutrefresh(w);
}

/* ------ draw: status bar ------ */
static void draw_sbar(WINDOW *w,int cols){
    if(!w) return;
    werase(w);
    wattron(w,COLOR_PAIR(C_SBAR)|A_BOLD);
    mvwprintw(w,0,0,"%*s",cols,"");
    if(cols>100)
        mvwprintw(w,0,1," n:new  e:edit  Spc:done  p:pin  t:tag  d:del  D:del-all  /:search  0-6:filter  Esc:clear  q:quit");
    else if(cols>50)
        mvwprintw(w,0,1," n:new  e:edit  Spc:done  d:del  D:all  /:search  q:quit");
    else
        mvwprintw(w,0,1," n  e  Spc  d  D  /  q");
    if(smsg[0]&&smsg_ttl>0){
        int slen=(int)strlen(smsg);
        if(cols-slen-2>2){
            wattron(w,COLOR_PAIR(C_GREEN)|A_BOLD);
            mvwprintw(w,0,cols-slen-2,"%s",smsg);
            wattroff(w,COLOR_PAIR(C_GREEN)|A_BOLD);
        }
    }
    wattroff(w,COLOR_PAIR(C_SBAR)|A_BOLD);
    wnoutrefresh(w);
}

/* ------ draw: progress bar ------ */
static void draw_progress(WINDOW *w,int y,int x,int width){
    if(!w||todo_count==0||width<4) return;
    int filled=(width*g_done_c)/todo_count;
    int pct=(g_done_c*100)/todo_count;
    wattron(w,COLOR_PAIR(C_GREEN)|A_BOLD); mvwprintw(w,y,x,"[");
    for(int i=0;i<width;i++){
        if(i<filled){wattron(w,COLOR_PAIR(C_GREEN)|A_BOLD);waddch(w,'#');}
        else        {wattron(w,COLOR_PAIR(C_DONE)|A_DIM); waddch(w,'-');}
    }
    wattron(w,COLOR_PAIR(C_GREEN)|A_BOLD);
    wprintw(w,"] %d%%  %d/%d",pct,g_done_c,todo_count);
    wattroff(w,A_BOLD|A_DIM);
}

/* ------ draw: celebration overlay ------ */
static void draw_celebrate(int rows,int cols){
    int cp=(celebrate%2)?COLOR_PAIR(C_TOPBAR):COLOR_PAIR(C_GREEN);
    attron(cp|A_BOLD);
    for(int r=1;r<rows-1;r++) mvhline(r,0,' ',cols);
    static const char *lines[]={
        "  =^..^=  =^..^=  =^..^=  ",
        "                           ",
        "   *  ALL TASKS DONE!!  *  ",
        "    purrfect!! nya nya~    ",
        "                           ",
        "  =^..^=  =^..^=  =^..^=  ",
    };
    int n=6, sy=(rows-1)/2-n/2; if(sy<1)sy=1;
    int sx=(cols-27)/2; if(sx<0)sx=0;
    for(int i=0;i<n&&sy+i<rows-1;i++) mvprintw(sy+i,sx,"%s",lines[i]);
    attroff(cp|A_BOLD); refresh();
}

/* ------ draw: left pane ------ */
static void draw_left(WINDOW *w,int h,int lw,int top){
    if(!w) return;
    werase(w);
    wattron(w,COLOR_PAIR(C_BORDER)|A_BOLD); box(w,0,0); wattroff(w,COLOR_PAIR(C_BORDER)|A_BOLD);
    wattron(w,COLOR_PAIR(C_HDR)|A_BOLD);   mvwprintw(w,0,2,"[ TODO ]"); wattroff(w,COLOR_PAIR(C_HDR)|A_BOLD);

    /* " N left" label — only if it fits */
    char leftlabel[20]; snprintf(leftlabel,sizeof leftlabel," %d left ",g_pend);
    int llen=(int)strlen(leftlabel);
    if(lw>llen+12){
        wattron(w,COLOR_PAIR(C_PIN)|A_BOLD);
        mvwprintw(w,0,lw-llen-1,"%s",leftlabel);
        wattroff(w,COLOR_PAIR(C_PIN)|A_BOLD);
    }

    /* progress bar — only if enough width */
    int pbw = lw - 22;
    if(pbw >= 4) draw_progress(w,1,2,pbw);

    /* column header — only if wide enough */
    if(lw > 20){
        wattron(w,A_DIM); mvwprintw(w,2,2,"  [/] #tag        task"); wattroff(w,A_DIM);
    }
    wattron(w,COLOR_PAIR(C_BORDER)|A_DIM); mvwhline(w,3,1,ACS_HLINE,lw-2); wattroff(w,COLOR_PAIR(C_BORDER)|A_DIM);

    int area=h-LIST_Y0-1; if(area<1)area=1;

    if(vis_count==0){
        wattron(w,COLOR_PAIR(C_BONGO)|A_DIM);
        int ey=h/2;
        mvwprintw(w,ey,  3,search[0]?"  no matches~ try again?":"  nothing yet! press n :3");
        mvwprintw(w,ey+1,3,search[0]?"  press Esc to clear /   ":"  meowdo is waiting nya~ ");
        wattroff(w,COLOR_PAIR(C_BONGO)|A_DIM);
        wnoutrefresh(w); return;
    }

    /* scroll arrows */
    if(top>0){
        wattron(w,COLOR_PAIR(C_SEARCH)|A_BOLD);
        mvwprintw(w,LIST_Y0-1,lw-5,"(^)");
        wattroff(w,COLOR_PAIR(C_SEARCH)|A_BOLD);
    }
    if(top+area<vis_count){
        wattron(w,COLOR_PAIR(C_SEARCH)|A_BOLD);
        mvwprintw(w,h-1,lw-5,"(v)");
        wattroff(w,COLOR_PAIR(C_SEARCH)|A_BOLD);
    }

    for(int i=0;i<area;i++){
        int vi=top+i, row=LIST_Y0+i;
        if(vi>=vis_count){ mvwprintw(w,row,1,"%*s",lw-2,""); continue; }
        Todo *t=&todos[vis[vi]];
        int is_sel=(vi==sel);

        wattrset(w,A_NORMAL);
        if(is_sel){
            wattron(w,COLOR_PAIR(C_SEL)|A_BOLD);
            mvwprintw(w,row,1,"%*s",lw-2,"");
        }

        char mark=(t->pinned&&!t->done)?'*':' ';
        char box_ch=t->done?'x':' ';
        int col_attr;
        if      (is_sel)    col_attr=COLOR_PAIR(C_SEL)|A_BOLD;
        else if (t->done)   col_attr=COLOR_PAIR(C_DONE)|A_DIM;
        else if (t->pinned) col_attr=COLOR_PAIR(C_PIN)|A_BOLD;
        else                col_attr=COLOR_PAIR(C_PEND)|A_BOLD;
        wattron(w,col_attr); mvwprintw(w,row,2,"%c [%c]",mark,box_ch);

        /* tag pill — only if enough room */
        int tx=8;
        if(t->tag[0]&&strcmp(t->tag,"none")!=0 && lw>tx+12){
            int tc=tag_color(t->tag), tcnt=tag_cnt_for[vis[vi]];
            if(!is_sel){wattrset(w,A_NORMAL);wattron(w,COLOR_PAIR(tc)|A_BOLD);}
            mvwprintw(w,row,tx,"#%.*s(%d)",5,t->tag,tcnt);
            tx+=9;
        }

        int avail=lw-tx-4; if(avail<1)avail=1;
        if     (is_sel)   {wattron(w,COLOR_PAIR(C_SEL)|A_BOLD);}
        else if(t->done)  {wattrset(w,A_NORMAL);wattron(w,COLOR_PAIR(C_DONE)|A_DIM);}
        else if(t->pinned){wattrset(w,A_NORMAL);wattron(w,COLOR_PAIR(C_PIN));}
        else              {wattrset(w,A_NORMAL);wattron(w,COLOR_PAIR(C_PEND));}
        mvwprintw(w,row,tx," %-*.*s",avail,avail,t->text);
        wattrset(w,A_NORMAL);
    }
    wnoutrefresh(w);
}

/* ------ draw: right pane ------ */
static void draw_right(WINDOW *w,int h,int rw){
    if(!w) return;
    werase(w);
    wattron(w,COLOR_PAIR(C_BORDER)|A_DIM); box(w,0,0); wattroff(w,COLOR_PAIR(C_BORDER)|A_DIM);
    int ww=getmaxx(w);

    /* bongo cat — only draw if right pane is wide enough */
    int bx = ww - 20;
    if(bx >= 1 && rw >= 22){
        wattron(w,COLOR_PAIR(C_BONGO)|A_BOLD);
        for(int i=0;i<BC_H&&i+1<h-2;i++) mvwprintw(w,i+1,bx,"%s",BONGO[i]);
        wattroff(w,COLOR_PAIR(C_BONGO)|A_BOLD);
    }

    int ca = (bx >= 1 && rw >= 22) ? bx-3 : ww-4;
    if(ca < 4){ wnoutrefresh(w); return; }

    wattron(w,COLOR_PAIR(C_HDR)|A_BOLD); mvwprintw(w,0,2,"[ MEOWDO ]"); wattroff(w,COLOR_PAIR(C_HDR)|A_BOLD);

    /* stats */
    wattron(w,COLOR_PAIR(C_PEND)|A_BOLD);  mvwprintw(w,2,2," * pending : %d",g_pend);    wattroff(w,COLOR_PAIR(C_PEND)|A_BOLD);
    wattron(w,COLOR_PAIR(C_GREEN)|A_BOLD); mvwprintw(w,3,2," x done    : %d",g_done_c);  wattroff(w,COLOR_PAIR(C_GREEN)|A_BOLD);
    wattron(w,COLOR_PAIR(C_PIN)|A_BOLD);   mvwprintw(w,4,2," ^ pinned  : %d",g_pin_c);   wattroff(w,COLOR_PAIR(C_PIN)|A_BOLD);
    wattron(w,A_DIM);                      mvwprintw(w,5,2," # total   : %d",todo_count); wattroff(w,A_DIM);

    /* mini progress */
    if(todo_count>0){
        int bw=ca-8; if(bw>20)bw=20; if(bw<4)bw=4;
        wattron(w,COLOR_PAIR(C_BORDER)|A_DIM); mvwhline(w,6,2,ACS_HLINE,ca); wattroff(w,COLOR_PAIR(C_BORDER)|A_DIM);
        int filled=(bw*g_done_c)/todo_count;
        wattron(w,COLOR_PAIR(C_GREEN)|A_BOLD); mvwprintw(w,7,2,"[");
        for(int i=0;i<bw;i++){
            if(i<filled){wattron(w,COLOR_PAIR(C_GREEN)|A_BOLD);waddch(w,'#');}
            else        {wattron(w,COLOR_PAIR(C_DONE)|A_DIM); waddch(w,'.');}
        }
        wattron(w,COLOR_PAIR(C_GREEN)|A_BOLD);
        wprintw(w,"] %d%%",(g_done_c*100)/todo_count);
        wattroff(w,A_BOLD|A_DIM);
    }

    /* tags */
    int ty=9;
    if(utag_n>0){
        wattron(w,COLOR_PAIR(C_BORDER)|A_DIM); mvwhline(w,ty,2,ACS_HLINE,ca); wattroff(w,COLOR_PAIR(C_BORDER)|A_DIM); ty++;
        wattron(w,COLOR_PAIR(C_HDR)|A_BOLD); mvwprintw(w,ty++,2,"[ TAGS  (1-6) ]"); wattroff(w,COLOR_PAIR(C_HDR)|A_BOLD);
        for(int i=0;i<utag_n&&i<6&&ty<h-5;i++){
            int tc=tag_color(utags[i]);
            int active=(filter_tag[0]&&strcmp(utags[i],filter_tag)==0);
            if(active){
                wattron(w,COLOR_PAIR(C_SEL)|A_BOLD);
                mvwprintw(w,ty++,2," %d > #%-8.8s %3d",i+1,utags[i],utag_counts[i]);
                wattroff(w,COLOR_PAIR(C_SEL)|A_BOLD);
            } else {
                wattron(w,COLOR_PAIR(tc)|A_DIM);
                mvwprintw(w,ty++,2," %d   #%-8.8s %3d",i+1,utags[i],utag_counts[i]);
                wattroff(w,COLOR_PAIR(tc)|A_DIM);
            }
        }
        ty++;
    }

    /* selected task detail */
    if(vis_count>0&&sel<vis_count&&ty<h-3){
        Todo *t=&todos[vis[sel]];
        wattron(w,COLOR_PAIR(C_BORDER)|A_DIM); mvwhline(w,ty,2,ACS_HLINE,ca); wattroff(w,COLOR_PAIR(C_BORDER)|A_DIM); ty++;
        wattron(w,COLOR_PAIR(C_HDR)|A_BOLD); mvwprintw(w,ty++,2,"[ SELECTED ]"); wattroff(w,COLOR_PAIR(C_HDR)|A_BOLD);
        int tlen=(int)strlen(t->text);
        for(int s=0;s<tlen&&ty<h-3;){
            int end=s+ca-2; if(end>tlen)end=tlen;
            wattron(w,COLOR_PAIR(C_PEND)|A_BOLD);
            mvwprintw(w,ty++,3,"%.*s",end-s,t->text+s);
            wattroff(w,COLOR_PAIR(C_PEND)|A_BOLD);
            s=end;
        }
        if(t->pinned&&ty<h-3){
            wattron(w,COLOR_PAIR(C_PIN)|A_DIM); mvwprintw(w,ty++,3,"* pinned"); wattroff(w,COLOR_PAIR(C_PIN)|A_DIM);
        }
        char tsbuf[32];
        fmt_ts(t->created_at,tsbuf,(int)sizeof tsbuf);
        if(ty<h-2){wattron(w,A_DIM); mvwprintw(w,ty++,3,"+ %s",tsbuf); wattroff(w,A_DIM);}
        if(t->done_at&&ty<h-2){
            fmt_ts(t->done_at,tsbuf,(int)sizeof tsbuf);
            wattron(w,COLOR_PAIR(C_GREEN)|A_DIM);
            mvwprintw(w,ty++,3,"v %s",tsbuf);
            wattroff(w,COLOR_PAIR(C_GREEN)|A_DIM);
        }
    }

    /* mood pinned to bottom */
    {
        const char *mood=bongo_mood(g_pend);
        int mlen=(int)strlen(mood), mx=ww-mlen-1; if(mx<1)mx=1;
        wattron(w,COLOR_PAIR(C_BONGO)|A_BOLD);
        mvwprintw(w,h-1,mx,"%s",mood);
        wattroff(w,COLOR_PAIR(C_BONGO)|A_BOLD);
    }
    wnoutrefresh(w);
}

static int get_left_window_width(int cols){
    #define LEFT_WINDOW_MIN_WIDTH 20
    int lw=cols*58/100; if(lw<LEFT_WINDOW_MIN_WIDTH)lw=LEFT_WINDOW_MIN_WIDTH;
    return lw;
}

/* ------ window allocation helper ------ */
typedef struct {
    WINDOW *top, *left, *rite, *sbar;
} Windows;

static void free_windows(Windows *w){
    if(w->top)  { delwin(w->top);  w->top=NULL;  }
    if(w->left) { delwin(w->left); w->left=NULL; }
    if(w->rite) { delwin(w->rite); w->rite=NULL; }
    if(w->sbar) { delwin(w->sbar); w->sbar=NULL; }
}

static bool alloc_windows(Windows *w, int rows, int cols, int lw, int rw, int ch, Layout layout){
    free_windows(w);
    w->top  = newwin(1,   cols, 0,      0);
    w->sbar = newwin(1,   cols, rows-1, 0);
    if(layout == LAYOUT_FULL){
        w->left = newwin(ch, lw,   1,  0);
        w->rite = newwin(ch, rw,   1, lw);
    } else {
        /* compact: left pane takes full width */
        w->left = newwin(ch, cols, 1, 0);
        w->rite = NULL;
    }
    return (w->top && w->left && w->sbar);
}

/* ------ main ------ */
int main(void){
    setlocale(LC_ALL,"");
    g_utf8 = detect_utf8();
    BONGO   = g_utf8 ? BONGO_UTF8 : BONGO_ASCII;
    build_paths();
    initscr(); cbreak(); noecho(); curs_set(0);
    keypad(stdscr,TRUE);
    if(!has_colors()){endwin();puts("need color terminal");return 1;}
    start_color(); use_default_colors();

    init_pair(C_BORDER, COLOR_CYAN,    -1);
    init_pair(C_TOPBAR, COLOR_BLACK,   COLOR_MAGENTA);
    init_pair(C_SEL,    COLOR_BLACK,   COLOR_CYAN);
    init_pair(C_DONE,   COLOR_WHITE,   -1);
    init_pair(C_PEND,   COLOR_YELLOW,  -1);
    init_pair(C_BONGO,  COLOR_MAGENTA, -1);
    init_pair(C_SBAR,   COLOR_BLACK,   COLOR_BLUE);
    init_pair(C_HDR,    COLOR_CYAN,    -1);
    init_pair(C_GREEN,  COLOR_GREEN,   -1);
    init_pair(C_PIN,    COLOR_RED,     -1);
    init_pair(C_TAG1,   COLOR_GREEN,   -1);
    init_pair(C_TAG2,   COLOR_YELLOW,  -1);
    init_pair(C_TAG3,   COLOR_CYAN,    -1);
    init_pair(C_TAG4,   COLOR_MAGENTA, -1);
    init_pair(C_TAG5,   COLOR_RED,     -1);
    init_pair(C_TAG6,   COLOR_BLUE,    -1);
    init_pair(C_TAG7,   COLOR_WHITE,   -1);
    init_pair(C_TAG8,   COLOR_GREEN,   COLOR_BLUE);
    init_pair(C_SEARCH, COLOR_BLACK,   COLOR_YELLOW);
    init_pair(C_BEAT,   COLOR_BLACK,   COLOR_YELLOW);
    init_pair(C_TITLE,  COLOR_BLACK,   COLOR_CYAN);

    todos_load(); rebuild_vis(); rebuild_tag_cache();
    erase(); refresh();

    int rows, cols; getmaxyx(stdscr, rows, cols);
    Layout layout = get_layout(rows, cols);
    int lw = get_left_window_width(cols);
    int rw = cols - lw;
    int ch = rows - 2;

    Windows win = {NULL,NULL,NULL,NULL};
    if(layout != LAYOUT_TOOSMALL){
        alloc_windows(&win, rows, cols, lw, rw, ch, layout);
    }

    int list_top = 0;
    bool running = true;

    while(running){
        int nr, nc; getmaxyx(stdscr, nr, nc);
        Layout new_layout = get_layout(nr, nc);

        if(nr!=rows || nc!=cols || new_layout!=layout){
            rows=nr; cols=nc; layout=new_layout;
            lw = get_left_window_width(cols);
            rw = cols - lw;
            ch = rows - 2; if(ch < 1) ch = 1;
            free_windows(&win);
            erase(); refresh();
            if(layout != LAYOUT_TOOSMALL){
                alloc_windows(&win, rows, cols, lw, rw, ch, layout);
            }
        }

        if(layout == LAYOUT_TOOSMALL){
            draw_toosmall(rows, cols);
            timeout(200);
            int key = getch();
            if(key == 'q') running = false;
            continue;
        }

        if(tags_dirty) rebuild_tag_cache();
        recount();

        int pane_w = (layout == LAYOUT_FULL) ? lw : cols;
        int area = ch - LIST_Y0 - 1; if(area<1)area=1;
        if(sel<list_top)       list_top=sel;
        if(sel>=list_top+area) list_top=sel-area+1;
        if(list_top<0)         list_top=0;

        draw_top (win.top,  cols);
        draw_left(win.left, ch, pane_w, list_top);
        if(layout == LAYOUT_FULL && win.rite)
            draw_right(win.rite, ch, rw);
        draw_sbar(win.sbar, cols);

        if(celebrate>0){ draw_celebrate(rows,cols); celebrate--; }

        doupdate();

        timeout(celebrate>0 ? 150 : -1);
        if(smsg_ttl>0) smsg_ttl--;

        int key=getch();
        if(key==ERR) continue;
        smsg[0]='\0';

        switch(key){
        case 'q': running=false; break;

        case KEY_UP:   case 'k': if(sel>0)           sel--; break;
        case KEY_DOWN: case 'j': if(sel<vis_count-1) sel++; break;
        case 'g': case KEY_HOME: sel=0; break;
        case 'G': case KEY_END:  sel=vis_count>0?vis_count-1:0; break;
        case KEY_NPAGE: sel+=area/2; if(sel>=vis_count) sel=vis_count>0?vis_count-1:0; break;
        case KEY_PPAGE: sel-=area/2; if(sel<0) sel=0; break;

        case 'n':{
            char buf[MAX_LINE]="";
            if(popup("New task","what needs doing?",buf,(int)sizeof buf)&&todo_count<MAX_TODOS){
                Todo *t=&todos[todo_count];
                t->done=false;t->pinned=false;t->created_at=time(NULL);t->done_at=0;
                snprintf(t->tag,  MAX_TAG,  "%s", filter_tag[0]?filter_tag:"none");
                snprintf(t->text, MAX_LINE, "%s", buf);
                todo_count++; todos_save(); rebuild_vis(); sel=vis_count-1;
                set_smsg("task added! nya~");
            }
            break;
        }

        case 'e':{
            if(vis_count==0) break;
            Todo *t=&todos[vis[sel]];
            char buf[MAX_LINE];
            snprintf(buf, MAX_LINE, "%s", t->text);
            if(popup("Edit task","edit text  (Esc=cancel)",buf,(int)sizeof buf)){
                snprintf(t->text, MAX_LINE, "%s", buf);
                todos_save(); rebuild_vis(); set_smsg("task updated! nya~");
            }
            break;
        }

        case ' ':
            if(vis_count>0){
                Todo *t=&todos[vis[sel]];
                t->done=!t->done; t->done_at=t->done?time(NULL):0;
                todos_save(); rebuild_vis();
                bool all_done=true;
                for(int i=0;i<todo_count;i++) if(!todos[i].done){all_done=false;break;}
                if(all_done&&todo_count>0){celebrate=10;set_smsg("ALL DONE!! purrfect!! =^..^=");}
                else set_smsg(t->done?"done! good job :3":"back to pending~");
            }
            break;

        case 'p':
            if(vis_count>0){
                Todo *t=&todos[vis[sel]]; t->pinned=!t->pinned;
                todos_save(); rebuild_vis();
                set_smsg(t->pinned?"pinned! *":"unpinned~");
            }
            break;

        case 't':{
            if(vis_count==0) break;
            char hint[256]="enter tag (blank=clear)";
            if(utag_n>0){
                int hpos=(int)snprintf(hint,sizeof hint,"existing: ");
                for(int i=0;i<utag_n&&i<6&&hpos<(int)sizeof hint-2;i++)
                    hpos+=(int)snprintf(hint+hpos,(int)sizeof hint-hpos,"#%s ",utags[i]);
            }
            char buf[MAX_TAG]="";
            popup("Set tag",hint,buf,(int)sizeof buf);
            Todo *t=&todos[vis[sel]];
            if(buf[0]){
                char clean[MAX_TAG]=""; int ci=0;
                for(char *pp=buf;*pp&&ci<MAX_TAG-1;pp++)
                    if(!isspace((unsigned char)*pp)) clean[ci++]=(char)tolower((unsigned char)*pp);
                clean[ci]='\0';
                snprintf(t->tag, MAX_TAG, "%s", clean);
                set_smsg("tag set!");
            } else {snprintf(t->tag, MAX_TAG, "none");set_smsg("tag cleared");}
            todos_save(); rebuild_vis(); mark_dirty();
            break;
        }

        case 'd':{
            if(vis_count==0) break;
            char buf[8]="",pr[80];
            snprintf(pr,sizeof pr,"delete \"%.40s\"? y/N",todos[vis[sel]].text);
            popup(pr,"",buf,(int)sizeof buf);
            if(buf[0]=='y'||buf[0]=='Y'){
                int ti=vis[sel];
                for(int i=ti;i<todo_count-1;i++) todos[i]=todos[i+1];
                todo_count--; if(sel>0&&sel>=vis_count-1) sel--;
                todos_save(); rebuild_vis(); set_smsg("deleted! poof~");
            }
            break;
        }

        case 'D':{
            if(todo_count==0) break;
            char buf[8]="",pr[80];
            snprintf(pr,sizeof pr,"delete ALL %d tasks? y/N",todo_count);
            popup(pr,"!! THIS CANNOT BE UNDONE !!",buf,(int)sizeof buf);
            if(buf[0]=='y'||buf[0]=='Y'){
                todo_count=0; sel=0; list_top=0;
                todos_save(); rebuild_vis(); mark_dirty();
                set_smsg("clean slate! fresh start nya~");
            }
            break;
        }

        case '/':{
            char buf[MAX_LINE]="";
            if(popup("Search","type to filter~",buf,(int)sizeof buf))
                snprintf(search,sizeof search,"%s",buf);
            else search[0]='\0';
            sel=0; list_top=0; rebuild_vis();
            break;
        }

        case 27:
            search[0]='\0'; filter_tag[0]='\0';
            sel=0; list_top=0; rebuild_vis(); set_smsg("filters cleared~");
            break;

        case '0':
            filter_tag[0]='\0'; sel=0; list_top=0;
            rebuild_vis(); set_smsg("showing all tasks");
            break;

        case '1':case '2':case '3':case '4':case '5':case '6':{
            int ti=key-'1';
            if(ti<utag_n){
                strncpy(filter_tag,utags[ti],MAX_TAG-1); filter_tag[MAX_TAG-1]='\0';
                sel=0; list_top=0; rebuild_vis();
                char m[64]; snprintf(m,sizeof m,"filter: #%s",filter_tag); set_smsg(m);
            }
            break;
        }

        case KEY_RESIZE: break;
        default: break;
        }
    }

    free(todo_path);
    free_windows(&win);
    endwin();
    printf("bye bye~ =^..^=\n");
    return 0;
}
