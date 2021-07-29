(: Name: followingsibling-5 :)
(: Description: Evaluation of the following-sibling axis that is part of an "node before" expression (return true). :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

exactly-one($input-context1/works[1]/employee[12]/overtime/day[1]/following-sibling::day) << exactly-one($input-context1/works[1]/employee[13])
