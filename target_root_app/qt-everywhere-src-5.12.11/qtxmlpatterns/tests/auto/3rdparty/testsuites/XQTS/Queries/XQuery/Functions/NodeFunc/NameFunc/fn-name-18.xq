(: Name: fn-name-18:)
(: Description: Evaluation of the fn:name function as an argument to the string-length function.:)
(: The context node is an attribute node. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)


fn:string-length(fn:name($input-context1/works[1]/employee[2]/@name))