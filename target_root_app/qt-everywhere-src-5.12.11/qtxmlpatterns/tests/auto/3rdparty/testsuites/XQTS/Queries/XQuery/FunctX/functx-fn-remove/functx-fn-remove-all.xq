(:**************************************************************:)
(: Test: functx-fn-remove-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(remove( ('a', 'b', 'c'), 2) , remove( ('a', 'b', 'c'), 10), remove( ('a', 'b', 'c'), 0))