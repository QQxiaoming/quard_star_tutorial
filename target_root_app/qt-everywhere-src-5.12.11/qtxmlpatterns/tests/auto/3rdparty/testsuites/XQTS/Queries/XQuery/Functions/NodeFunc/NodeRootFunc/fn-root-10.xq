(: Name: fn-root-10:)
(: Description: Evaluation of the fn:root function with argument set to an computed element with attribute node by setting via a variable.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := element anElement {attribute anAttribute {"Attribute Value"}}
return
 fn:root($var)