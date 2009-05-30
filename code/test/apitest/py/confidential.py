#!/usr/bin/env python

# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

import os
import jagpdf
import jag.testlib as testlib
from jag.table import SimpleTable, DefaultTableStyle

class Style(DefaultTableStyle):
    def header_row_bg(self, canvas, *rect):
        canvas.move_to(rect[0], rect[1] + rect[3])
        canvas.line_to(rect[0] + rect[2], rect[1] + rect[3])
        canvas.move_to(rect[0], rect[1])
        canvas.line_to(rect[0] + rect[2], rect[1])
        canvas.path_paint('s')

    def footer_row_bg(self, canvas, *rect):
        canvas.move_to(rect[0], rect[1] + rect[3])
        canvas.line_to(rect[0] + rect[2], rect[1] + rect[3])
        canvas.path_paint('s')


def test_main(argv=None):
    profile = jagpdf.create_profile()
    profile.set('doc.encryption', 'standard')
    profile.set("info.static_producer", "1")
    profile.set("doc.static_file_id", "1")
    profile.set("info.creation_date", "0")
    profile.set('stdsh.pwd_user', 'user')
    profile.set('stdsh.pwd_owner', 'owner')
    profile.set("stdsh.permissions", "no_print; no_copy")
    doc = testlib.create_test_doc(argv, 'topsecret.pdf', profile)
    fnt = doc.font_load("standard; name=Helvetica; size=10; enc=windows-1252")
    fnt_b = doc.font_load("standard; name=Helvetica-Bold; size=10; enc=windows-1252")
    doc.page_start(597.6, 848.68)
    canvas = doc.page().canvas()
    canvas.color_space("fs", jagpdf.CS_DEVICE_RGB)
    # confidential
    canvas.state_save()
    cx, cy = 597.6/2, 848.68/2
    cfnt = doc.font_load("standard; name=Helvetica-Bold; size=120; enc=windows-1252")
    canvas.text_font(cfnt)
    cf_w = cfnt.advance("TOP SECRET")
    cf_x = (597.6 - cf_w) / 2
    canvas.translate(cx, cy)
    canvas.rotate(3.14/3)
    canvas.translate(-cx, -cy)
    #canvas.text_rendering_mode('s')
    canvas.color('f', 0.8, 0.8, 0.8)
    canvas.text(cf_x, cy - 20, "TOP SECRET")
    canvas.state_restore()
    # page header
    fnt_clients = doc.font_load("standard; name=Helvetica-Bold; size=13; enc=windows-1252")
    top_str = "Top 50 Clients"
    top_w = fnt_clients.advance(top_str)
    top_x = (597.6 - top_w) / 2
    canvas.text_font(fnt_clients)
    canvas.text(top_x, 785, top_str)
    # table
    widths = [150, 100, 100, 120]
    tbl = SimpleTable(doc, widths, font=fnt, header=1, footer=1, style=Style())
    # table header
    tbl.add_line(('Name', 'Account Nr.', 'Opened', 'Balance'))
    tbl.set_line_prop("font", 0, fnt_b)
    tbl.set_line_prop("halign", 0, "center")
    tbl.set_line_prop("cell_pad", 0, {'top': 4, 'bottom': 4})
    # table body
    for rec in records.split('\n')[:50]:
        tbl.add_line(rec.split('|'))
    tbl.set_column_prop("cell_pad", 0, {'left': 15})
    tbl.set_column_prop("cell_pad", 3, {'right': 15})
    tbl.set_column_prop("halign", 1, "center")
    tbl.set_column_prop("halign", 2, "center")
    tbl.set_column_prop("halign", 3, "right")
    # table footer
    tbl.add_line(4*[])
    # table render
    tbl_width = sum(widths)
    tbl_x = (597.6 - tbl_width) / 2
    tbl.render(tbl_x, 780, canvas)
    # page footer
    fnt_clients = doc.font_load("standard; name=Helvetica-Oblique; size=10; enc=windows-1252")
    canvas.text_font(fnt_clients)
    canvas.text(tbl_x, 40, "Classification: TOP SECRET")
    testlib.paint_image('/images/GWBush-signature.png', doc, tbl_x + 340, 10)
    # stamp
    testlib.paint_image('/images/topsecret.png', doc, 450, 20)
    #
    testlib.paint_image('/images/greedybank.png', doc, tbl_x, 790)
    #
    doc.page_end()
    doc.finalize()


