(:**************************************************************:)
(: Test: functx-fn-abs-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(abs(3.5), abs(-4), abs(xs:float('-INF')))