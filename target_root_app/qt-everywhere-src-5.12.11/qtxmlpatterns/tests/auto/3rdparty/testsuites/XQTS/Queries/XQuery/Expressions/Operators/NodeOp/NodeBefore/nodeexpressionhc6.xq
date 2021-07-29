(:*******************************************************:)
(: Test: nodeexpressionhc6                               :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 8, 2005                                     :)
(:Purpose: Test of a Node before Expression used as an   :)
(:argument to a "fn:not" function.                       :)
(:*******************************************************:)

(: insert-start :)
declare variable $input-context2 external;
(: insert-end :)

fn:not(($input-context2/staff[1]/employee[1]/empnum[1]) << ($input-context2/staff[1]/employee[1]/empnum[1]))