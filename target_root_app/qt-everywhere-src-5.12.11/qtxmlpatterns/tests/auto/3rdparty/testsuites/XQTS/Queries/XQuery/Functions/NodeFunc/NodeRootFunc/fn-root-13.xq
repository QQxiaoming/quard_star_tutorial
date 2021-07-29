(: Name: fn-root-13:)
(: Description: Evaluation of the fn:root function with argument set to an computed document node by setting directly on argument.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

 fn:root(document {<anElement><anInternalElement>element content</anInternalElement></anElement>})