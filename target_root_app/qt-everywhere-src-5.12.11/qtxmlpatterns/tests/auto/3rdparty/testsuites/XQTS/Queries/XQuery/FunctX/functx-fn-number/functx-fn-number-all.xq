(:**************************************************************:)
(: Test: functx-fn-number-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)


let $priceDoc := doc($input-context1)
return (number(
    $priceDoc//prod[1]/price), number(
    $priceDoc//prod[1]/@currency), number('29.99'), number('ABC'), number( () ), $priceDoc//
    prod/price[number() > 35])