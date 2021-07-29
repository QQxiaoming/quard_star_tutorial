(:**************************************************************:)
(: Test: functx-fn-dateTime-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(dateTime(xs:date('2006-08-15'),
            xs:time('12:30:45-05:00')))