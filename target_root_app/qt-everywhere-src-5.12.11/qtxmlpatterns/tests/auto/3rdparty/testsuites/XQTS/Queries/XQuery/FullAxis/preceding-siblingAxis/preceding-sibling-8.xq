(: Name: preceding-sibling-8 :)
(: Description: Evaluation of the preceding-sibling axis that is part of a "node after" expression (returns true). :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

exactly-one($input-context1/works[1]/employee[13]) >> exactly-one($input-context1/works[1]/employee[12]/preceding-sibling::employee[1])
