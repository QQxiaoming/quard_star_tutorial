(: Name: followingsibling-17 :)
(: Description: Evaluation of the following-sibling axis that is part of a boolean expression ("and" and fn:false()). :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

($input-context1/works[1]/employee[12]/following-sibling::employee) and fn:false()