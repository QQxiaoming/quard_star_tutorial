(:**************************************************************:)
(: Test: functx-fn-months-from-duration-2                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(months-from-duration(
  xs:yearMonthDuration('-P18M')))
