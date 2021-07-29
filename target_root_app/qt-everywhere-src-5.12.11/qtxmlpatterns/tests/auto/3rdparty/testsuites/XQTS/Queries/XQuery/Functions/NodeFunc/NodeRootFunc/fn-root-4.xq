(: Name: fn-root-4:)
(: Description: Evaluation of the fn:root function with argument set to a comment node (via a variable).:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := <!-- A Comment Node -->
return
 fn:root($var)