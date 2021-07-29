(:**************************************************************:)
(: Test: functx-fn-adjust-time-to-timezone-3                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(adjust-time-to-timezone(
   xs:time('17:00:00'), ()))