def generate_records(num):
    import random
    import datetime
    import locale
    from jag.data import names
    balance = [10000000 * random.random() + 900000 for i in xrange(num)]
    balance.sort(lambda l, r: -cmp(l, r))
    locale.setlocale(locale.LC_ALL, '')
    for i in xrange(num):
        rand_date = random.randint(1980, 2008), \
                    random.randint(1, 12), \
                    random.randint(1, 28)
        print "%s|%d|%s|%s" % (names[i], \
                               random.randint(1000000000, 9999999999), \
                               datetime.date(*rand_date).isoformat(), \
                               locale.currency(balance[i], grouping=True))


records="""Steven Eden|1571604031|2002-01-06|$10,769,811.37
Madelyn E. Vinci|7164849703|2003-10-13|$10,732,504.13
Corliss S. Widener|8407604959|2004-08-06|$10,715,181.11
Terresa Dumond|7934773315|1997-02-05|$10,652,082.88
Lean C. Willison|8511145032|2008-06-14|$10,332,065.34
Man King|5548628277|2004-08-02|$10,139,231.34
Bettina M. Haslam|4954537621|1999-04-08|$10,097,366.58
Angele V. Heidelberg|5853391256|1987-11-05|$10,002,460.74
James E. Satter|9752899788|2005-03-22|$9,881,437.11
Maryjane Kunkel|1299456951|2004-01-18|$9,802,320.47
Chara Y. Cyr|5045651012|2005-04-25|$9,750,234.99
Suzanne S. Perrin|6933698775|1987-05-25|$9,741,455.62
Emilia V. Noda|8859784524|2006-11-19|$9,706,346.76
Aliza R. Charpentier|5040508442|1981-04-19|$9,673,722.49
Neva Guida|3289447837|1999-04-06|$9,585,853.91
Mariette L. Mcvay|8995223026|1989-02-07|$9,400,374.67
Cristine Heid|9099333057|1997-04-07|$9,353,322.06
Daisy Weidman|5835885345|1980-06-16|$9,348,812.10
Tilda W. Trosclair|1638669669|1992-06-02|$9,294,094.51
Jaunita Mathes|2942758393|1998-07-20|$8,848,322.51
Janey Tidwell|2220539687|2005-02-16|$8,706,317.98
Verline Migliore|3000380695|1991-06-16|$8,558,068.41
Etta K. Staley|5365016124|1982-01-07|$8,452,119.61
Yvone Pinedo|8219651209|1980-03-11|$8,233,441.78
Alexander Gerner|3766811887|2003-07-13|$8,210,518.06
Phung X. Vandenbosch|4269555729|1998-09-01|$7,993,574.38
Karlene Sandusky|8282035337|1980-10-10|$7,968,083.04
Gertrudis H. Newell|6053606049|2002-08-20|$7,929,859.16
Emelia Mcninch|5414637504|1996-01-19|$7,850,591.09
Khadijah M. Doyal|6017724509|1985-12-02|$7,726,615.57
Hollie Donahoe|6101557389|2000-03-07|$7,601,061.18
Debera M. Schley|2205716280|2007-07-27|$7,500,608.17
Afton T. Blind|3005267897|1999-02-23|$7,374,560.93
Fatimah K. Masson|5969141641|1990-04-10|$7,296,872.51
Narcisa K. Pollitt|4731479793|1997-06-04|$7,181,273.05
Leida C. Portillo|6292972684|1988-08-28|$7,028,450.34
Adena Carranza|5972394077|1991-04-15|$6,936,467.91
Angeline T. Honore|1916499337|1993-08-22|$6,906,022.55
Dominga H. Clyburn|5656560121|2000-12-03|$6,853,403.94
Tinisha L. Krol|2781019046|1986-06-12|$6,748,865.92
Alene A. Antonelli|9451774748|2000-05-07|$6,618,513.84
Jodee S. Yeomans|4031070253|1986-03-25|$6,538,653.81
Eveline G. Hedley|1346976083|2000-01-26|$6,536,286.92
Sherika Mcgeorge|1127142763|2000-08-11|$6,476,575.64
Wei L. Mclaren|8259270332|2004-02-13|$6,470,881.31
Aracelis M. Beets|6023207055|1992-01-27|$6,356,506.16
Mina Nicholes|9869260372|1998-11-08|$6,298,674.54
Stephaine D. Fishback|4954920270|1989-07-08|$6,204,640.72
Kathie E. Otten|5548769244|2008-10-17|$5,764,087.16
Arla Cannon|9245122278|1991-12-16|$5,725,118.28
Janette Bolling|9906631018|1990-02-21|$5,689,422.18
Nichelle G. Cronan|6502462490|1987-11-18|$5,632,009.00
Lucina T. Severe|9885153983|1988-12-13|$5,565,129.92
Nancy M. Moorman|9693544615|1998-10-03|$5,551,356.54
Mandie Macdougall|6842197777|1983-02-21|$5,517,030.34
Roxana Bancroft|8646013032|1992-10-10|$5,186,061.57
Livia Wahl|6517657387|2000-03-28|$4,830,899.08
Toshiko S. Fujiwara|2773285039|1995-12-08|$4,710,413.09
Delinda J. Cascio|5188475978|1989-01-20|$4,624,258.97
Margit L. Shore|8101034552|1982-08-16|$4,582,721.63
China Mccurdy|8099652817|1995-06-07|$4,549,320.21
Cris Wolfgang|8994084219|1991-01-15|$4,342,121.39
Eartha A. Dorrance|1008502113|1987-03-12|$4,298,326.93
Thu Florence|7289540984|1990-09-27|$4,289,687.54
Nguyet J. Marchetti|9814231254|1980-09-23|$4,168,101.21
Julianne D. Shadwick|6358118210|1999-02-21|$4,161,494.54
Margherita R. Nimmons|3059795691|1983-10-21|$4,152,706.81
Valorie Benevides|2605873784|1985-10-20|$4,152,672.30
Scott C. Carolina|6608313525|1994-12-26|$4,124,852.39
Ghislaine Johannes|1904139378|1998-12-25|$4,041,122.51
Many R. Bosserman|6147877077|2000-06-18|$3,957,263.04
Jeanice S. Arevalo|8372145041|2003-02-04|$3,836,461.58
Lashaun A. Pritts|5704997947|1987-01-19|$3,790,610.26
Leon M. Limon|1041823558|2007-07-12|$3,218,629.30
Perry Nicklas|9857993917|2006-07-08|$3,150,914.69
Harmony K. Levenson|8850828335|1998-06-18|$3,076,514.99
Wendy Crosland|5126541484|1984-07-03|$3,020,518.98
Luisa P. Lombardi|8782447452|1996-11-04|$2,952,955.60
Roxie Quesenberry|3956784116|1999-11-09|$2,917,051.13
Stephen M. Stell|6985533015|2008-10-14|$2,829,928.94
Cristie A. Gries|9493092726|1997-10-09|$2,742,564.75
Luanne P. Foshee|7859897469|2002-07-05|$2,727,531.59
Jen N. Okelley|8185698234|1998-11-23|$2,723,119.97
Lily P. Merryman|2761958682|1983-02-10|$2,664,643.94
Wynell L. Hamrick|7535226447|2002-03-23|$2,536,328.46
Harriette Greenhill|8199017717|1999-02-06|$2,338,726.01
Evalyn Welter|5148662717|1997-07-13|$2,170,941.02
Indira K. Goguen|2877672488|1990-08-27|$2,062,338.83
Epifania Grabowski|8483560295|2000-12-07|$1,933,208.97
Lou Espino|3572136167|1988-07-18|$1,921,647.56
Nickie P. Guinn|9951518624|1982-06-16|$1,788,835.95
Farrah M. Henthorn|7765321027|2000-11-26|$1,721,666.23
Mathilde Fraley|5890452079|1981-03-17|$1,677,226.79
Ling P. Duren|2765148475|2008-03-15|$1,633,481.69
Fern E. Cloninger|9852000478|2002-11-14|$1,626,637.30
Tony Goudeau|9099590056|1989-07-03|$1,552,398.32
Jana Luker|9396914887|1997-08-18|$1,499,686.77
Hue M. Bass|7086118935|2002-07-09|$1,485,046.48
Kit Specht|3823254191|2004-12-27|$1,098,792.97
Carmon Burgoon|6479898045|1994-01-06|$1,075,875.58"""


if __name__ == '__main__':
    #generate_records(100)
    test_main()

