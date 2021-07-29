(: Name: fn-root-21:)
(: Description: Evaluation of the fn:root function used as argument to namespace-uri function (use a comment).:)
(: use fn:count to avoid empty file :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := <!-- An Element Node -->
return
 fn:count(fn:namespace-uri(fn:root($var)))