(:**************************************************************:)
(: Test: functx-fn-timezone-from-date-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(timezone-from-date(
   xs:date('2006-08-15-05:00')), timezone-from-date(
   xs:date('2006-08-15')))