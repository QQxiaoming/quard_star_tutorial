(:**************************************************************:)
(: Test: functx-fn-round-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(round(5), round(5.1), round(5.5), round(-5.5), round(-5.51))