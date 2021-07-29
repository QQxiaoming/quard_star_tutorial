(: Name: fn-root-12:)
(: Description: Evaluation of the fn:root function with argument set to a document node by setting via a variable.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := document {<anElement><anInternalElement>element content</anInternalElement></anElement>}
return
 fn:root($var)