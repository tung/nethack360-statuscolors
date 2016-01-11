/* NetHack 3.6	botl.c	$NHDT-Date: 1447978683 2015/11/20 00:18:03 $  $NHDT-Branch: master $:$NHDT-Revision: 1.69 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include <limits.h>

extern const char *hu_stat[]; /* defined in eat.c */

const char *const enc_stat[] = { "",         "Burdened",  "Stressed",
                                 "Strained", "Overtaxed", "Overloaded" };

STATIC_OVL NEARDATA int mrank_sz = 0; /* loaded by max_rank_sz (from u_init) */
STATIC_DCL const char *NDECL(rank);

#ifndef STATUS_VIA_WINDOWPORT

STATIC_DCL void NDECL(bot1);
STATIC_DCL void NDECL(bot2);

STATIC_OVL void
bot1()
{
    char newbot1[MAXCO];
    register char *nb;
    register int i, j;

    Strcpy(newbot1, plname);
    if ('a' <= newbot1[0] && newbot1[0] <= 'z')
        newbot1[0] += 'A' - 'a';
    newbot1[10] = 0;
    Sprintf(nb = eos(newbot1), " the ");

    if (Upolyd) {
        char mbot[BUFSZ];
        int k = 0;

        Strcpy(mbot, mons[u.umonnum].mname);
        while (mbot[k] != 0) {
            if ((k == 0 || (k > 0 && mbot[k - 1] == ' ')) && 'a' <= mbot[k]
                && mbot[k] <= 'z')
                mbot[k] += 'A' - 'a';
            k++;
        }
        Strcpy(nb = eos(nb), mbot);
    } else
        Strcpy(nb = eos(nb), rank());

    Sprintf(nb = eos(nb), "  ");
    i = mrank_sz + 15;
    j = (int) ((nb + 2) - newbot1); /* strlen(newbot1) but less computation */
    if ((i - j) > 0)
        Sprintf(nb = eos(nb), "%*s", i - j, " "); /* pad with spaces */
    if (ACURR(A_STR) > 18) {
        if (ACURR(A_STR) > STR18(100))
            Sprintf(nb = eos(nb), "St:%2d ", ACURR(A_STR) - 100);
        else if (ACURR(A_STR) < STR18(100))
            Sprintf(nb = eos(nb), "St:18/%02d ", ACURR(A_STR) - 18);
        else
            Sprintf(nb = eos(nb), "St:18/** ");
    } else
        Sprintf(nb = eos(nb), "St:%-1d ", ACURR(A_STR));
    Sprintf(nb = eos(nb), "Dx:%-1d Co:%-1d In:%-1d Wi:%-1d Ch:%-1d",
            ACURR(A_DEX), ACURR(A_CON), ACURR(A_INT), ACURR(A_WIS),
            ACURR(A_CHA));
    Sprintf(nb = eos(nb),
            (u.ualign.type == A_CHAOTIC)
                ? "  Chaotic"
                : (u.ualign.type == A_NEUTRAL) ? "  Neutral" : "  Lawful");
#ifdef SCORE_ON_BOTL
    if (flags.showscore)
        Sprintf(nb = eos(nb), " S:%ld", botl_score());
#endif
    curs(WIN_STATUS, 1, 0);
    putstr(WIN_STATUS, 0, newbot1);
}

