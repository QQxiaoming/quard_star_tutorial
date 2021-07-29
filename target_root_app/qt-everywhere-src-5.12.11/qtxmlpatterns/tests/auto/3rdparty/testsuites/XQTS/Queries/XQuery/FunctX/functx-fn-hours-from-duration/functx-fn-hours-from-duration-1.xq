(:**************************************************************:)
(: Test: functx-fn-hours-from-duration-1                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(hours-from-duration(
   xs:dayTimeDuration('P1DT5H')))
