(:**************************************************************:)
(: Test: functx-fn-adjust-date-to-timezone-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(adjust-date-to-timezone(
   xs:date('2006-02-15'),
   xs:dayTimeDuration('-PT8H')), adjust-date-to-timezone(
   xs:date('2006-02-15-03:00'),
   xs:dayTimeDuration('-PT8H')), adjust-date-to-timezone(
   xs:date('2006-02-15'), ()), adjust-date-to-timezone(
   xs:date('2006-02-15-03:00'), ()))