STATIC_OVL void
bot2()
{
    char newbot2[MAXCO];
    register char *nb;
    int hp, hpmax;
    int cap = near_capacity();

    hp = Upolyd ? u.mh : u.uhp;
    hpmax = Upolyd ? u.mhmax : u.uhpmax;

    if (hp < 0)
        hp = 0;
    (void) describe_level(newbot2);
    Sprintf(nb = eos(newbot2), "%s:%-2ld HP:%d(%d) Pw:%d(%d) AC:%-2d",
            encglyph(objnum_to_glyph(GOLD_PIECE)), money_cnt(invent), hp,
            hpmax, u.uen, u.uenmax, u.uac);

    if (Upolyd)
        Sprintf(nb = eos(nb), " HD:%d", mons[u.umonnum].mlevel);
    else if (flags.showexp)
        Sprintf(nb = eos(nb), " Xp:%u/%-1ld", u.ulevel, u.uexp);
    else
        Sprintf(nb = eos(nb), " Exp:%u", u.ulevel);

    if (flags.time)
        Sprintf(nb = eos(nb), " T:%ld", moves);
    if (strcmp(hu_stat[u.uhs], "        ")) {
        Sprintf(nb = eos(nb), " ");
        Strcat(newbot2, hu_stat[u.uhs]);
    }
    if (Confusion)
        Sprintf(nb = eos(nb), " Conf");
    if (Sick) {
        if (u.usick_type & SICK_VOMITABLE)
            Sprintf(nb = eos(nb), " FoodPois");
        if (u.usick_type & SICK_NONVOMITABLE)
            Sprintf(nb = eos(nb), " Ill");
    }
    if (Blind)
        Sprintf(nb = eos(nb), " Blind");
    if (Stunned)
        Sprintf(nb = eos(nb), " Stun");
    if (Hallucination)
        Sprintf(nb = eos(nb), " Hallu");
    if (Slimed)
        Sprintf(nb = eos(nb), " Slime");
    if (cap > UNENCUMBERED)
        Sprintf(nb = eos(nb), " %s", enc_stat[cap]);
    curs(WIN_STATUS, 1, 1);
    putmixed(WIN_STATUS, 0, newbot2);
}

void
bot()
{
    if (youmonst.data) {
        bot1();
        bot2();
    }
    context.botl = context.botlx = 0;
}

#endif /* !STATUS_VIA_WINDOWPORT */

/* convert experience level (1..30) to rank index (0..8) */
int
xlev_to_rank(xlev)
int xlev;
{
    return (xlev <= 2) ? 0 : (xlev <= 30) ? ((xlev + 2) / 4) : 8;
}

#if 0 /* not currently needed */
/* convert rank index (0..8) to experience level (1..30) */
int
rank_to_xlev(rank)
int rank;
{
    return (rank <= 0) ? 1 : (rank <= 8) ? ((rank * 4) - 2) : 30;
}
#endif

const char *
rank_of(lev, monnum, female)
int lev;
short monnum;
boolean female;
{
    register const struct Role *role;
    register int i;

    /* Find the role */
    for (role = roles; role->name.m; role++)
        if (monnum == role->malenum || monnum == role->femalenum)
            break;
    if (!role->name.m)
        role = &urole;

    /* Find the rank */
    for (i = xlev_to_rank((int) lev); i >= 0; i--) {
        if (female && role->rank[i].f)
            return role->rank[i].f;
        if (role->rank[i].m)
            return role->rank[i].m;
    }

    /* Try the role name, instead */
    if (female && role->name.f)
        return role->name.f;
    else if (role->name.m)
        return role->name.m;
    return "Player";
}

STATIC_OVL const char *
rank()
{
    return rank_of(u.ulevel, Role_switch, flags.female);
}

int
title_to_mon(str, rank_indx, title_length)
const char *str;
int *rank_indx, *title_length;
{
    register int i, j;

    /* Loop through each of the roles */
    for (i = 0; roles[i].name.m; i++)
        for (j = 0; j < 9; j++) {
            if (roles[i].rank[j].m
                && !strncmpi(str, roles[i].rank[j].m,
                             strlen(roles[i].rank[j].m))) {
                if (rank_indx)
                    *rank_indx = j;
                if (title_length)
                    *title_length = strlen(roles[i].rank[j].m);
                return roles[i].malenum;
            }
            if (roles[i].rank[j].f
                && !strncmpi(str, roles[i].rank[j].f,
                             strlen(roles[i].rank[j].f))) {
                if (rank_indx)
                    *rank_indx = j;
                if (title_length)
                    *title_length = strlen(roles[i].rank[j].f);
                return (roles[i].femalenum != NON_PM) ? roles[i].femalenum
                                                      : roles[i].malenum;
            }
        }
    return NON_PM;
}

