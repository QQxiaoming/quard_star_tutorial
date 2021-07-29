(: Name: following-18 :)
(: Description: Evaluation of the following axis that is part of a boolean expression ("or" and fn:true()). :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

($input-context1/works[1]/employee[12]/following::employee) or fn:true()