import locale
import time

locales = ['en_US', 'fr_FR', 'de_DE', 'es_ES', 'it_IT', 'nl_NL', 'pt_BR', 'tr_TR', 'cs_CZ', 'el_GR', 'sv_SE','pl_PL','sk_SK', 'ro_RO', 'ca_ES', 'no_NO'];

timeData = list(time.localtime());

for l in locales:
    print(l)
    locale.setlocale(locale.LC_ALL, l)

    days = u'{';
    for i in [6, 0, 1, 2, 3, 4, 5]:
        timeData[6] = i;
        days += time.strftime('"%a", ', tuple(timeData)).upper();
    days += u'}';
    print(days);

    months = u'{';
    for i in range(1, 13):
        timeData[1] = i;
        months += time.strftime('"%b", ', tuple(timeData)).upper();
    months += u'}';
    print(months);
