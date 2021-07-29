(:**************************************************************:)
(: Test: functx-fn-reverse-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(reverse( (1, 2, 3, 4, 5) ), reverse( (6, 2, 4) ), reverse( () ))