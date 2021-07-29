(: Name: fn-root-6:)
(: Description: Evaluation of the fn:root function with argument set to an elemnt node set via a variable.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := <anElement>An Element Content</anElement>
return 
 fn:root($var)