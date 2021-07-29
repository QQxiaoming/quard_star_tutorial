(: Name: fn-root-19:)
(: Description: Evaluation of the fn:root function used in conjunction with "is" operator (returns false).:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := element anElement {"Element Content"}
return
 fn:root($var) is fn:root($var)