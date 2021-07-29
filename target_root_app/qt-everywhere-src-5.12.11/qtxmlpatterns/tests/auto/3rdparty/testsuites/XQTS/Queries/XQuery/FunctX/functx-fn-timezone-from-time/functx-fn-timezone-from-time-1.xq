(:**************************************************************:)
(: Test: functx-fn-timezone-from-time-1                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(timezone-from-time(
   xs:time('09:54:00-05:00')))