void
max_rank_sz()
{
    register int i, r, maxr = 0;
    for (i = 0; i < 9; i++) {
        if (urole.rank[i].m && (r = strlen(urole.rank[i].m)) > maxr)
            maxr = r;
        if (urole.rank[i].f && (r = strlen(urole.rank[i].f)) > maxr)
            maxr = r;
    }
    mrank_sz = maxr;
    return;
}

#ifdef SCORE_ON_BOTL
long
botl_score()
{
    long deepest = deepest_lev_reached(FALSE);
    long utotal;

    utotal = money_cnt(invent) + hidden_gold();
    if ((utotal -= u.umoney0) < 0L)
        utotal = 0L;
    utotal += u.urexp + (50 * (deepest - 1))
          + (deepest > 30 ? 10000 : deepest > 20 ? 1000 * (deepest - 20) : 0);
    if (utotal < u.urexp)
        utotal = LONG_MAX; /* wrap around */
    return utotal;
}
#endif /* SCORE_ON_BOTL */

/* provide the name of the current level for display by various ports */
int
describe_level(buf)
char *buf;
{
    int ret = 1;

    /* TODO:    Add in dungeon name */
    if (Is_knox(&u.uz))
        Sprintf(buf, "%s ", dungeons[u.uz.dnum].dname);
    else if (In_quest(&u.uz))
        Sprintf(buf, "Home %d ", dunlev(&u.uz));
    else if (In_endgame(&u.uz))
        Sprintf(buf, Is_astralevel(&u.uz) ? "Astral Plane " : "End Game ");
    else {
        /* ports with more room may expand this one */
        Sprintf(buf, "Dlvl:%-2d ", depth(&u.uz));
        ret = 0;
    }
    return ret;
}

#ifdef STATUS_VIA_WINDOWPORT
/* =======================================================================*/

STATIC_DCL void FDECL(status_info_add_cond,
                      (struct status_info *, int *, const char *));
STATIC_DCL void FDECL(populate_status_info, (struct status_info *));

static boolean blinit = FALSE;

STATIC_OVL void
status_info_add_cond(si, i, str)
struct status_info *si;
int *i;
const char *str;
{
    if (*i > sizeof(si->conds) / sizeof(si->conds[0]))
        panic("status_info_add_cond: condition limit exceeded (%d)", *i);
    strncpy(si->conds[*i], str, sizeof(si->conds[0]));
    si->conds[*i][sizeof(si->conds[0]) - 1] = '\0';
    *i += 1;
}

STATIC_OVL void
populate_status_info(si)
struct status_info *si;
{
    char buf[BUFSZ];
    int i;

    /*
     *  Player name and title.
     */
    strncpy(si->name, plname, sizeof(si->name));
    si->name[sizeof(si->name) - 1] = '\0';
    if ('a' <= si->name[0] && si->name[0] <= 'z')
        si->name[0] += 'A' - 'a';

    if (Upolyd) {
        char mbot[BUFSZ];
        int k = 0;
        Strcpy(mbot, mons[u.umonnum].mname);
        while (mbot[k] != 0) {
            if ((k == 0 || (k > 0 && mbot[k - 1] == ' ')) && 'a' <= mbot[k]
                && mbot[k] <= 'z')
                mbot[k] += 'A' - 'a';
            k++;
        }
        strncpy(si->title, mbot, sizeof(si->title));
    } else {
        strncpy(si->title, rank(), sizeof(si->title));
    }
    si->title[sizeof(si->title) - 1] = '\0';

    /* Strength */
    if (ACURR(A_STR) > STR18(100)) {
        si->st = ACURR(A_STR) - 100;
        si->st_extra = 0;
    } else if (ACURR(A_STR) > 18 && ACURR(A_STR) <= STR18(100)) {
        si->st = 18;
        si->st_extra = ACURR(A_STR) - 18;
    } else {
        si->st = ACURR(A_STR);
        si->st_extra = 0;
    }

