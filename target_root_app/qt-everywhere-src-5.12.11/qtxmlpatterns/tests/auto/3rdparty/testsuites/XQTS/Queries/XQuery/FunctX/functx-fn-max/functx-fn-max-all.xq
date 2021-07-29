(:**************************************************************:)
(: Test: functx-fn-max-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)


let $ordDoc := doc($input-context1)
return (max( (2, 1, 5, 4, 3) ), max( ('a', 'b', 'c') ), max( 2 ), max($ordDoc//item/string(@dept)))