(:**************************************************************:)
(: Test: functx-fn-seconds-from-duration-4                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(seconds-from-duration(
   xs:dayTimeDuration('PT60S')))
