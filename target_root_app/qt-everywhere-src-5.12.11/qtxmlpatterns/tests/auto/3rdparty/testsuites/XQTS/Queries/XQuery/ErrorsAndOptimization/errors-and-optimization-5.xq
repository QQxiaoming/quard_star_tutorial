(: Name: errors-and-optimization-5.xq :)
(: Description: 

   If a processor evaluates an operand E (wholly or in part), then it
   is required to establish that the actual value of the operand E
   does not violate any constraints on its cardinality.
   
:)

let $e := (1,2)
return $e eq 0 