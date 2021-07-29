(:**************************************************************:)
(: Test: functx-fn-days-from-duration-4                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(days-from-duration(
   xs:dayTimeDuration('P1DT36H')))
