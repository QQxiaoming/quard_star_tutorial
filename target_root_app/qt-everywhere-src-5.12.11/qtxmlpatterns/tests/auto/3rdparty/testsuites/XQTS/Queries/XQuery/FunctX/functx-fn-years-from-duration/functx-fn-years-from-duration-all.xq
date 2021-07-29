(:**************************************************************:)
(: Test: functx-fn-years-from-duration-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(years-from-duration(
   xs:yearMonthDuration('P3Y')), years-from-duration(
   xs:yearMonthDuration('P3Y11M')), years-from-duration(
   xs:yearMonthDuration('-P18M')), years-from-duration(
   xs:yearMonthDuration('P1Y18M')), years-from-duration(
   xs:yearMonthDuration('P12M')))