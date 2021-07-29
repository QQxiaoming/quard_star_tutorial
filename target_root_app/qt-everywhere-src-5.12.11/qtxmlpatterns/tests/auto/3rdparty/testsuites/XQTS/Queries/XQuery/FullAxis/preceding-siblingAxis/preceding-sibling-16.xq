(: Name: preceding-sibling-16 :)
(: Description: Evaluation of the preceding-sibling axis that is part of a boolean expression ("and" and fn:true(). :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

($input-context1/works[1]/employee[12]/preceding-sibling::employee) and fn:true()