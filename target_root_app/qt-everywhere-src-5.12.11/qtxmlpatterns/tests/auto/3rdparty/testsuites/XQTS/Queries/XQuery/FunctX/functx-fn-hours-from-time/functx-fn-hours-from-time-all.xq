(:**************************************************************:)
(: Test: functx-fn-hours-from-time-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(hours-from-time(
   xs:time('10:30:23')), hours-from-time(
   xs:time('10:30:23-05:00')))