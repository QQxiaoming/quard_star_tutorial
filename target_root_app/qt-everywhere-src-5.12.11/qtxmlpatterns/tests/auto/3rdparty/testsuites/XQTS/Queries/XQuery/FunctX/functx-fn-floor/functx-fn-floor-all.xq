(:**************************************************************:)
(: Test: functx-fn-floor-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(floor(5), floor(5.1), floor(5.7), floor(-5.1), floor(-5.7), floor( () ))