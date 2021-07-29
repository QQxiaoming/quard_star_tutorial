(:**************************************************************:)
(: Test: functx-fn-hours-from-duration-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(hours-from-duration(
   xs:dayTimeDuration('P1DT5H')), hours-from-duration(
   xs:dayTimeDuration('-PT36H')), hours-from-duration(
   xs:dayTimeDuration('PT1H90M')), hours-from-duration(
   xs:dayTimeDuration('PT2H59M')), hours-from-duration(
   xs:dayTimeDuration('PT3600S')))