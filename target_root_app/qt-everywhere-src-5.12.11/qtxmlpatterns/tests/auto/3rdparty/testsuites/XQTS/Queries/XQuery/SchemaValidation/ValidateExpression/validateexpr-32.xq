(: Name : validateexpr-32 :)
(: Description: Test that validating a union of document nodes does not raise a type error. :)
(: Author: Oliver Hallam :)
(: Date: 2010-03-15 :)

(: insert-start :)
import schema default element namespace "http://www.w3.org/XQueryTestOrderBy";
(: insert-end :)

declare function local:numbers($positive)
{
  if ($positive)
  then <PositiveNumbers><orderData>1</orderData></PositiveNumbers>
  else <NegativeNumbers><orderData>-1</orderData></NegativeNumbers>
};

let $positive as element(orderData, xs:decimal) := (validate { document { local:numbers(true()) } })//orderData
let $negative as element(orderData, xs:decimal) := (validate { document { local:numbers(false()) } })//orderData
return $positive + $negative
