(:**************************************************************:)
(: Test: functx-fn-adjust-dateTime-to-timezone-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(adjust-dateTime-to-timezone(
   xs:dateTime('2006-02-15T17:00:00'),
   xs:dayTimeDuration('-PT7H')), adjust-dateTime-to-timezone(
   xs:dateTime('2006-02-15T17:00:00-03:00'),
   xs:dayTimeDuration('-PT7H')), adjust-dateTime-to-timezone(
   xs:dateTime('2006-02-15T17:00:00'),
   ()), adjust-dateTime-to-timezone(
   xs:dateTime('2006-02-15T17:00:00-03:00'),
   ()), adjust-dateTime-to-timezone(
   xs:dateTime('2006-02-15T01:00:00-03:00'),
   xs:dayTimeDuration('-PT7H')))