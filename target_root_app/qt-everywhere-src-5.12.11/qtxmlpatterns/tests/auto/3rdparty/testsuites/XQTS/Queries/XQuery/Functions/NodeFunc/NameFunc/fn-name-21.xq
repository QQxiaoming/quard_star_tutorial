(: Name: fn-name-21:)
(: Description: Evaluation of the fn:name function used as part of a sequence.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)


(fn:name($input-context1/works[1]/employee[1]),fn:name($input-context1/works[1]/employee[2]))