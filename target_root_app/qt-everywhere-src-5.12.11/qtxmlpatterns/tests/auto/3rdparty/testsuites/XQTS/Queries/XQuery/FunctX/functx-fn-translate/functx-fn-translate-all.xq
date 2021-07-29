(:**************************************************************:)
(: Test: functx-fn-translate-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(translate('1999/01/02', '/', '-'), translate('xml query', 'qlmx', 'QLMX'), translate('xml query', 'qlmx ', 'Q'), translate('xml query', 'qlmx ', ''), translate('xml query', 'abcd', 'ABCD'), translate('', 'qlmx ', 'Q'), translate((), 'qlmx ', 'Q'))