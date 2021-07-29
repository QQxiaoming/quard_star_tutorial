(:**************************************************************:)
(: Test: functx-fn-avg-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)


let $ordDoc := doc($input-context1)
return (avg( (1, 2, 3, 4, 5) ), avg( (1, 2, 3, (), 4, 5) ), avg(
   (xs:yearMonthDuration('P4M'),
    xs:yearMonthDuration('P6M') ) ), avg($ordDoc//item/@quantity), avg( () ))