(: Name: fn-namespace-uri-12:)
(: Description: Evaluation of the fn:namespace-uri function argument set to a directly constructed attribute node with no namespace.:)
(: Use the fn:count function to avoid empty file. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:count(fn:namespace-uri(<anElement anAttribute="Attribute Value No Namespace"></anElement>))
