(:**************************************************************:)
(: Test: functx-fn-hours-from-dateTime-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(hours-from-dateTime(
   xs:dateTime('2006-08-15T10:30:23')), hours-from-dateTime(
   xs:dateTime('2006-08-15T10:30:23-05:00')))