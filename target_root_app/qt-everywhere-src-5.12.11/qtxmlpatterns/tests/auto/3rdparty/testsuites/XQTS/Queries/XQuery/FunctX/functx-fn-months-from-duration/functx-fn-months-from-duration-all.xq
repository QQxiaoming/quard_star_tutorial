(:**************************************************************:)
(: Test: functx-fn-months-from-duration-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(months-from-duration(
  xs:yearMonthDuration('P3M')), months-from-duration(
  xs:yearMonthDuration('-P18M')), months-from-duration(
  xs:yearMonthDuration('P1Y')), months-from-duration(
  xs:yearMonthDuration('P12M')))