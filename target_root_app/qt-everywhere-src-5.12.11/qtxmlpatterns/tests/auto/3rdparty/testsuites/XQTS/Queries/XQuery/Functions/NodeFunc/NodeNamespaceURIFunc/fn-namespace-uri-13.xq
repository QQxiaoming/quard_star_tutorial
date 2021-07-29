(: Name: fn-namespace-uri-13:)
(: Description: Evaluation of the fn:namespace-uri function argument set to a computed attribute node with no namespace.:)
(: Use the fn:count function to avoid empty file. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:count(fn:namespace-uri(attribute anAttribute {"Attribute Value No Namespace"}))
