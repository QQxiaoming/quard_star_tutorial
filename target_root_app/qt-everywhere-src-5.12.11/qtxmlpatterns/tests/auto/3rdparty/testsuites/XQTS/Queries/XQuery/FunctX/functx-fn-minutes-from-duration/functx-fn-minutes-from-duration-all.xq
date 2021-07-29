(:**************************************************************:)
(: Test: functx-fn-minutes-from-duration-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(minutes-from-duration(
  xs:dayTimeDuration('PT30M')), minutes-from-duration(
  xs:dayTimeDuration('-PT90M')), minutes-from-duration(
  xs:dayTimeDuration('PT1M90S')), minutes-from-duration(
  xs:dayTimeDuration('PT3H')), minutes-from-duration(
  xs:dayTimeDuration('PT60M')))