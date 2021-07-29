(:**************************************************************:)
(: Test: functx-fn-deep-equal-6                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";

let $prod1 := <product dept='MEN' id='P123'>
  <number>784</number>
</product>
return 
let $prod2 := <product id='P123' dept='MEN'><!--comment-->
  <number>784</number>
</product>
return (deep-equal( $prod1/number, $prod2/number ))
