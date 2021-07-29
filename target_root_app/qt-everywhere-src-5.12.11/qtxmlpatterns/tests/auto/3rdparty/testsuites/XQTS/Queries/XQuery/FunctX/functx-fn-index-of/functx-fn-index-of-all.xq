(:**************************************************************:)
(: Test: functx-fn-index-of-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(index-of( ('a', 'b', 'c'), 'a'), index-of( ('a', 'b', 'c'), 'd'), index-of( (4, 5, 6, 4), 4), index-of( (4, 5, 6, 4), 04.0), index-of( ('a', 5, 6), 'a'), index-of( (), 'a'), index-of( (<a>1</a>, <b>1</b>), <c>1</c> ))