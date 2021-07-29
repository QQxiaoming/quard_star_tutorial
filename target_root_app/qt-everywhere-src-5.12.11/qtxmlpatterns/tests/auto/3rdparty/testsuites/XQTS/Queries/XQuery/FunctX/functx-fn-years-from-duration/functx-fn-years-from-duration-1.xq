(:**************************************************************:)
(: Test: functx-fn-years-from-duration-1                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(years-from-duration(
   xs:yearMonthDuration('P3Y')))
