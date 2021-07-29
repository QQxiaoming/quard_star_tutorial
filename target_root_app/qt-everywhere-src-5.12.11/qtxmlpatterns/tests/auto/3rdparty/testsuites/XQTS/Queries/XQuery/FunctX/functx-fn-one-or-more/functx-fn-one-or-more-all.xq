(:**************************************************************:)
(: Test: functx-fn-one-or-more-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(one-or-more('a'), one-or-more( ('a', 'b') ))