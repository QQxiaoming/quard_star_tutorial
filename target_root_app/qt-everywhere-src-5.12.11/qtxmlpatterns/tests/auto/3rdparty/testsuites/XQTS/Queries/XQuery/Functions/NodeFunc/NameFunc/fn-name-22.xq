(: Name: fn-name-22:)
(: Description: Evaluation of the fn:name function as argument to fn:count.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)


fn:count(((fn:name($input-context1/works[1]/employee[1]),fn:name($input-context1/works[1]/employee[2]))))