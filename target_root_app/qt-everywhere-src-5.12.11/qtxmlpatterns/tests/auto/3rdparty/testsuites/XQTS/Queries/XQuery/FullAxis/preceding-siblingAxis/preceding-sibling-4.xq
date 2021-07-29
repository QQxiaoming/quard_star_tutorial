(: Name: preceding-sibling-4 :)
(: Description: Evaluation of the preceding-sibling axis that is part of an "is" expression (return false). :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

exactly-one($input-context1/works[1]/employee[2]/preceding-sibling::employee) is exactly-one($input-context1/works[1]/employee[2])
