(:**************************************************************:)
(: Test: functx-fn-month-from-dateTime-1                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(month-from-dateTime(
  xs:dateTime('2006-08-15T10:30:23')))
