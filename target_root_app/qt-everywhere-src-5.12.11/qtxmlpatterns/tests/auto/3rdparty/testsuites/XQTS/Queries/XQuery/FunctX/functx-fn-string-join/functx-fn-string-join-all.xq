(:**************************************************************:)
(: Test: functx-fn-string-join-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(string-join( ('a', 'b', 'c'), ''), string-join( ('a', 'b', 'c'), '/*'), string-join( ('a', '', 'c'), '/*'), string-join( 'a', '/*'), string-join((), '/*'))