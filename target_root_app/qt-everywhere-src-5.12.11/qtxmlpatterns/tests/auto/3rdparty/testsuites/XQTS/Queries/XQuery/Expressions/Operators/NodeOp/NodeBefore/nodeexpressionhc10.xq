(:*******************************************************:)
(:Test: nodeexpressionhc10                               :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 8, 2005                                     :)
(:Purpose: Test of a Node before Expression used as part :)
(:of a boolean-greater-than expression (le operator).    :)
(:*******************************************************:)

(: insert-start :)
declare variable $input-context2 external;
(: insert-end :)

(($input-context2/staff[1]/employee[1]/empnum[1]) << ($input-context2/staff[1]/employee[1]/empnum[1])) le (($input-context2/staff[1]/employee[1]/empnum[1]) << ($input-context2/staff[1]/employee[1]/empnum[1]))