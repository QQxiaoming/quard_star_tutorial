(: Name: fn-root-8:)
(: Description: Evaluation of the fn:root function with argument set to an processing instruction node by setting via a variable.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := <?format role="output" ?>
return
 fn:root($var)