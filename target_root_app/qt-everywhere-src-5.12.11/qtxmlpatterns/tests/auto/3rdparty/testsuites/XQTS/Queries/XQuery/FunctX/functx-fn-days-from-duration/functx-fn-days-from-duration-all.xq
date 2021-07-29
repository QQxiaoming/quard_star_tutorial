(:**************************************************************:)
(: Test: functx-fn-days-from-duration-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(days-from-duration(
   xs:dayTimeDuration('P5D')), days-from-duration(
   xs:dayTimeDuration('-PT24H')), days-from-duration(
   xs:dayTimeDuration('PT23H')), days-from-duration(
   xs:dayTimeDuration('P1DT36H')), days-from-duration(
   xs:dayTimeDuration('PT1440M')))