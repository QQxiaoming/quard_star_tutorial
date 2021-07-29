(: Name: fn-root-20:)
(: Description: Evaluation of the fn:root function used as argument to namespace-uri function (use an element).:)
(: use fn:count to avoid empty file :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := element anElement {"Element Content"}
return
 fn:count(fn:namespace-uri(fn:root($var)))