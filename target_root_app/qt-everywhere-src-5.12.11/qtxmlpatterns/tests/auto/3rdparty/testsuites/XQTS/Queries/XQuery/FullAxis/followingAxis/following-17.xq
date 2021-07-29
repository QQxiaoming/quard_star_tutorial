(: Name: following-17 :)
(: Description: Evaluation of the following axis that is part of a boolean expression ("and" and fn:false()). :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

($input-context1/works[1]/employee[12]/following::employee) and fn:false()