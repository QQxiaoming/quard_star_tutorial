(:**************************************************************:)
(: Test: functx-fn-ceiling-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(ceiling(5), ceiling(5.1), ceiling(5.5), ceiling(-5.5), ceiling(-5.51), ceiling( () ))