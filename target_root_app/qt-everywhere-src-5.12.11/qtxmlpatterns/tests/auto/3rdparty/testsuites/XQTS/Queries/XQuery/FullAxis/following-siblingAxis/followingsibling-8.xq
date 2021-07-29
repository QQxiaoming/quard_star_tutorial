(: Name: followingsibling-8 :)
(: Description: Evaluation of the following-sibling axis that is part of an "node after" expression (returns true). :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

exactly-one($input-context1/works[1]/employee[13]) >> exactly-one($input-context1/works[1]/employee[12]/overtime[1]/day[1]/following-sibling::day)