    /*  Dexterity, constitution, intelligence, wisdom, charisma. */
    si->dx = ACURR(A_DEX);
    si->co = ACURR(A_CON);
    si->in = ACURR(A_INT);
    si->wi = ACURR(A_WIS);
    si->ch = ACURR(A_CHA);

    /* Alignment */
    if (u.ualign.type == A_CHAOTIC) {
        Strcpy(si->align, "Chaotic");
    } else if (u.ualign.type == A_NEUTRAL) {
        Strcpy(si->align, "Neutral");
    } else {
        Strcpy(si->align, "Lawful");
    }

    /* Score */
#ifdef SCORE_ON_BOTL
    si->score = botl_score();
#else
    si->score = 0;
#endif

    /*  Dungeon level. */
    (void) describe_level(buf);
    strncpy(si->dlvl, buf, sizeof(si->dlvl));
    si->dlvl[sizeof(si->dlvl) - 1] = '\0';

    /* Gold */
    strncpy(si->gold_sym, encglyph(objnum_to_glyph(GOLD_PIECE)),
            sizeof(si->gold_sym));
    si->gold_sym[sizeof(si->gold_sym) - 1] = '\0';
    si->gold = money_cnt(invent);

    /*  Hit points  */
    if (Upolyd) {
        si->hp = u.mh;
        si->hp_max = u.mhmax;
    } else {
        si->hp = u.uhp;
        si->hp_max = u.uhpmax;
    }

    /* Power (magical energy) */
    si->pw = u.uen;
    si->pw_max = u.uenmax;

    /* Armor class */
    si->ac = u.uac;

    /* Experience */
    if (Upolyd) {
        Strcpy(si->exp_label, "HD");
        si->exp_level = mons[u.umonnum].mlevel;
        si->exp_points = 0;
    } else {
        Strcpy(si->exp_label, "Xp");
        si->exp_level = u.ulevel;
        si->exp_points = u.uexp;
    }

    /* Time (moves) */
    si->turns = moves;

    /* Conditions */
    (void) memset(si->conds, 0, sizeof(si->conds));
    i = 0;
    if (strcmp(hu_stat[u.uhs], "        "))
        status_info_add_cond(si, &i, hu_stat[u.uhs]);
    if (Confusion)
        status_info_add_cond(si, &i, "Conf");
    if (Sick && (u.usick_type & SICK_VOMITABLE))
        status_info_add_cond(si, &i, "FoodPois");
    if (Sick && (u.usick_type & SICK_NONVOMITABLE))
        status_info_add_cond(si, &i, "Ill");
    if (Blind)
        status_info_add_cond(si, &i, "Blind");
    if (Stunned)
        status_info_add_cond(si, &i, "Stun");
    if (Hallucination)
        status_info_add_cond(si, &i, "Hallu");
    if (Slimed)
        status_info_add_cond(si, &i, "Slime");
    if (near_capacity() > UNENCUMBERED)
        status_info_add_cond(si, &i, enc_stat[near_capacity()]);

#ifdef SCORE_ON_BOTL
    si->show_score = flags.showscore;
#else
    si->show_score = FALSE;
#endif
    si->show_exp_points = !Upolyd && flags.showexp;
    si->show_turns = flags.time;
}

void
bot()
{
    struct status_info si;

    if (!blinit)
        panic("bot before init.");
    if (!youmonst.data) {
        context.botl = context.botlx = 0;
        return;
    }

    populate_status_info(&si);
    status_update(&si);

    context.botl = context.botlx = 0;
}

void
status_initialize()
{
    (*windowprocs.win_status_init)();
    blinit = TRUE;
}

void
status_finish()
{
    /* call the window port cleanup routine */
    (*windowprocs.win_status_finish)();
}

#endif /*STATUS_VIA_WINDOWPORT*/

/*botl.c*/
