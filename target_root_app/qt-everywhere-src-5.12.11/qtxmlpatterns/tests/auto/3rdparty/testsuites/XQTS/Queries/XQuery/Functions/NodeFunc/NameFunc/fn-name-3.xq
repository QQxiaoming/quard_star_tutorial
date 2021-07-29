(: Name: fn-name-3:)
(: Description: Evaluation of the fn:name function with the argument set to an attribute node.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

(fn:name($input-context1/works[1]/employee[1]/@name))