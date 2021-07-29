(: Name: fn-root-22:)
(: Description: Evaluation of the fn:root function used as argument to namespace-uri function (use a Processing Instruction).:)
(: use fn:count to avoid empty file :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := <?format role="output" ?>

return
 fn:count(fn:namespace-uri(fn:root($var)))